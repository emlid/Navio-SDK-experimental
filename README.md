
**This code is experimental. Please use https://github.com/emlid/Navio instead!**

# Navio SDK

Main goal of this SDK is to introduce event-driven (https://en.wikipedia.org/wiki/Event-driven_programming) approach for hardware interaction and application logic.

## Compilling

Requirements:

	c++11 compiller (clang and gcc is ok)
	cmake
	make

On target device:

	cmake .
	make

Cross compiling with toolchain:

	cmake . -DCMAKE_TOOLCHAIN_FILE=<here goes your toolchain decription file>.cmake
	make

Toolchain description file may look like that(static toolchain linking):

	set( CMAKE_SYSTEM_NAME Linux )
	set( CMAKE_C_COMPILER "${CMAKE_CURRENT_LIST_DIR}/armv6-rpi-linux-gnueabi/bin/armv6-rpi-linux-gnueabi-gcc" )
	set( CMAKE_CXX_COMPILER "${CMAKE_CURRENT_LIST_DIR}/armv6-rpi-linux-gnueabi/bin/armv6-rpi-linux-gnueabi-g++" )
	set( CMAKE_SYSROOT "${CMAKE_CURRENT_LIST_DIR}/armv6-rpi-linux-gnueabi/armv6-rpi-linux-gnueabi/sysroot" )
	set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER )
	set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY )
	set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY )
	set( CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY )
	set( CMAKE_FIND_LIBRARY_SUFFIXES ".a" )
	set( CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -s" CACHE STRING "thread cflags" )

## Documentation and Examples

Documentation can be generated from source code itself, just use doxygen to generate it:

	doxygen doxygen.conf

Also we have some examples which you can use to test your hardware, you can find them in examples folder.
Before using any examples make sure that your i2c and spi drivers are loaded.

## Device Drivers

* ADS1115 - 4 channel 16-bit ADC
* PCA9685 - 16-channel, 12-bit PWM
* MS5611 - Barometric pressure sensor
* BMP180 - Barometric pressure sensor
* L3GD20H - Three-axis gyroscope
* SSD1306 - OLED display

# TODO

* SPI bus driver
* MPU9250
* U-blox M8N
* MB85RC FRAM
* HAT EEPROM
* LSM303DLHC

