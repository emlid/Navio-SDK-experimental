
**This code is not yet ready. Please use https://github.com/emlid/Navio instead!**

# TODO

* SPI bus driver
* MPU9250
* MS5611
* U-blox M8N
* ADS1115
* MB85RC FRAM
* HAT EEPROM
* LSM303DLHC

# Navio SDK

Main goal of this SDK is to introduce event-driven (https://en.wikipedia.org/wiki/Event-driven_programming) approach for hardware interaction and application logic. SDK was designed to work efficiently with navio/navio+ boards, at the same time you are free to use it with any hardware you want to(currently we also provide drivers for adafruit i2c 10dof and PWM). Also feel free to contribute any ideas/code which you think worth it.

## Compilling

On raspberry pi:

	cmake .
	make

Cross compiling with toolchain:

	cmake . -DCMAKE_TOOLCHAIN_FILE=<here goes your toolchain decription file>.cmake
	make

Toolchain description file should look like that:

	set(CMAKE_SYSTEM_NAME Linux)
	set(CMAKE_SYSROOT /Toolchains/arm-rpi-linux-gnueabi/arm-rpi-linux-gnueabi/sysroot)
	set(CMAKE_C_COMPILER /Toolchains/arm-rpi-linux-gnueabi/bin/arm-rpi-linux-gnueabi-gcc)
	set(CMAKE_CXX_COMPILER /Toolchains/arm-rpi-linux-gnueabi/bin/arm-rpi-linux-gnueabi-g++)
	set(CMAKE_STAGING_PREFIX /RPI-root/)
	set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
	set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
	set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
	set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

## Documentation and Examples

Precompiled documentation can be found in documentation folder.
Use doxygen if you want to update it.

Also we have some examples which you can use to test your hardware.
Before using any examples make sure that your i2c and spi drivers are loaded.

* navio - navio/navio+ boards with MPU9250, MS5611, U-blox M8N, ADS1115, PCA9685, MB85RC FRAM, HAT EEPROM.
* adafruit - combination of product id: 815 + 1604: PCA9685, BMP180, L3GD20H, LSM303DLHC.
