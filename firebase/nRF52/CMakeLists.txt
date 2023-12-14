# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(firebase)

target_sources(
  app
  PRIVATE
    src/main.c
    src/config.c 
    src/mqtt.c
)

target_sources_ifdef(CONFIG_ADC app PRIVATE src/adc.c)
target_sources_ifndef(CONFIG_DISABLE_GY61 app PRIVATE src/sensors/gy61.c)

if(CONFIG_ENABLE_T67XX_I2C)
  target_sources(app PRIVATE src/sensors/t67xx_i2c.c)
elseif(CONFIG_ENABLE_T67XX_PWM)
  target_sources(app PRIVATE src/sensors/t67xx.c)
endif()
