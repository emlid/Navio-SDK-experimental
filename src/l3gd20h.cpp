#include "l3gd20h.h"

#include "poller.h"
#include "timer.h"
#include "i2c.h"
#include "log.h"

#define L3GD20H_RA_WHO_AM_I             0x0F
#define L3GD20H_RA_CTRL1                0x20
#define L3GD20H_RA_CTRL2                0x21
#define L3GD20H_RA_CTRL3                0x22
#define L3GD20H_RA_CTRL4                0x23
#define L3GD20H_RA_CTRL5                0x24
#define L3GD20H_RA_REFERENCE            0x25
#define L3GD20H_RA_OUT_TEMP             0x26
#define L3GD20H_RA_STATUS               0x27
#define L3GD20H_RA_OUT_X_L              0x28
#define L3GD20H_RA_OUT_X_H              0x29
#define L3GD20H_RA_OUT_Y_L              0x2A
#define L3GD20H_RA_OUT_Y_H              0x2B
#define L3GD20H_RA_OUT_Z_L              0x2C
#define L3GD20H_RA_OUT_Z_H              0x2D
#define L3GD20H_RA_FIFO_CTRL            0x2E
#define L3GD20H_RA_FIFO_SRC             0x2F
#define L3GD20H_RA_IG_CFG               0x30
#define L3GD20H_RA_IG_SRC               0x31
#define L3GD20H_RA_IG_THS_XH            0x32
#define L3GD20H_RA_IG_THS_XL            0x33
#define L3GD20H_RA_IG_THS_YH            0x34
#define L3GD20H_RA_IG_THS_YL            0x35
#define L3GD20H_RA_IG_THS_ZH            0x36
#define L3GD20H_RA_IG_THS_ZL            0x37
#define L3GD20H_RA_IG_DURATION          0x38
#define L3GD20H_RA_LOW_ODR              0x39

#define L3GD20H_WHO_AM_I_VALUE          0xD7

#define L3GD20H_CTRL1_FLAG_Y_EN         0x01
#define L3GD20H_CTRL1_FLAG_X_EN         0x02
#define L3GD20H_CTRL1_FLAG_Z_EN         0x04
#define L3GD20H_CTRL1_FLAG_POWER_EN     0x08

#define L3GD20H_CTRL5_FLAG_HPF_EN       0x10
#define L3GD20H_CTRL5_FLAG_FTH_EN       0x20
#define L3GD20H_CTRL5_FLAG_FIFO_EN      0x40
#define L3GD20H_CTRL5_FLAG_REBOOT       0x80

#define L3GD20H_STATUS_FLAG_XDA         0x01
#define L3GD20H_STATUS_FLAG_YDA         0x02
#define L3GD20H_STATUS_FLAG_ZDA         0x04
#define L3GD20H_STATUS_FLAG_ZYXDA       0x08
#define L3GD20H_STATUS_FLAG_XOR         0x10
#define L3GD20H_STATUS_FLAG_YOR         0x20
#define L3GD20H_STATUS_FLAG_ZOR         0x40
#define L3GD20H_STATUS_FLAG_ZYXOR       0x80

#define L3GD20H_FIFO_CTRL_MODE_BYPASS   0x00
#define L3GD20H_FIFO_CTRL_MODE_FIFO     0x01
#define L3GD20H_FIFO_CTRL_MODE_STREAM   0x02
#define L3GD20H_FIFO_CTRL_MODE_S2F      0x03
#define L3GD20H_FIFO_CTRL_MODE_B2S      0x04
#define L3GD20H_FIFO_CTRL_MODE_DS       0x06
#define L3GD20H_FIFO_CTRL_MODE_B2F      0x07

#define L3GD20H_FIFO_SRC_FLAG_FTH       0x80
#define L3GD20H_FIFO_SRC_FLAG_OVERRUN   0x40
#define L3GD20H_FIFO_SRC_FLAG_EMPTY     0x20

#define L3GD20H_LOW_ODR_FLAG_LOW_ODR    0x01
#define L3GD20H_LOW_ODR_FLAG_SW_RES     0x04
#define L3GD20H_LOW_ODR_FLAG_I2C_DIS    0x08
#define L3GD20H_LOW_ODR_FLAG_DRDY_HL    0x20

#define L3GD20H_AUTOINCREMENT           0x80

L3GD20H::L3GD20H():
    L3GD20H(L3GD20H_DEFAULT_ADDRESS, I2C::getDefault(), Poller::getDefault())
{
}

L3GD20H::L3GD20H(uint8_t address, I2C *bus, Poller *event_poller):
    _state(NotReady), _i2c(bus), _timer(new Timer(event_poller)),
    _address(address), _range(L3GD20H_RANGE_245)
{
    _timer->onTimeout = std::bind(&L3GD20H::_readData, this);
}

L3GD20H::~L3GD20H()
{
    delete _timer; _timer = nullptr;
}

int L3GD20H::initialize()
{
    if (_i2c->writeByte(_address, L3GD20H_RA_LOW_ODR, L3GD20H_LOW_ODR_FLAG_SW_RES) < 0) {
        Error() << "Unable to turn on device, device communication error";
        return -1;
    }

    uint8_t data;
    if (_i2c->readByte(_address, L3GD20H_RA_WHO_AM_I, data) < 0) {
        Error() << "Who am i check failed, device communication error";
        return -1;
    }
    if (data != L3GD20H_WHO_AM_I_VALUE) {
        Error() << "Unrecognized device";
        return -1;
    }

    if (_i2c->writeByte(_address, L3GD20H_RA_CTRL1, L3GD20H_CTRL1_FLAG_POWER_EN) < 0) {
        Error() << "Unable to turn on device, device communication error";
        return -1;
    }

    if (_i2c->writeByte(_address, L3GD20H_RA_CTRL5, L3GD20H_CTRL5_FLAG_FIFO_EN) < 0) {
        Error() << "Unable to enable FIFO, device communication error";
        return -1;
    }

    if (_i2c->writeByte(_address, L3GD20H_RA_FIFO_CTRL, L3GD20H_FIFO_CTRL_MODE_DS << 5) < 0) {
        Error() << "Unable to setup FIFO mode, device communication error";
        return -1;
    }
    _state = Ready;
    return 0;
}

int L3GD20H::setRange(uint8_t range)
{
    if (_state != Ready) {
        Error() << "Device is not ready";
        return -1;
    }
    if (range < 0 && range > 3) {
        Error() << "Invalid range" << range;
    }
    uint8_t data = range << 4;
    if (_i2c->writeByte(_address, L3GD20H_RA_CTRL4, data) < 0) {
        Error() << "Unable to set range, device communication problem";;
    }
    _range = range;
    return 0;
}

int L3GD20H::start(uint8_t rate, uint8_t bandwidth)
{
    if (_state != Ready) {
        Error() << "Device is not ready.";
        return -1;
    }
    if ((bandwidth & 0x3) != bandwidth) {
        Error() << "Wrong bandwith value";
        return -1;
    } else if ((rate & 0x3) != rate) {
        Error() << "Wrong rate value";
        return -1;
    }

    uint8_t data = rate << 6 | bandwidth << 4
            | L3GD20H_CTRL1_FLAG_POWER_EN
            | L3GD20H_CTRL1_FLAG_X_EN
            | L3GD20H_CTRL1_FLAG_Y_EN
            | L3GD20H_CTRL1_FLAG_Z_EN;

    if (_i2c->writeByte(_address, L3GD20H_RA_CTRL1, data) < 0) {
        Error() << "Unable to start sampling";
        return -1;
    }

    timespec interval;
    interval.tv_sec = 0;
    switch (rate) {
    case L3GD20H_RATE_NORMAL:
        interval.tv_nsec = 1000000000/100*24;
        break;
    case L3GD20H_RATE_DOUBLE:
        interval.tv_nsec = 1000000000/200*24;
        break;
    case L3GD20H_RATE_QUAD:
        interval.tv_nsec = 1000000000/400*24;
        break;
    case L3GD20H_RATE_OCTA:
        interval.tv_nsec = 1000000000/800*24;
        break;
    }
    _timer->start(interval);

    _state = Running;

    return 0;
}

int L3GD20H::stop()
{
    if (_state != Running) {
        Error() << "Device is not runnig.";
        return -1;
    }

    if (_i2c->writeByte(_address, L3GD20H_RA_CTRL1, L3GD20H_CTRL1_FLAG_POWER_EN) < 0) {
        Error() << "Unable to put device into sleep mode";
        return -1;
    }

    _timer->stop();
    _state = Ready;

    return 0;
}

void L3GD20H::_readData()
{
    uint8_t fifo;
    if (_i2c->readByte(_address, L3GD20H_RA_FIFO_SRC, fifo) < 0) {
        Error() << "Unable to get fifo control data, device communication error";
        return;
    }

    if (fifo & L3GD20H_FIFO_SRC_FLAG_EMPTY) {
        Debug() << "FIFO is empty";
        return;
    } else if (fifo & L3GD20H_FIFO_SRC_FLAG_OVERRUN) {
        Debug() << "FIFO overrun";
    }

    uint8_t size = fifo & 0x1F; // last 5 bits is size
    uint8_t data[size * 2 * 3];
    if (_i2c->readBytes(_address, L3GD20H_RA_OUT_X_L | L3GD20H_AUTOINCREMENT, size * 2 * 3, data) < 0) {
        Error() << "Unable to retrive data from fifo, device communication error";
        return;
    }

    for (uint8_t i=0; i<size; i++) {
        float x,y,z;

        x = (int16_t)(data[i*6+0] | (data[i*6+1] << 8));
        y = (int16_t)(data[i*6+2] | (data[i*6+3] << 8));
        z = (int16_t)(data[i*6+4] | (data[i*6+5] << 8));
        switch (_range) {
        case L3GD20H_RANGE_245:
            x *= (245.0f / 65535.0f);
            y *= (245.0f / 65535.0f);
            z *= (245.0f / 65535.0f);
            break;
        case L3GD20H_RANGE_500:
            x *= (500.0f / 65535.0f);
            y *= (500.0f / 65535.0f);
            z *= (500.0f / 65535.0f);
            break;
        case L3GD20H_RANGE_2000:
            x *= (2000.0f / 65535.0f);
            y *= (2000.0f / 65535.0f);
            z *= (2000.0f / 65535.0f);
            break;
        }

        if (onData) {
            onData(x, y, z);
        } else {
            Warn() << "No data callback was set";
        }
    }
}
