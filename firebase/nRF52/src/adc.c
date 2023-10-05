#include "adc.h"

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(app, LOG_LEVEL_DBG);

const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(adc));

int init_channel(const struct adc_channel_cfg *chan) {
  int err = adc_channel_setup(dev, chan);
  LOG_DBG("%d", err);
  return err;
}

int read(const struct adc_sequence *seq) {
  int err = adc_read(dev, seq);
  LOG_DBG("%d", err);
  return err;
}
