/*
 ******************************************************************************
 * @file    lps22hh_reg.c
 * @author  Sensors Software Solution Team
 * @brief   LPS22HH driver file
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

#include "lps22hb_reg_In.h"

/**
  * @defgroup  LPS22HH
  * @brief     This file provides a set of functions needed to drive the
  *            lps22hh enhanced inertial module.
  * @{
  *
  */

/**
  * @defgroup  LPS22HH_Interfaces_Functions
  * @brief     This section provide a set of functions used to read and
  *            write a generic register of the device.
  *            MANDATORY: return 0 -> no Error.
  * @{
  *
  */

/**
  * @brief  Read generic device register
  *
  * @param  ctx   read / write interface definitions(ptr)
  * @param  reg   register to read
  * @param  data  pointer to buffer that store the data read(ptr)
  * @param  len   number of consecutive register to read
  * @retval       interface status (MANDATORY: return 0 -> no Error)
  *
  */
int32_t lps22hh_read_reg(stmdev_ctx_t *ctx, uint8_t reg,
                         uint8_t *data,
                         uint16_t len)
{
  int32_t ret;
  ret = ctx->read_reg(ctx->handle, reg, data, len);
  return ret;
}

/**
  * @brief  Write generic device register
  *
  * @param  ctx   read / write interface definitions(ptr)
  * @param  reg   register to write
  * @param  data  pointer to data to write in register reg(ptr)
  * @param  len   number of consecutive register to write
  * @retval       interface status (MANDATORY: return 0 -> no Error)
  *
  */
int32_t lps22hh_write_reg(stmdev_ctx_t *ctx, uint8_t reg,
                          uint8_t *data,
                          uint16_t len)
{
  int32_t ret;
  ret = ctx->write_reg(ctx->handle, reg, data, len);
  return ret;
}

/**
  * @}
  *
  */

/**
  * @defgroup    LPS22HH_Sensitivity
  * @brief       These functions convert raw-data into engineering units.
  * @{
  *
  */
float_t lps22hh_from_lsb_to_hpa(uint32_t lsb)
{
  return ( (float_t) lsb / 1048576.0f );
}

float_t lps22hh_from_lsb_to_celsius(int16_t lsb)
{
  return ( (float_t) lsb / 100.0f );
}

/**
  * @}
  *
  */

/**
  * @defgroup  LPS22HH_Data_Generation
  * @brief     This section groups all the functions concerning
  *            data generation.
  * @{
  *
  */

/**
  * @brief  Block Data Update.[set]
  *
  * @param  ctx      read / write interface definitions
  * @param  val      change the values of bdu in reg CTRL_REG1
  * @retval          interface status (MANDATORY: return 0 -> no Error)
  *
  */
int32_t lps22hh_block_data_update_set(stmdev_ctx_t *ctx, uint8_t val)
{
  lps22hh_ctrl_reg1_t reg;
  int32_t ret;
  ret = lps22hh_read_reg(ctx, LPS22HH_CTRL_REG1, (uint8_t *) &reg, 1);

  if (ret == 0) {
    reg.bdu = val;
    ret = lps22hh_write_reg(ctx, LPS22HH_CTRL_REG1, (uint8_t *) &reg, 1);
  }

  return ret;
}

/**
  * @brief  Output data rate selection.[set]
  *
  * @param  ctx      read / write interface definitions
  * @param  val      change the values of odr in reg CTRL_REG1
  * @retval          interface status (MANDATORY: return 0 -> no Error)
  *
  */
int32_t lps22hh_data_rate_set(stmdev_ctx_t *ctx, lps22hh_odr_t val)
{
  lps22hh_ctrl_reg1_t ctrl_reg1;
  lps22hh_ctrl_reg2_t ctrl_reg2;
  int32_t ret;
  ret = lps22hh_read_reg(ctx, LPS22HH_CTRL_REG1, (uint8_t *)&ctrl_reg1,
                         1);

  if (ret == 0) {
    ret = lps22hh_read_reg(ctx, LPS22HH_CTRL_REG2, (uint8_t *)&ctrl_reg2,
                           1);
  }

  if (ret == 0) {
    ctrl_reg1.odr = (uint8_t)val & 0x07U;
    ret = lps22hh_write_reg(ctx, LPS22HH_CTRL_REG1, (uint8_t *)&ctrl_reg1,
                            1);
  }

  if (ret == 0) {
    ctrl_reg2.low_noise_en = ((uint8_t)val & 0x10U) >> 4;
    ctrl_reg2.one_shot = ((uint8_t)val & 0x08U) >> 3;
    ret = lps22hh_write_reg(ctx, LPS22HH_CTRL_REG2, (uint8_t *)&ctrl_reg2,
                            1);
  }

  return ret;
}

/**
  * @brief  Output data rate selection.[get]
  *
  * @param  ctx      read / write interface definitions
  * @param  val      Get the values of odr in reg CTRL_REG1
  * @retval          interface status (MANDATORY: return 0 -> no Error)
  *
  */
int32_t lps22hh_data_rate_get(stmdev_ctx_t *ctx, lps22hh_odr_t *val)
{
  lps22hh_ctrl_reg1_t ctrl_reg1;
  lps22hh_ctrl_reg2_t ctrl_reg2;
  int32_t ret;
  ret = lps22hh_read_reg(ctx, LPS22HH_CTRL_REG1, (uint8_t *)&ctrl_reg1,
                         1);

  if (ret == 0) {
    ret = lps22hh_read_reg(ctx, LPS22HH_CTRL_REG2, (uint8_t *)&ctrl_reg2,
                           1);
  }

  if (ret == 0) {
    ret = lps22hh_read_reg(ctx, LPS22HH_CTRL_REG2, (uint8_t *)&ctrl_reg2,
                           1);

    switch (((ctrl_reg2.low_noise_en << 4) + (ctrl_reg2.one_shot << 3) +
             ctrl_reg1.odr )) {
                 
      case LPS22HH_10_Hz_LOW_NOISE:
        *val = LPS22HH_10_Hz_LOW_NOISE;
        break;

      default:
        *val = LPS22HH_POWER_DOWN;
        break;
    }
  }

  return ret;
}

/**
  * @}
  *
  */

/**
  * @defgroup  LPS22HH_Data_Output
  * @brief     This section groups all the data output functions.
  * @{
  *
  */

/**
  * @brief  Pressure output value.[get]
  *
  * @param  ctx      read / write interface definitions
  * @param  buff     buffer that stores data read
  * @retval          interface status (MANDATORY: return 0 -> no Error)
  *
  */
int32_t lps22hh_pressure_raw_get(stmdev_ctx_t *ctx, uint32_t *buff)
{
  int32_t ret;
  uint8_t reg[3];
  ret =  lps22hh_read_reg(ctx, LPS22HH_PRESS_OUT_XL, reg, 3);
  *buff = reg[2];
  *buff = (*buff * 256) + reg[1];
  *buff = (*buff * 256) + reg[0];
  *buff *= 256;
  return ret;
}

/**
  * @brief  Temperature output value.[get]
  *
  * @param  ctx      read / write interface definitions
  * @param  buff     buffer that stores data read
  * @retval          interface status (MANDATORY: return 0 -> no Error)
  *
  */
int32_t lps22hh_temperature_raw_get(stmdev_ctx_t *ctx, int16_t *buff)
{
  int32_t ret;
  uint8_t reg[2];
  ret =  lps22hh_read_reg(ctx, LPS22HH_TEMP_OUT_L, reg, 2);
  *buff = reg[1];
  *buff = (*buff * 256) + reg[0];
  return ret;
}

/**
  * @}
  *
  */

/**
  * @defgroup  LPS22HH_Common
  * @brief     This section groups common useful functions.
  * @{
  *
  */

/**
  * @brief  DeviceWhoamI[get]
  *
  * @param  ctx      read / write interface definitions
  * @param  buff     buffer that stores data read
  * @retval          interface status (MANDATORY: return 0 -> no Error)
  *
  */
int32_t lps22hh_device_id_get(stmdev_ctx_t *ctx, uint8_t *buff)
{
  int32_t ret;
  ret =  lps22hh_read_reg(ctx, LPS22HH_WHO_AM_I, buff, 1);
  return ret;
}

/**
  * @brief  Software reset. Restore the default values
  *         in user registers.[set]
  *
  * @param  ctx      read / write interface definitions
  * @param  val      change the values of swreset in reg CTRL_REG2
  * @retval          interface status (MANDATORY: return 0 -> no Error)
  *
  */
int32_t lps22hh_reset_set(stmdev_ctx_t *ctx, uint8_t val)
{
  lps22hh_ctrl_reg2_t reg;
  int32_t ret;
  ret = lps22hh_read_reg(ctx, LPS22HH_CTRL_REG2, (uint8_t *) &reg, 1);

  if (ret == 0) {
    reg.swreset = val;
    ret = lps22hh_write_reg(ctx, LPS22HH_CTRL_REG2, (uint8_t *) &reg, 1);
  }

  return ret;
}

/**
  * @brief   Software reset. Restore the default values
  *          in user registers.[get]
  *
  * @param  ctx      read / write interface definitions
  * @param  val      change the values of swreset in reg CTRL_REG2
  * @retval          interface status (MANDATORY: return 0 -> no Error)
  *
  */
int32_t lps22hh_reset_get(stmdev_ctx_t *ctx, uint8_t *val)
{
  lps22hh_ctrl_reg2_t reg;
  int32_t ret;
  ret = lps22hh_read_reg(ctx, LPS22HH_CTRL_REG2, (uint8_t *) &reg, 1);
  *val = reg.swreset;
  return ret;
}

/**
  * @}
  *
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
