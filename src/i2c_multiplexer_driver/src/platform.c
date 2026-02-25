/**
 * Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.
 *
 * Modified for Raspberry Pi 5 / libi2cd platform by Robto SRM, IEEE SoutheastCon 2026.
 *
 * The VL53L5CX uses 16-bit register addresses. All read/write functions must
 * send the address as two bytes (high byte first) before the data.
 * (VL53L5CX ULD API vl53l5cx_api.c - all register transactions use 16-bit addressing)
 ******************************************************************************
 */

#include "platform.h"
#include <stdlib.h>
#include <time.h>

/**
 * @brief Write a single byte to a 16-bit register address.
 * Sends: [addr_high, addr_low, value] in one I2C transaction.
 */
uint8_t VL53L5CX_WrByte(
      VL53L5CX_Platform *p_platform,
      uint16_t RegisterAdress,
      uint8_t value)
{
   uint8_t buf[3];
   uint8_t status;

   buf[0] = (uint8_t)(RegisterAdress >> 8);   // Register address high byte
   buf[1] = (uint8_t)(RegisterAdress & 0xFF); // Register address low byte
   buf[2] = value;

   status = (i2cd_write(p_platform->i2c_dev, p_platform->address, buf, 3) >= 0) ? 0 : 255;

   return status;
}

/**
 * @brief Read a single byte from a 16-bit register address.
 * Sends: [addr_high, addr_low], then reads 1 byte back.
 */
uint8_t VL53L5CX_RdByte(
      VL53L5CX_Platform *p_platform,
      uint16_t RegisterAdress,
      uint8_t *p_value)
{
   uint8_t addr[2];
   uint8_t status;

   addr[0] = (uint8_t)(RegisterAdress >> 8);   // Register address high byte
   addr[1] = (uint8_t)(RegisterAdress & 0xFF); // Register address low byte

   status = (i2cd_write_read(p_platform->i2c_dev, p_platform->address, addr, 2, p_value, 1) >= 0) ? 0 : 255;

   return status;
}

/**
 * @brief Write multiple bytes starting at a 16-bit register address.
 * Sends: [addr_high, addr_low, data[0], data[1], ...] in one I2C transaction.
 * Allocates a temporary buffer to prepend the address to the data.
 */
uint8_t VL53L5CX_WrMulti(
      VL53L5CX_Platform *p_platform,
      uint16_t RegisterAdress,
      uint8_t *p_values,
      uint32_t size)
{
   uint8_t  addr[2];
   uint8_t *buf;
   uint8_t  status;

   // Allocate buffer: 2 address bytes + data
   buf = (uint8_t *)malloc(size + 2);
   if (buf == NULL)
      return 255;

   addr[0] = (uint8_t)(RegisterAdress >> 8);   // Register address high byte
   addr[1] = (uint8_t)(RegisterAdress & 0xFF); // Register address low byte

   buf[0] = addr[0];
   buf[1] = addr[1];
   memcpy(&buf[2], p_values, size);

   status = (i2cd_write(p_platform->i2c_dev, p_platform->address, buf, size + 2) >= 0) ? 0 : 255;

   free(buf);

   return status;
}

/**
 * @brief Read multiple bytes starting at a 16-bit register address.
 * Sends: [addr_high, addr_low], then reads `size` bytes back.
 */
uint8_t VL53L5CX_RdMulti(
      VL53L5CX_Platform *p_platform,
      uint16_t RegisterAdress,
      uint8_t *p_values,
      uint32_t size)
{
   uint8_t addr[2];
   uint8_t status;

   addr[0] = (uint8_t)(RegisterAdress >> 8);   // Register address high byte
   addr[1] = (uint8_t)(RegisterAdress & 0xFF); // Register address low byte

   status = (i2cd_write_read(p_platform->i2c_dev, p_platform->address, addr, 2, p_values, size) >= 0) ? 0 : 255;

   return status;
}

/**
 * @brief Optional hardware reset. Not implemented - no GPIO control wired.
 * Returns 0 (success) without doing anything.
 */
uint8_t VL53L5CX_Reset_Sensor(
      VL53L5CX_Platform *p_platform)
{
   (void)p_platform;

   return 0;
}

/**
 * @brief Swap bytes within each 4-byte word in the buffer.
 * Required by the ULD firmware loader for endian conversion.
 * (VL53L5CX ULD API platform.c - ST reference implementation)
 */
void VL53L5CX_SwapBuffer(
      uint8_t  *buffer,
      uint16_t  size)
{
   uint32_t i;
   uint32_t tmp;

   for (i = 0; i < size; i = i + 4)
   {
      tmp = (
         buffer[i]     << 24)
         | (buffer[i+1] << 16)
         | (buffer[i+2] << 8)
         | (buffer[i+3]);

      memcpy(&(buffer[i]), &tmp, 4);
   }

   return;
}

/**
 * @brief Wait for a given number of milliseconds.
 * Uses nanosleep for millisecond-accurate delay on Linux.
 */
uint8_t VL53L5CX_WaitMs(
      VL53L5CX_Platform *p_platform,
      uint32_t TimeMs)
{
   struct timespec ts;
   uint8_t         status;

   (void)p_platform;

   ts.tv_sec  = TimeMs / 1000;
   ts.tv_nsec = (TimeMs % 1000) * 1000000L;

   status = (nanosleep(&ts, NULL) == 0) ? 0 : 255;

   return status;
}
