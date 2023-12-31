# nRF52840 DK and sensor examples

## Building and flashing

Open nRF connect sdk command line tools and navigate to project
folder.

To build the project run the following command in the project root
(this readme's folder). This will build the project for the
nRF52840 DK.

```
west build --build-dir build -b nrf52840dk_nrf52840
```

After building the project flash it using the following command

```
west flash --build-dir build
```

## Wiring diagrams

Wiring diagrams for the examples is located in the repository root at folder called `documents`

## Changing sensor pins and analog channel configurations

To enable analog sensor set `CONFIG_DISABLE_GY61` in `prj.conf` to `n`

In this source code there is example code for reading analog
sensor using GY-61 3-Axis accelometer as the analog sensor.
The sensor specific code is found in `/src/sensors/gy61.c and .h`
files.

The sensors pins are configured in device specific `.overlay`
files, in this project it is `nrf52840dk_nrf52840.overlay`.
To change the pins used to connect the sensor, edit the
`overlay` file and change input-positive in the `&adc` channels
0-2. NOTE: The value has to be between `<>` like show below.

Channel sensor axis map

| Axis | ADC Channel |
| ---- | ----------- |
| x    | channel 0   |
| y    | channel 1   |
| z    | channel 2   |

Other values are used to set the channel register (nRF52840 has
12 analog channels), reference
voltage gain, refecence voltage source and acquisition time.
The gain and reference voltage used here give us a maximum of 3.6V
so our input can be any where between this and VSS.

The range is calculated using this formula
`Input range = reference / gain`, the available reference options
for nRF52840 are VDD/4 or internal 0.6V.

nRF52840 SAADC documentaion
https://infocenter.nordicsemi.com/topic/ps_nrf52840/saadc.html?cp=5_0_0_5_22
Last looked up 10.10.2023

Overlay example

```dts
&adc {
  #address-cells = <1>;
  #size-cells = <0>;

  channel@0 {
    reg = <0>;
    zephyr,gain = "ADC_GAIN_1_6";
    zephyr,reference = "ADC_REF_INTERNAL";
    zephyr,acquisition-time = <ADC_ACQ_TIME_DEFAULT>;
    zephyr,input-positive = <NRF_SAADC_AIN1>;
  };

  channel@1 {
    reg = <1>;
    zephyr,gain = "ADC_GAIN_1_6";
    zephyr,reference = "ADC_REF_INTERNAL";
    zephyr,acquisition-time = <ADC_ACQ_TIME_DEFAULT>;
    zephyr,input-positive = <NRF_SAADC_AIN2>;
  };

  channel@2 {
    reg = <2>;
    zephyr,gain = "ADC_GAIN_1_6";
    zephyr,reference = "ADC_REF_INTERNAL";
    zephyr,acquisition-time = <ADC_ACQ_TIME_DEFAULT>;
    zephyr,input-positive = <NRF_SAADC_AIN4>;
  };
};
```

The mapping for the pins on nRF52840 DK is show in the following
table

| GPIO  | Analog pin | DT name        |
| ----- | ---------- | -------------- |
| P0.03 | AIN1       | NRF_SAADC_AIN1 |
| P0.04 | AIN2       | NRF_SAADC_AIN2 |
| P0.28 | AIN4       | NRF_SAADC_AIN4 |
| P0.29 | AIN5       | NRF_SAADC_AIN5 |
| P0.30 | AIN6       | NRF_SAADC_AIN6 |
| P0.31 | AIN7       | NRF_SAADC_AIN7 |

nRF52840 DK analog pin map
https://infocenter.nordicsemi.com/index.jsp?topic=%2Fug_nrf52840_dk%2FUG%2Fdk%2Fhw_analog_pins.html.
Last looked up 10.10.2023

nRF52840 pin map
https://infocenter.nordicsemi.com/index.jsp?topic=%2Fps_nrf52840%2Fpin.html
Last looked up 10.10.2023

## I2C configuration

To enable I2C set `CONFIG_ENABLE_T67XX_I2C` in `prj.conf` to `y` and `CONFIG_ENABLE_T67XX_PWM` to `n`

There is also example code for communication with T67xx CO2 sensor using PWM and I2C.
The sensor specific I2C code is found in `/src/sensors/t67xx_i2c.c and .h`
files.

The pins are decided by the I2C controlles pin configuration and in this example default configuration is uses, the default pins for nRF52840 are `P0.26` and `P0.27`.
The I2C device address is configured using the same `.overlay`
files as the ADC, in this project `nrf52840dk_nrf52840.overlay`.

Example of I2C fragment

```dts
&i2c0 {
  t67xx: t67xx@15 {
    compatible = "i2c-device";
    reg = <0x15>; // I2C device address
    label = "T67XX";
  };
};
```

To change the device address for this example change the `reg` of the t67xx node and the part after `t67xx@` of the node.

## PWM configuration

To enable PWM set `CONFIG_ENABLE_T67XX_PWM` in `prj.conf` to `y` and `CONFIG_ENABLE_T67XX_I2C` to `n`

The T67xx PWM mode has no easy way to configure. All configurations have to be done by editing the source code directly. The code can be found in `/src/sensors/t67xx.c and .h`.

The pins are configured using two global variable `rising_edge` and `falling_edge` found in the `t67xx.c`
