/*
 ******************************************************************************
 * @file    lps22hh_reg.h
 * @author  Sensors Software Solution Team
 * @brief   This file contains all the functions prototypes for the
 *          lps22hh_reg.c driver.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef LPS22HH_REGS_OUT_H
#define LPS22HH_REGS_OUT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <math.h>

/** @addtogroup LPS22HH
  * @{
  *
  */


/**
  * @}
  *
  */
#define PROPERTY_ENABLE                 (1U)

/** @addtogroup  Interfaces_Functions
  * @brief       This section provide a set of functions used to read and
  *              write a generic register of the device.
  *              MANDATORY: return 0 -> no Error.
  * @{
  *
  */

typedef int32_t (*stmdev_write_ptr)(void *, uint8_t, uint8_t *,
                                    uint16_t);
typedef int32_t (*stmdev_read_ptr) (void *, uint8_t, uint8_t *,
                                    uint16_t);

typedef struct {
  /** Component mandatory fields **/
  stmdev_write_ptr  write_reg;
  stmdev_read_ptr   read_reg;
  /** Customizable optional pointer **/
  void *handle;
} stmdev_ctx_t;


/** @defgroup    Generic address-data structure definition
  * @brief       This structure is useful to load a predefined configuration
  *              of a sensor.
  *              You can create a sensor configuration by your own or using
  *              Unico / Unicleo tools available on STMicroelectronics
  *              web site.
  *
  * @{
  *
  */


/**
  * @}
  *
  */

/** @defgroup LPS22HH_Infos
  * @{
  *
  */

/** I2C Device Address 8 bit format  if SA0=0 -> B9 if SA0=1 -> BB **/
//#define LPS22HH_I2C_ADD_H   0xBAU
//#define LPS22HH_I2C_ADD_L   0xB8U

/** Device Identification (Who am I) **/
#define LPS22HH_ID                              0xB3U

/**
  * @}
  *
  */


#define LPS22HH_WHO_AM_I                        0x0FU
#define LPS22HH_CTRL_REG1                       0x10U
typedef struct {
  uint8_t sim                             : 1;
  uint8_t bdu                             : 1;
  uint8_t lpfp_cfg                        : 2;  /* en_lpfp + lpfp_cfg */
  uint8_t odr                             : 3;
  uint8_t not_used_01                     : 1;
} lps22hh_ctrl_reg1_t;

#define LPS22HH_CTRL_REG2                       0x11U
typedef struct {
  uint8_t one_shot                        : 1;
  uint8_t low_noise_en                    : 1;
  uint8_t swreset                         : 1;
  uint8_t not_used_01                     : 1;
  uint8_t if_add_inc                      : 1;
  uint8_t pp_od                           : 1;
  uint8_t int_h_l                         : 1;
  uint8_t boot                            : 1;
} lps22hh_ctrl_reg2_t;

#define LPS22HH_CTRL_REG3                       0x12U
typedef struct {
  uint8_t int_s                           : 2;
  uint8_t drdy                            : 1;
  uint8_t int_f_ovr                       : 1;
  uint8_t int_f_wtm                       : 1;
  uint8_t int_f_full                      : 1;
  uint8_t not_used_01                     : 2;
} lps22hh_ctrl_reg3_t;


#define LPS22HH_STATUS                          0x27U
typedef struct {
  uint8_t p_da                            : 1;
  uint8_t t_da                            : 1;
  uint8_t not_used_01                     : 2;
  uint8_t p_or                            : 1;
  uint8_t t_or                            : 1;
  uint8_t not_used_02                     : 2;
} lps22hh_status_t;

#define LPS22HH_PRESS_OUT_XL                    0x28U
#define LPS22HH_TEMP_OUT_L                      0x2BU
#define LPS22HH_TEMP_OUT_H                      0x2CU

/**
  * @defgroup LPS22HH_Register_Union
  * @brief    This union group all the registers that has a bitfield
  *           description.
  *           This union is useful but not need by the driver.
  *
  *           REMOVING this union you are compliant with:
  *           MISRA-C 2012 [Rule 19.2] -> " Union are not allowed "
  *
  * @{
  *
  */
typedef union {
  lps22hh_ctrl_reg1_t            ctrl_reg1;
  lps22hh_ctrl_reg2_t            ctrl_reg2;
  lps22hh_ctrl_reg3_t            ctrl_reg3;
  lps22hh_status_t               status;
  uint8_t                        byte;
} lps22hh_reg_t;

/**
  * @}
  *
  */

int32_t lps22hh_read_reg(stmdev_ctx_t *ctx, uint8_t reg,
                         uint8_t *data,
                         uint16_t len);
int32_t lps22hh_write_reg(stmdev_ctx_t *ctx, uint8_t reg,
                          uint8_t *data,
                          uint16_t len);

float_t lps22hh_from_lsb_to_hpa(uint32_t lsb);
float_t lps22hh_from_lsb_to_celsius(int16_t lsb);

int32_t lps22hh_block_data_update_set(stmdev_ctx_t *ctx, uint8_t val);

typedef enum {
  LPS22HH_POWER_DOWN          = 0x00,
  LPS22HH_10_Hz_LOW_NOISE     = 0x12,
} lps22hh_odr_t;
int32_t lps22hh_data_rate_set(stmdev_ctx_t *ctx, lps22hh_odr_t val);
int32_t lps22hh_data_rate_get(stmdev_ctx_t *ctx, lps22hh_odr_t *val);

int32_t lps22hh_pressure_raw_get(stmdev_ctx_t *ctx, uint32_t *buff);

int32_t lps22hh_temperature_raw_get(stmdev_ctx_t *ctx, int16_t *buff);

int32_t lps22hh_device_id_get(stmdev_ctx_t *ctx, uint8_t *buff);

int32_t lps22hh_reset_set(stmdev_ctx_t *ctx, uint8_t val);
int32_t lps22hh_reset_get(stmdev_ctx_t *ctx, uint8_t *val);

/**
  * @}
  *
  */

#ifdef __cplusplus
}
#endif

#endif /*LPS22HH_REGS_OUT_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
