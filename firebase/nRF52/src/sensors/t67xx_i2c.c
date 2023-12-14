#include "t67xx_i2c.h"

#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(app, LOG_LEVEL_DBG);

#define I2C0_NODE DT_NODELABEL(t67xx)
static const struct i2c_dt_spec dev = I2C_DT_SPEC_GET(I2C0_NODE);

#define DUMP(data) LOG_ERR("Invalid device response: %02x %02x %02x %02x", data[0], data[1], data[2], data[3])

int init_t67xx_i2c() {
  if (!device_is_ready(dev.bus)) {
    LOG_ERR("I2C not ready");
    return -1;
  }

  const uint8_t cmd[] = {0x04, 0x13, 0x89, 0x00, 0x01};
  i2c_write_dt(&dev, cmd, sizeof(cmd));  // Request for firmaware version
  k_msleep(10);                          // Wait for sensor to prepare response (10ms)

  uint8_t res[4];
  i2c_read_dt(&dev, res, sizeof(res));
  if (res[0] != 0x04 && res[1] != 0x02) {  // Test first two bytes for command and response size
    DUMP(res);
    return -2;
  }

  const uint8_t reset[] = {0x05, 0x03, 0xe8, 0xff, 0x00};
  i2c_write_dt(&dev, reset, sizeof(reset)); // Reset the device

  return 0;
}

uint16_t t67xx_read_status() {
  const uint8_t cmd[] = {0x04, 0x13, 0x8a, 0x00, 0x01};
  i2c_write_dt(&dev, cmd, sizeof(cmd));  // Request device status
  k_msleep(10);                          // Wait for sensor to prepare response (10ms)

  uint8_t data[4];
  i2c_read_dt(&dev, data, sizeof(data));
  if (data[0] != 0x04 && data[1] != 0x02) {  // Verify the response header
    DUMP(data);
    return -2;
  }

  return data[2] << 8 | data[3];
}

uint16_t t67xx_read_ppm() {
  const uint8_t cmd[] = {0x04, 0x13, 0x8b, 0x00, 0x01};
  i2c_write_dt(&dev, cmd, sizeof(cmd));  // Request current ppm
  k_msleep(10);                          // Wait for sensor to prepare response (10ms)

  uint8_t data[4];
  i2c_read_dt(&dev, data, sizeof(data));
  if (data[0] != 0x04 && data[1] != 0x02) {  // Verify the response header
    DUMP(data);
    return -2;
  }

  LOG_HEXDUMP_DBG(data, sizeof(data), "DATA: ");
  return data[2] << 8 | data[3];
}
