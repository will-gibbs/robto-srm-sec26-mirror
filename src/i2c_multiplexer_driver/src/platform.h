/**
 * Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.
 *
 * Modified for Raspberry Pi 5 / libi2cd platform by Robto SRM, IEEE SoutheastCon 2026.
 ******************************************************************************
 */

#ifndef _PLATFORM_H_
#define _PLATFORM_H_
#pragma once

#include <stdint.h>
#include <string.h>

// libi2cd handle for I2C communication on Raspberry Pi
#include "i2cd.h"

/**
 * @brief VL53L5CX platform structure.
 * Extended from ST default to include libi2cd device handle for Raspberry Pi.
 */
typedef struct
{
   uint16_t      address; // I2C address of the VL53L5CX sensor
   struct i2cd  *i2c_dev; // libi2cd device handle (opened externally, shared with mux driver)
} VL53L5CX_Platform;

/*
 * @brief Number of targets per zone sent through I2C.
 * Value must be between 1 and 4. Lower = less RAM and fewer I2C bytes.
 * (VL53L5CX ULD API platform.h)
 */
#define VL53L5CX_NB_TARGET_PER_ZONE 1U

/*
 * @brief Disable unused result fields to reduce I2C traffic and RAM.
 * We only need distance_mm, target_status, and nb_target_detected.
 * (VL53L5CX ULD API Example_6_I2C_and_RAM_optimization.c)
 */
#define VL53L5CX_DISABLE_AMBIENT_PER_SPAD
#define VL53L5CX_DISABLE_NB_SPADS_ENABLED
#define VL53L5CX_DISABLE_SIGNAL_PER_SPAD
#define VL53L5CX_DISABLE_RANGE_SIGMA_MM
#define VL53L5CX_DISABLE_REFLECTANCE_PERCENT
#define VL53L5CX_DISABLE_MOTION_INDICATOR

uint8_t VL53L5CX_RdByte(
      VL53L5CX_Platform *p_platform,
      uint16_t RegisterAdress,
      uint8_t *p_value);

uint8_t VL53L5CX_WrByte(
      VL53L5CX_Platform *p_platform,
      uint16_t RegisterAdress,
      uint8_t value);

uint8_t VL53L5CX_RdMulti(
      VL53L5CX_Platform *p_platform,
      uint16_t RegisterAdress,
      uint8_t *p_values,
      uint32_t size);

uint8_t VL53L5CX_WrMulti(
      VL53L5CX_Platform *p_platform,
      uint16_t RegisterAdress,
      uint8_t *p_values,
      uint32_t size);

uint8_t VL53L5CX_Reset_Sensor(
      VL53L5CX_Platform *p_platform);

void VL53L5CX_SwapBuffer(
      uint8_t  *buffer,
      uint16_t  size);

uint8_t VL53L5CX_WaitMs(
      VL53L5CX_Platform *p_platform,
      uint32_t TimeMs);

#endif // _PLATFORM_H_
