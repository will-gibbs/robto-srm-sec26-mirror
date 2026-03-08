//******************************************************************************
//* Project: Robto SRM, IEEE SoutheastCon 2026                                 *
//* Package: I2C Multiplexer Driver                                            *
//* Name:    i2c_multiplexer_driver.cpp                                        *
//* Author:  Christian Phan                                                    *
//******************************************************************************

//******************************************************************************
//* Controls and interprets the data from the devices to the PI.               *
//*   1. Write the channel select byte to the multiplexer (0x70)               *
//*   2. Communicate with the component on the selected channel                *
//*   3. Reset the multiplexer (writes 0x00) when done                         *
//*                                                                            *
//* Components:                                                                *
//*   Channel TBD - TCS34725  Color Sensor      (addr 0x29)                    *
//*   Channel TBD - VL53L5CX  Distance Sensor   (addr 0x52)                    *
//*   Channel TBD - SSD1306   OLED Display      (addr 0x3C or 0x3D)            *
//*                                                                            *
//* Publishers:                                                                *
//*   - i2c_multiplexer/color_sensor    (std_msgs/msg/Int32)                   *
//*     Antenna LED color reading from TCS34725. Used to identify which        *
//*     color (red, blue, green, purple) each antenna's dish LED is showing.   *
//*     Score = R + (G/2) - B. Higher = more yellow/orange.                    *
//*   - i2c_multiplexer/distance_sensor (std_msgs/msg/Float32MultiArray)       *
//*     64 zone distances in mm from VL53L5CX (8x8 grid, row-major order).     *
//*     Used for obstacle avoidance. Invalid zones published as -1.0.          *
//*                                                                            *
//* Subscriptions:                                                             *
//*   - i2c_multiplexer/display         (std_msgs/msg/String)                  *
//*     2-character string to show on OLED: antenna number + color letter      *
//******************************************************************************

// C++-specific packages
#include <memory>
#include <chrono>
#include <algorithm>

// ROS2-specific packages
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "std_msgs/msg/float32.hpp"
#include "std_msgs/msg/int32.hpp"
#include "std_msgs/msg/float32_multi_array.hpp"

// libi2cd - C library for I2C communication
extern "C" {
#include "i2cd.h"
}

// VL53L5CX ULD driver - ST Ultra Lite Driver for distance sensor (STSW-IMG023)
extern "C" {
#include "platform.h"
#include "vl53l5cx_api.h"
}

// Namespaces
using namespace std;
using namespace rclcpp;
using namespace std::chrono_literals;

// I2C bus device path on Raspberry Pi 5
#define I2C_BUS "/dev/i2c-1"

// PCA9546 Multiplexer I2C address (Adafruit PCA9546 guide p5)
#define MUX_ADDR 0x70

// Multiplexer channel select bytes - write (1 << channel) to select, 0x00 to deselect all
// (Adafruit PCA9546 guide p4, p16 I2C Scanner Example)
#define MUX_CHANNEL_0  0x01
#define MUX_CHANNEL_1  0x02
#define MUX_CHANNEL_2  0x04
#define MUX_CHANNEL_3  0x08
#define MUX_RESET      0x00

// Component I2C addresses
#define COLOR_SENSOR_ADDR    0x29  // TCS34725 (TCS34725.pdf p3, Available Options table)
#define DISTANCE_SENSOR_ADDR 0x52  // VL53L5CX default I2C address (UM2884 p4, s2.3)
#define DISPLAY_ADDR         0x3C  // SSD1306 OLED (may be 0x3D depending on hardware config)

// TODO: Confirm channel assignments with team
#define COLOR_SENSOR_CHANNEL    MUX_CHANNEL_0
#define DISTANCE_SENSOR_CHANNEL MUX_CHANNEL_1
#define DISPLAY_CHANNEL         MUX_CHANNEL_3

// TCS34725 Color Sensor registers (TCS34725.pdf p13, Table 3)
#define TCS34725_COMMAND_BIT  0x80  // Must be set when addressing registers (TCS34725.pdf p14, Table 4)
#define TCS34725_ENABLE       0x00  // Enable register (TCS34725.pdf p15, Table 5)
#define TCS34725_ATIME        0x01  // Integration time register (TCS34725.pdf p16, Table 6)
#define TCS34725_CONTROL      0x0F  // Gain control register (TCS34725.pdf p18, Table 11)
#define TCS34725_STATUS       0x13  // Status register, bit 0 = data ready (TCS34725.pdf p19, Table 13)
#define TCS34725_CDATAL       0x14  // Clear channel low byte (TCS34725.pdf p19, Table 14)
#define TCS34725_RDATAL       0x16  // Red channel low byte (TCS34725.pdf p19, Table 14)
#define TCS34725_GDATAL       0x18  // Green channel low byte (TCS34725.pdf p19, Table 14)
#define TCS34725_BDATAL       0x1A  // Blue channel low byte (TCS34725.pdf p19, Table 14)
#define TCS34725_ENABLE_PON   0x01  // Power on bit (TCS34725.pdf p15, Table 5)
#define TCS34725_ENABLE_AEN   0x02  // RGBC enable bit (TCS34725.pdf p15, Table 5)

// SSD1306 OLED Display commands (SSD1306.pdf p28, Table 9-1)
#define SSD1306_CMD_CHARGE_PUMP   0x8D  // Charge pump command (SSD1306.pdf p28, Table 9-1)
#define SSD1306_CMD_ENABLE_PUMP   0x14  // Enable internal charge pump (SSD1306.pdf p28, Table 9-1)
#define SSD1306_CMD_DISPLAY_ON    0xAF  // Turn display on (SSD1306.pdf p37, s10.1.12)
#define SSD1306_CMD_MEM_MODE      0x20  // Set memory addressing mode (SSD1306.pdf p34, s10.1.3)
#define SSD1306_CMD_HORIZ_MODE    0x00  // Horizontal addressing mode (SSD1306.pdf p34, s10.1.3)

// VL53L5CX target status value indicating a valid measurement (UM2884 p14, Table 4)
#define VL53L5CX_STATUS_VALID 5

// Poll rate
#define POLL_RATE 100ms

//******************************************************************************
//*                              Class Definition                              *
//******************************************************************************
class I2CMultiplexerDriver : public Node
{
   struct i2cd *i2c_dev = nullptr; // I2C device handle (Bus)

   // VL53L5CX sensor configuration (persists between ticks - init once, read each tick)
   VL53L5CX_Configuration distance_dev;
   bool                   distance_dev_ready = false; // true after successful init

   // Publishers
   Publisher<std_msgs::msg::Int32>::SharedPtr             color_sensor_publisher;    // Antenna LED color score from TCS34725
   Publisher<std_msgs::msg::Float32MultiArray>::SharedPtr distance_sensor_publisher; // 64 zone distances in mm from VL53L5CX

   // Subscription
   Subscription<std_msgs::msg::String>::SharedPtr display_subscriber; // 2-char string to show on OLED e.g. "1R", "2B"

   // Timer
   TimerBase::SharedPtr timer;

   // Switch the multiplexer to the given channel
   bool select_channel(uint8_t channel);
   // Reset the multiplexer (deselect all channels)
   void reset_mux();

   // Sensor initialization functions
   bool init_color_sensor();
   bool init_distance_sensor();
   bool init_display();

   // Sensor read functions
   void read_color_sensor();
   void read_distance_sensor();

   // Display write function
   void write_display(const std::string &text);
   void display_callback(const std_msgs::msg::String::SharedPtr message);

   // Timer callback - reads all sensors each tick
   void timer_callback();

public:
   I2CMultiplexerDriver();
   ~I2CMultiplexerDriver();
};

//******************************************************************************
//*                   I2CMultiplexerDriver Constructor                         *
//******************************************************************************
I2CMultiplexerDriver::I2CMultiplexerDriver() : Node("i2c_multiplexer")
{
   // Open the I2C bus
   i2c_dev = i2cd_open(I2C_BUS);
   if (i2c_dev == nullptr)
   {
      RCLCPP_ERROR(get_logger(), "Failed to open I2C bus at %s. Is the hardware connected?", I2C_BUS);
      // Node will still start but sensor reads will fail gracefully
   }
   else
   {
      RCLCPP_INFO(get_logger(), "I2C bus opened successfully.");

      // Initialize sensors
      if (!init_color_sensor())
         RCLCPP_WARN(get_logger(), "Color sensor initialization failed.");

      if (!init_distance_sensor())
         RCLCPP_WARN(get_logger(), "Distance sensor initialization failed.");

      if (!init_display())
         RCLCPP_WARN(get_logger(), "OLED display initialization failed.");
   }

   // Create publishers
   color_sensor_publisher    = create_publisher<std_msgs::msg::Int32>("i2c_multiplexer/color_sensor", 10);
   distance_sensor_publisher = create_publisher<std_msgs::msg::Float32MultiArray>("i2c_multiplexer/distance_sensor", 10);

   // Create display subscriber
   display_subscriber = create_subscription<std_msgs::msg::String>(
      "i2c_multiplexer/display", 10,
      [this](const std_msgs::msg::String::SharedPtr message) { display_callback(message); }
   );

   // Create timer to poll sensors
   timer = create_wall_timer(POLL_RATE, [this]() { timer_callback(); });

   RCLCPP_INFO(get_logger(), "I2C Multiplexer Driver is online.");
}

//******************************************************************************
//*                    I2CMultiplexerDriver Destructor                         *
//******************************************************************************
I2CMultiplexerDriver::~I2CMultiplexerDriver()
{
   if (i2c_dev != nullptr)
   {
      // Stop ranging before shutting down (VL53L5CX ULD API Example_1_ranging_basic.c)
      if (distance_dev_ready)
      {
         select_channel(DISTANCE_SENSOR_CHANNEL);
         vl53l5cx_stop_ranging(&distance_dev);
         reset_mux();
      }

      reset_mux();
      i2cd_close(i2c_dev);
      RCLCPP_INFO(get_logger(), "I2C bus closed.");
   }
}

//******************************************************************************
//*              Switch the PCA9546 multiplexer to the given channel           *
//******************************************************************************
bool I2CMultiplexerDriver::select_channel(uint8_t channel)
{
   bool success = (i2c_dev != nullptr);

   if (success && i2cd_write(i2c_dev, MUX_ADDR, &channel, 1) < 0)
   {
      RCLCPP_ERROR(get_logger(), "Failed to select multiplexer channel 0x%02X.", channel);
      success = false;
   }

   return success;
}

//******************************************************************************
//*                  Reset the multiplexer (deselect all channels)             *
//******************************************************************************
void I2CMultiplexerDriver::reset_mux()
{
   uint8_t reset = MUX_RESET;

   if (i2c_dev != nullptr)
      i2cd_write(i2c_dev, MUX_ADDR, &reset, 1);

   return;
}

//******************************************************************************
//*                     Initialize the TCS34725 color sensor                   *
//******************************************************************************
bool I2CMultiplexerDriver::init_color_sensor()
{
   uint8_t cmd_aen[2]   = { (uint8_t)(TCS34725_COMMAND_BIT | TCS34725_ENABLE), (uint8_t)(TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN) };
   uint8_t cmd_atime[2] = { (uint8_t)(TCS34725_COMMAND_BIT | TCS34725_ATIME), 0xD5 };
   uint8_t cmd_gain[2]  = { (uint8_t)(TCS34725_COMMAND_BIT | TCS34725_CONTROL), 0x00 };
   uint8_t cmd_pon[2]   = { (uint8_t)(TCS34725_COMMAND_BIT | TCS34725_ENABLE), TCS34725_ENABLE_PON };
   bool    success      = select_channel(COLOR_SENSOR_CHANNEL);

   if (success)
   {
      // Power on: write PON bit to ENABLE register (TCS34725.pdf p15, Table 5)
      success = (i2cd_write(i2c_dev, COLOR_SENSOR_ADDR, cmd_pon, 2) >= 0);
   }

   if (success)
   {
      // Wait for power-on: 2.4ms minimum after PON (TCS34725.pdf p15, Table 5 note 2)
      rclcpp::sleep_for(3ms);

      // Enable RGBC: write PON | AEN to ENABLE register (TCS34725.pdf p15, Table 5)
      success = (i2cd_write(i2c_dev, COLOR_SENSOR_ADDR, cmd_aen, 2) >= 0);
   }

   if (success)
   {
      // Set integration time: 0xD5 = ~101ms (TCS34725.pdf p16, Table 6)
      success = (i2cd_write(i2c_dev, COLOR_SENSOR_ADDR, cmd_atime, 2) >= 0);
   }

   if (success)
   {
      // Set gain: 0x00 = 1x gain (TCS34725.pdf p18, Table 11)
      success = (i2cd_write(i2c_dev, COLOR_SENSOR_ADDR, cmd_gain, 2) >= 0);
   }

   reset_mux();

   if (success)
      RCLCPP_INFO(get_logger(), "TCS34725 color sensor initialized.");

   return success;
}

//******************************************************************************
//*   Initialize the VL53L5CX distance sensor and start continuous ranging     *
//*   This is called once at startup. The sensor stays ranging between ticks   *
//******************************************************************************
bool I2CMultiplexerDriver::init_distance_sensor()
{
   uint8_t  is_alive = 0;
   bool     success  = select_channel(DISTANCE_SENSOR_CHANNEL);
   uint8_t  status   = 0;

   if (success)
   {
      // Provide libi2cd handle and I2C address to the platform layer
      distance_dev.platform.i2c_dev = i2c_dev;
      distance_dev.platform.address = DISTANCE_SENSOR_ADDR;

      // Check sensor is present and responding (VL53L5CX ULD API Example_1_ranging_basic.c)
      status  = vl53l5cx_is_alive(&distance_dev, &is_alive);
      success = (status == 0 && is_alive);
   }

   if (success)
   {
      // Upload firmware and initialize sensor - this takes ~1-2 seconds (VL53L5CX ULD API Example_1_ranging_basic.c)
      RCLCPP_INFO(get_logger(), "Loading VL53L5CX firmware, please wait...");
      status  = vl53l5cx_init(&distance_dev);
      success = (status == 0);
   }

   if (success)
   {
      // Set 8x8 resolution for full zone grid - must be set before ranging frequency (UM2884 p8, s4.3)
      status  = vl53l5cx_set_resolution(&distance_dev, VL53L5CX_RESOLUTION_8X8);
      success = (status == 0);
   }

   if (success)
   {
      // Set ranging frequency to 10 Hz - default is 1 Hz, too slow for 100ms poll rate
      // Max for 8x8 is 15 Hz (UM2884 p8, Table 2)
      status  = vl53l5cx_set_ranging_frequency_hz(&distance_dev, 10);
      success = (status == 0);
   }

   if (success)
   {
      // Start continuous ranging - sensor keeps measuring between ticks (VL53L5CX ULD API Example_1_ranging_basic.c)
      status  = vl53l5cx_start_ranging(&distance_dev);
      success = (status == 0);
   }

   reset_mux();

   if (success)
   {
      distance_dev_ready = true;
      RCLCPP_INFO(get_logger(), "VL53L5CX distance sensor initialized.");
   }

   return success;
}

//******************************************************************************
//*                        Initialize the SSD1306 OLED display                *
//******************************************************************************
bool I2CMultiplexerDriver::init_display()
{
   uint8_t init_seq[] = {
      0x00,                      // Control byte: all following bytes are commands (SSD1306.pdf p19, s8.1.5)
      SSD1306_CMD_CHARGE_PUMP,   // Charge pump command (SSD1306.pdf p28, Table 9-1)
      SSD1306_CMD_ENABLE_PUMP,   // Enable internal charge pump (SSD1306.pdf p28, Table 9-1)
      SSD1306_CMD_MEM_MODE,      // Set memory addressing mode (SSD1306.pdf p34, s10.1.3)
      SSD1306_CMD_HORIZ_MODE,    // Horizontal addressing mode (SSD1306.pdf p34, s10.1.3)
      SSD1306_CMD_DISPLAY_ON     // Turn display on (SSD1306.pdf p37, s10.1.12)
   };
   bool success = select_channel(DISPLAY_CHANNEL);

   if (success)
   {
      // Send initialization sequence (SSD1306.pdf p27, Figure 8-16 power on sequence)
      success = (i2cd_write(i2c_dev, DISPLAY_ADDR, init_seq, sizeof(init_seq)) >= 0);
   }

   reset_mux();

   if (success)
      RCLCPP_INFO(get_logger(), "SSD1306 OLED display initialized.");

   return success;
}

//******************************************************************************
//*        Read RGBC data from TCS34725 and publish the raw color values.      *
//*        Used by the navigation node to identify each antenna's LED color    *
//*        (red, blue, green, purple) when the arm positions the sensor        *
//*        over the antenna dish.                                              *
//*        Score = R + (G/2) - B. Published as Int32.                          *
//******************************************************************************
void I2CMultiplexerDriver::read_color_sensor()
{
   uint16_t blue       = 0;
   uint16_t green      = 0;
   uint16_t red        = 0;
   uint8_t  raw[8]     = {0};
   uint8_t  data_reg   = TCS34725_COMMAND_BIT | 0x10 | TCS34725_CDATAL; // Auto-increment mode (TCS34725.pdf p14, Table 4)
   uint8_t  status     = 0;
   uint8_t  status_reg = TCS34725_COMMAND_BIT | TCS34725_STATUS;
   int32_t  score      = 0;
   bool     success    = (i2c_dev != nullptr) && select_channel(COLOR_SENSOR_CHANNEL);

   if (success)
   {
      // Check AVALID bit (bit 0) in STATUS register - indicates conversion complete (TCS34725.pdf p19, Table 13)
      success = (i2cd_write_read(i2c_dev, COLOR_SENSOR_ADDR, &status_reg, 1, &status, 1) >= 0)
                && (status & 0x01);
   }

   if (success)
   {
      // Read 8 bytes starting at CDATAL using auto-increment (TCS34725.pdf p14, Table 4 and p19, Table 14)
      // Order: C_low, C_high, R_low, R_high, G_low, G_high, B_low, B_high
      success = (i2cd_write_read(i2c_dev, COLOR_SENSOR_ADDR, &data_reg, 1, raw, 8) >= 0);
   }

   reset_mux();

   if (success)
   {
      // Combine low and high bytes into 16-bit values (TCS34725.pdf p19, Table 14)
      red   = (uint16_t)(raw[2] | (raw[3] << 8));
      green = (uint16_t)(raw[4] | (raw[5] << 8));
      blue  = (uint16_t)(raw[6] | (raw[7] << 8));

      // Color score used to identify antenna LED color (red, blue, green, purple)
      // Higher R = red, higher G = green, higher B = blue/purple, high R+G = yellow (not used here)
      score = (int32_t)red + (int32_t)(green / 2) - (int32_t)blue;

      auto message = std_msgs::msg::Int32();
      message.data = score;
      color_sensor_publisher->publish(message);
   }

   return;
}

//******************************************************************************
//*       Read distances and publish an array of 64 zone distances in mm       *
//******************************************************************************
void I2CMultiplexerDriver::read_distance_sensor()
{
   uint8_t  is_ready = 0;
   auto     message  = std_msgs::msg::Float32MultiArray();
   bool     success  = distance_dev_ready && select_channel(DISTANCE_SENSOR_CHANNEL);
   uint8_t  status   = 0;
   int      zone     = 0;
   VL53L5CX_ResultsData results;

   if (success)
   {
      // Poll for new data - skip tick if not ready yet (VL53L5CX ULD API Example_1_ranging_basic.c)
      status  = vl53l5cx_check_data_ready(&distance_dev, &is_ready);
      success = (status == 0 && is_ready);
   }

   if (success)
   {
      // Retrieve the 8x8 results grid (VL53L5CX ULD API Example_1_ranging_basic.c)
      status  = vl53l5cx_get_ranging_data(&distance_dev, &results);
      success = (status == 0);
   }

   reset_mux();

   if (success)
   {
      // Publish all 64 zone distances - invalid zones published as -1.0
      // target_status == 5 means valid (UM2884 p14, Table 4)
      message.data.resize(VL53L5CX_RESOLUTION_8X8);

      for (zone = 0; zone < VL53L5CX_RESOLUTION_8X8; zone++)
      {
         if (results.target_status[zone * VL53L5CX_NB_TARGET_PER_ZONE] == VL53L5CX_STATUS_VALID)
            message.data[zone] = (float)results.distance_mm[zone * VL53L5CX_NB_TARGET_PER_ZONE];
         else
            message.data[zone] = -1.0f;
      }

      distance_sensor_publisher->publish(message);
   }

   return;
}

//******************************************************************************
//*             2-character string display (SSD1306 OLED display)              *
//******************************************************************************
void I2CMultiplexerDriver::write_display(const std::string &text)
{
   if (i2c_dev != nullptr && select_channel(DISPLAY_CHANNEL))
   {
      // TODO: Implement full SSD1306 framebuffer rendering (SSD1306.pdf p25, s8.7 GDDRAM)
      RCLCPP_INFO(get_logger(), "Display (TODO): %s", text.c_str());

      reset_mux();
   }

   return;
}

//******************************************************************************
//*                   Callback for incoming display messages                   *
//******************************************************************************
void I2CMultiplexerDriver::display_callback(const std_msgs::msg::String::SharedPtr message)
{
   write_display(message->data);

   return;
}

//******************************************************************************
//*              Timer callback - poll all sensors every tick                  *
//******************************************************************************
void I2CMultiplexerDriver::timer_callback()
{
   read_color_sensor();
   read_distance_sensor();

   return;
}

//******************************************************************************
//*                               Main Function                                *
//******************************************************************************
int main(int argc, char **argv)
{
   init(argc, argv);
   RCLCPP_INFO(get_logger("rclcpp"), "Starting I2C Multiplexer Driver...");
   spin(make_shared<I2CMultiplexerDriver>());
   shutdown();
   
   return 0;
}
