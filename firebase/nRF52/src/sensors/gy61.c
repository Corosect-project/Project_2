/**
 * @file gy61.c
 * @author Astru43
 * @brief gy-61 3-Axis Accelerometer handler
 * @version 0.1
 * @date 2023-10-05
 *
 * @copyright Copyright (c) 2023 MIT
 *
 */
#include "gy61.h"

#include <zephyr/logging/log.h>

#include "../adc.h"
#include "../util.h"

LOG_MODULE_DECLARE(app, LOG_LEVEL_DBG);

static const struct adc_channel_cfg ch0_cfg_dt = ADC_CHANNEL_CFG_DT(DT_CHILD(DT_NODELABEL(adc), channel_0));
static const struct adc_channel_cfg ch1_cfg_dt = ADC_CHANNEL_CFG_DT(DT_CHILD(DT_NODELABEL(adc), channel_1));
static const struct adc_channel_cfg ch2_cfg_dt = ADC_CHANNEL_CFG_DT(DT_CHILD(DT_NODELABEL(adc), channel_2));

int gy61_init() {
  LOG_DBG("Initializing gy-61 sensor");
  int err = init_channel(&ch0_cfg_dt);
  if (ERROR(err)) return err;

  err = init_channel(&ch1_cfg_dt);
  if (ERROR(err)) return err;

  err = init_channel(&ch2_cfg_dt);
  if (ERROR(err)) return err;

  return 0;
}

int gy61_read(uint16_t out_raw[3], size_t out_raw_len, double out[3], size_t out_len) {
  if (out_raw_len < sizeof(uint16_t) * 3) return -ENOMEM;
  if (out_len < sizeof(double) * 3) return -ENOMEM;

  struct adc_sequence seq = {
    .channels = 0b0111,
    .calibrate = true,
    .resolution = 12,
    .buffer = out_raw,
    .buffer_size = out_raw_len,
  };

  int err = read(&seq);
  if (ERROR(err)) return err;

  LOG_DBG("%d, %d, %d", out_raw[0], out_raw[1], out_raw[2]);

  out[0] = 0.0266250134946285 * out_raw[0] - 50.0822420503627;
  out[1] = 0.0265549994892716 * out_raw[1] - 49.0641972785777;
  out[2] = 0.0266211855123551 * out_raw[2] - 52.4387070131872;

  LOG_DBG("%.3f, %.3f, %.3f", out[0], out[1], out[2]);

  return 0;
}
