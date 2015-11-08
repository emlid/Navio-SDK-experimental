#include "vz89.h"
#include "i2c.h"
#include "log.h"

#define VZ89_COMMAND_SET_PPMCO2 0x08
#define VZ89_COMMAND_GET_STATUS 0x09

VZ89::VZ89():
    VZ89(VZ89_I2C_ADDRESS, I2C::getDefault())
{
}

VZ89::VZ89(uint8_t address, I2C *bus):
    _i2c(bus), _address(address)
{
}

VZ89::~VZ89()
{
}

int VZ89::getStatus(float &co2, uint8_t &reactivity, float &tvoc)
{
    uint8_t data[6];
    if (_i2c->readBytes(_address, VZ89_COMMAND_GET_STATUS, 6, data) < 0) {
        Error() << "Failed to read device status";
        return -1;
    }

    if (data[0] < 13 || data[1] < 13 || data[2] < 13) {
        Error() << "device is not ready";
        return -1;
    }

    co2 = (data[0] - 13) * (1600.0 / 229) + 400;
    reactivity = data[1];
    tvoc = (data[2] - 13) * (1000.0/229);

    return 0;
}
