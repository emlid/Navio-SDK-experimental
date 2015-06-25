#include "i2c.h"
#include "log.h"

#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <cassert>

static I2C* _default_i2c = nullptr;

I2C::I2C():
    _fd(-1)
{
    if (_default_i2c == nullptr) {
        _default_i2c = this;
    }
}

I2C::~I2C()
{
    if (_default_i2c == this) {
        _default_i2c = nullptr;
    }

    if (_fd != -1) {
        close(_fd); _fd = -1;
    }
}

int I2C::openDevice(const char *dev_path)
{
    assert(_fd == -1);
    Debug() << "Opening i2c dev" << dev_path;
    _fd = open(dev_path, O_RDWR);
    if (_fd < 0) {
        Error() << "Failed to open device. errno" << errno << strerror(errno);
        return -1;
    }

    return 0;
}

int I2C::readByte(uint8_t device_address, uint8_t register_address, uint8_t *data)
{
    return readBytes(device_address, register_address, 1, data);
}

int I2C::readBytes(uint8_t device_address, uint8_t register_address, uint8_t size, uint8_t *data)
{
    assert(data != 0);
    i2c_msg read_reg[2]={
        { device_address, I2C_M_WR, 1, &register_address },
        { device_address, I2C_M_RD, size, data }
    };

    i2c_rdwr_ioctl_data messages;
    messages.nmsgs = 2;
    messages.msgs = read_reg;

    return readWrite(messages);
}

int I2C::writeByte(uint8_t device_address, uint8_t register_address, uint8_t *data)
{
    return writeBytes(device_address, register_address, 1, data);
}

int I2C::writeBytes(uint8_t device_address, uint8_t register_address, uint8_t size, uint8_t *data)
{
    assert(data != 0);
    if (size > 127) {
        Error() << "Byte write count" << size << "> 127";
        return -1;
    }

    int16_t r_size = size + 1;
    uint8_t r_data[r_size];
    r_data[0] = register_address;
    memcpy(r_data+1, data, size);

    i2c_msg message [1]= {
        { device_address, I2C_M_WR, r_size, r_data },
    };

    i2c_rdwr_ioctl_data messages;
    messages.nmsgs = 1;
    messages.msgs = message;

    return readWrite(messages);
}

int I2C::readWrite(i2c_rdwr_ioctl_data &messages)
{
    int ret = ioctl(_fd, I2C_RDWR, &messages);
    if (ret < 0) {
        Error() << "Failed to communicate with device:" << strerror(errno);
        return -1;
    } else if ((uint32_t)ret != messages.nmsgs) {
        Warn() << "No all messages was processed. Expected" <<  messages.nmsgs << "got" << ret;
        return -1;
    }
    return 0;
}

I2C* I2C::getDefault()
{
    assert(_default_i2c != nullptr);
    return _default_i2c;
}
