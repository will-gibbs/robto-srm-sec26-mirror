//******************************************************************************
//* Project: Robto SRM, IEEE SoutheastCon 2026                                 *
//* Package: I2C Multiplexer Driver                                            *
//* Name:    i2c_multiplexer_driver.cpp                                        *
//******************************************************************************

//******************************************************************************
//* This node interfaces with the PCA9546 4-channel I2C multiplexer and all   *
//* I2C components connected to it. Because all components share the same I2C  *
//* bus through the multiplexer, this single node handles all communication    *
//* so that channel switching can be properly sequenced.                       *
//*                                                                            *
//* To communicate with a component, this node:                               *
//*   1. Writes the channel select byte to the multiplexer (0x70)             *
//*   2. Communicates with the component on the selected channel               *
//*   3. Resets the multiplexer (writes 0x00) when done                       *
//*                                                                            *
//* Components:                                                                *
//*   Channel TBD - TCS34725  Color Sensor      (addr 0x29)                   *
//*   Channel TBD - VL53L5CX  Distance Sensor   (addr 0x29)                   *
//*   Channel TBD - VL53L4CD  Duck ToF Sensor   (addr 0x29)                   *
//*   Channel TBD - SSD1306   OLED Display      (addr 0x3C or 0x3D)           *
//*                                                                            *
//* Publishers:                                                                *
//*   - i2c_multiplexer/color_sensor    (std_msgs/msg/UInt16MultiArray)        *
//*   - i2c_multiplexer/distance_sensor (std_msgs/msg/Float32)                 *
//*   - i2c_multiplexer/duck_sensor     (std_msgs/msg/Float32)                 *
//*                                                                            *
//* Subscriptions:                                                             *
//*   - i2c_multiplexer/display         (std_msgs/msg/String)                  *
//******************************************************************************

// C++-specific packages
#include <memory>
#include <chrono>

// ROS2-specific packages
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "std_msgs/msg/float32.hpp"
#include "std_msgs/msg/u_int16_multi_array.hpp"

// libi2cd - C library for I2C communication
extern "C" {
#include "i2cd.h"
}

// Namespaces
using namespace std;
using namespace rclcpp;
using namespace std::chrono_literals;

// I2C bus device path on Raspberry Pi 5 (GPIO 2/3 = I2C1)
#define I2C_BUS "/dev/i2c-1"

// PCA9546 Multiplexer I2C address
#define MUX_ADDR 0x70

// Multiplexer channel select bytes
#define MUX_CHANNEL_0  0x01
#define MUX_CHANNEL_1  0x02
#define MUX_CHANNEL_2  0x04
#define MUX_CHANNEL_3  0x08
#define MUX_RESET      0x00

// Component I2C addresses
#define COLOR_SENSOR_ADDR    0x29  // TCS34725
#define DISTANCE_SENSOR_ADDR 0x29  // VL53L5CX  (on different mux channel than color sensor)
#define DUCK_SENSOR_ADDR     0x29  // VL53L4CD  (on different mux channel than above)
#define DISPLAY_ADDR         0x3C  // SSD1306 OLED (may be 0x3D depending on hardware config)

// TODO: Assign correct channels once hardware is confirmed
#define COLOR_SENSOR_CHANNEL    MUX_CHANNEL_0
#define DISTANCE_SENSOR_CHANNEL MUX_CHANNEL_1
#define DUCK_SENSOR_CHANNEL     MUX_CHANNEL_2
#define DISPLAY_CHANNEL         MUX_CHANNEL_3

// TCS34725 Color Sensor registers
#define TCS34725_COMMAND_BIT  0x80  // Must be set when writing to registers
#define TCS34725_ENABLE       0x00  // Enable register
#define TCS34725_ATIME        0x01  // Integration time register
#define TCS34725_CONTROL      0x0F  // Gain control register
#define TCS34725_STATUS       0x13  // Status register (bit 0 = data ready)
#define TCS34725_CDATAL       0x14  // Clear channel low byte
#define TCS34725_RDATAL       0x16  // Red channel low byte
#define TCS34725_GDATAL       0x18  // Green channel low byte
#define TCS34725_BDATAL       0x1A  // Blue channel low byte
#define TCS34725_ENABLE_PON   0x01  // Power on
#define TCS34725_ENABLE_AEN   0x02  // RGBC enable

// SSD1306 OLED Display commands
#define SSD1306_CMD_CHARGE_PUMP   0x8D  // Charge pump command
#define SSD1306_CMD_ENABLE_PUMP   0x14  // Enable internal charge pump
#define SSD1306_CMD_DISPLAY_ON    0xAF  // Turn display on
#define SSD1306_CMD_MEM_MODE      0x20  // Set memory addressing mode
#define SSD1306_CMD_HORIZ_MODE    0x00  // Horizontal addressing mode

// Poll rate
#define POLL_RATE 100ms

//******************************************************************************
//*                              Class Definition                              *
//******************************************************************************
class I2CMultiplexerDriver : public Node
{
   // I2C device handle (opened once, used throughout node lifetime)
   struct i2cd *i2c_dev = nullptr;

   // Publishers
   Publisher<std_msgs::msg::UInt16MultiArray>::SharedPtr color_sensor_publisher;    // RGBC raw values from TCS34725
   Publisher<std_msgs::msg::Float32>::SharedPtr          distance_sensor_publisher; // Distance in mm from VL53L5CX
   Publisher<std_msgs::msg::Float32>::SharedPtr          duck_sensor_publisher;     // Distance in mm from VL53L4CD

   // Subscription
   Subscription<std_msgs::msg::String>::SharedPtr display_subscriber; // Text/state to show on OLED

   // Timer
   TimerBase::SharedPtr timer;

   // Switch the multiplexer to the given channel
   bool select_channel(uint8_t channel);
   // Reset the multiplexer (deselect all channels)
   void reset_mux();

   // Sensor initialization functions
   bool init_color_sensor();
   bool init_display();

   // Sensor read functions
   void read_color_sensor();
   void read_distance_sensor();
   void read_duck_sensor();

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
      if (!init_color_sensor()) RCLCPP_WARN(get_logger(), "Color sensor initialization failed.");
      if (!init_display())      RCLCPP_WARN(get_logger(), "OLED display initialization failed.");
   }

   // Create publishers
   color_sensor_publisher    = create_publisher<std_msgs::msg::UInt16MultiArray>("i2c_multiplexer/color_sensor", 10);
   distance_sensor_publisher = create_publisher<std_msgs::msg::Float32>("i2c_multiplexer/distance_sensor", 10);
   duck_sensor_publisher     = create_publisher<std_msgs::msg::Float32>("i2c_multiplexer/duck_sensor", 10);

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
   if (i2c_dev == nullptr) return false;

   if (i2cd_write(i2c_dev, MUX_ADDR, &channel, 1) < 0)
   {
      RCLCPP_ERROR(get_logger(), "Failed to select multiplexer channel 0x%02X.", channel);
      return false;
   }
   return true;
}

//******************************************************************************
//*                  Reset the multiplexer (deselect all channels)             *
//******************************************************************************
void I2CMultiplexerDriver::reset_mux()
{
   if (i2c_dev == nullptr) return;
   uint8_t reset = MUX_RESET;
   i2cd_write(i2c_dev, MUX_ADDR, &reset, 1);
}

//******************************************************************************
//*                     Initialize the TCS34725 color sensor                  *
//******************************************************************************
bool I2CMultiplexerDriver::init_color_sensor()
{
   if (!select_channel(COLOR_SENSOR_CHANNEL)) return false;

   // Power on the sensor
   uint8_t cmd_pon[2] = { (uint8_t)(TCS34725_COMMAND_BIT | TCS34725_ENABLE), TCS34725_ENABLE_PON };
   if (i2cd_write(i2c_dev, COLOR_SENSOR_ADDR, cmd_pon, 2) < 0) { reset_mux(); return false; }

   // Wait for power-on (2.4ms minimum per datasheet)
   rclcpp::sleep_for(3ms);

   // Enable RGBC sensing
   uint8_t cmd_aen[2] = { (uint8_t)(TCS34725_COMMAND_BIT | TCS34725_ENABLE), (uint8_t)(TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN) };
   if (i2cd_write(i2c_dev, COLOR_SENSOR_ADDR, cmd_aen, 2) < 0) { reset_mux(); return false; }

   // Set integration time: 0xEB = ~103ms (good balance of speed and accuracy)
   uint8_t cmd_atime[2] = { (uint8_t)(TCS34725_COMMAND_BIT | TCS34725_ATIME), 0xEB };
   if (i2cd_write(i2c_dev, COLOR_SENSOR_ADDR, cmd_atime, 2) < 0) { reset_mux(); return false; }

   // Set gain: 0x00 = 1x gain
   uint8_t cmd_gain[2] = { (uint8_t)(TCS34725_COMMAND_BIT | TCS34725_CONTROL), 0x00 };
   if (i2cd_write(i2c_dev, COLOR_SENSOR_ADDR, cmd_gain, 2) < 0) { reset_mux(); return false; }

   reset_mux();
   RCLCPP_INFO(get_logger(), "TCS34725 color sensor initialized.");
   return true;
}

//******************************************************************************
//*                        Initialize the SSD1306 OLED display                *
//******************************************************************************
bool I2CMultiplexerDriver::init_display()
{
   if (!select_channel(DISPLAY_CHANNEL)) return false;

   // Send charge pump command sequence (required or screen stays black)
   uint8_t init_seq[] = {
      0x00,                      // Control byte: following bytes are commands
      SSD1306_CMD_CHARGE_PUMP,   // Charge pump command
      SSD1306_CMD_ENABLE_PUMP,   // Enable internal charge pump
      SSD1306_CMD_MEM_MODE,      // Set memory addressing mode
      SSD1306_CMD_HORIZ_MODE,    // Horizontal addressing mode
      SSD1306_CMD_DISPLAY_ON     // Turn display on
   };
   if (i2cd_write(i2c_dev, DISPLAY_ADDR, init_seq, sizeof(init_seq)) < 0)
   {
      reset_mux();
      return false;
   }

   reset_mux();
   RCLCPP_INFO(get_logger(), "SSD1306 OLED display initialized.");
   return true;
}

//******************************************************************************
//*               Read RGBC data from the TCS34725 and publish it             *
//******************************************************************************
void I2CMultiplexerDriver::read_color_sensor()
{
   if (i2c_dev == nullptr) return;
   if (!select_channel(COLOR_SENSOR_CHANNEL)) return;

   // Check if data is ready (status register bit 0)
   uint8_t status_reg = TCS34725_COMMAND_BIT | TCS34725_STATUS;
   uint8_t status     = 0;
   if (i2cd_write_read(i2c_dev, COLOR_SENSOR_ADDR, &status_reg, 1, &status, 1) < 0)
   {
      reset_mux(); return;
   }
   if (!(status & 0x01))
   {
      // Data not ready yet, skip this tick
      reset_mux(); return;
   }

   // Read 8 bytes: C_low, C_high, R_low, R_high, G_low, G_high, B_low, B_high
   uint8_t data_reg = TCS34725_COMMAND_BIT | 0x10 | TCS34725_CDATAL; // Auto-increment mode
   uint8_t raw[8]   = {0};
   if (i2cd_write_read(i2c_dev, COLOR_SENSOR_ADDR, &data_reg, 1, raw, 8) < 0)
   {
      reset_mux(); return;
   }

   reset_mux();

   // Combine low and high bytes into 16-bit values
   auto message   = std_msgs::msg::UInt16MultiArray();
   message.data   = {
      (uint16_t)(raw[0] | (raw[1] << 8)),  // Clear
      (uint16_t)(raw[2] | (raw[3] << 8)),  // Red
      (uint16_t)(raw[4] | (raw[5] << 8)),  // Green
      (uint16_t)(raw[6] | (raw[7] << 8))   // Blue
   };
   color_sensor_publisher->publish(message);
}

//******************************************************************************
//*            Read distance from the VL53L5CX and publish it                 *
//******************************************************************************
void I2CMultiplexerDriver::read_distance_sensor()
{
   if (i2c_dev == nullptr) return;
   if (!select_channel(DISTANCE_SENSOR_CHANNEL)) return;

   // TODO: Implement VL53L5CX reading using ST ULD driver
   // The VL53L5CX requires ST's Ultra Lite Driver (ULD) for full functionality.
   // For now, publishing a placeholder value.
   RCLCPP_DEBUG(get_logger(), "VL53L5CX read not yet implemented.");

   reset_mux();

   auto message  = std_msgs::msg::Float32();
   message.data  = -1.0f; // Placeholder
   distance_sensor_publisher->publish(message);
}

//******************************************************************************
//*            Read distance from the VL53L4CD and publish it                 *
//******************************************************************************
void I2CMultiplexerDriver::read_duck_sensor()
{
   if (i2c_dev == nullptr) return;
   if (!select_channel(DUCK_SENSOR_CHANNEL)) return;

   // TODO: Implement VL53L4CD reading using ST ULD driver
   // Must wait for FIRMWARE_SYSTEM_STATUS register to return 0x03 after boot.
   RCLCPP_DEBUG(get_logger(), "VL53L4CD read not yet implemented.");

   reset_mux();

   auto message  = std_msgs::msg::Float32();
   message.data  = -1.0f; // Placeholder
   duck_sensor_publisher->publish(message);
}

//******************************************************************************
//*              Write a string to the SSD1306 OLED display                   *
//******************************************************************************
void I2CMultiplexerDriver::write_display(const std::string &text)
{
   if (i2c_dev == nullptr) return;
   if (!select_channel(DISPLAY_CHANNEL)) return;

   // TODO: Implement full SSD1306 framebuffer rendering
   // For now just log what would be displayed
   RCLCPP_INFO(get_logger(), "Display (TODO): %s", text.c_str());

   reset_mux();
}

//******************************************************************************
//*                   Callback for incoming display messages                   *
//******************************************************************************
void I2CMultiplexerDriver::display_callback(const std_msgs::msg::String::SharedPtr message)
{
   write_display(message->data);
}

//******************************************************************************
//*              Timer callback - poll all sensors every tick                  *
//******************************************************************************
void I2CMultiplexerDriver::timer_callback()
{
   read_color_sensor();
   read_distance_sensor();
   read_duck_sensor();
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
