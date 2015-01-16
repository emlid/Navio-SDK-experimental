#include "pca9685.h"
#include "i2c.h"
#include "log.h"

#include <cassert>
#include <unistd.h>
#include <math.h>

#define PCA9685_RA_MODE1            0x00
#define PCA9685_RA_MODE2            0x01
#define PCA9685_RA_SUBADR1          0x02
#define PCA9685_RA_SUBADR2          0x03
#define PCA9685_RA_SUBADR3          0x04
#define PCA9685_RA_ALLCALLADR       0x05
#define PCA9685_RA_LED_START        0x06
#define PCA9685_RA_ALL_LED_START    0xFA
#define PCA9685_RA_PRE_SCALE        0xFE
#define PCA9685_RA_TESTMODE         0xFF

#define PCA9685_MODE1_FLAG_RESTART  0x80
#define PCA9685_MODE1_FLAG_EXTCLK   0x40
#define PCA9685_MODE1_FLAG_AI       0x20
#define PCA9685_MODE1_FLAG_SLEEP    0x10
#define PCA9685_MODE1_FLAG_SUB1     0x08
#define PCA9685_MODE1_FLAG_SUB2     0x04
#define PCA9685_MODE1_FLAG_SUB3     0x02
#define PCA9685_MODE1_FLAG_ALLCALL  0x01

#define PCA9685_MODE2_INVRT_BIT     4
#define PCA9685_MODE2_OCH_BIT       3
#define PCA9685_MODE2_OUTDRV_BIT    2
#define PCA9685_MODE2_OUTNE1_BIT    1
#define PCA9685_MODE2_OUTNE0_BIT    0

PCA9685::PCA9685():
    PCA9685(PCA9685_I2C_DEFAULT_ADDR, I2C::getDefault())
{

}
PCA9685::PCA9685(uint8_t address, I2C *i2c):
    _i2c(i2c), _address(address), _frequency(0), _clock(25000000.f)
{
    assert(i2c != nullptr);
}

PCA9685::~PCA9685()
{
    setAllPWM(0);
}

int PCA9685::initialize(float ext_clock)
{
    uint8_t data;
    if (ext_clock > 0) {
        data = PCA9685_MODE1_FLAG_SLEEP;
        if (_i2c->writeByte(_address, PCA9685_RA_PRE_SCALE, &data) < 0) {
            Error() << "Can not put device into sleep mode, device communication error.";
            return -1;
        }
        data = PCA9685_MODE1_FLAG_SLEEP | PCA9685_MODE1_FLAG_EXTCLK;
        if (_i2c->writeByte(_address, PCA9685_RA_PRE_SCALE, &data) < 0) {
            Error() << "Can not set ext clock mode, device communication error.";
            return -1;
        }
        _clock = ext_clock;
    }

    if (_i2c->readByte(_address, PCA9685_RA_PRE_SCALE, &data) < 0) {
        Error() << "Read pre-scale register failed.";
        return -1;
    }
    _frequency = _clock / 4096.f / (data + 1);

    data = PCA9685_MODE1_FLAG_AI;
    if (_i2c->writeByte(_address, PCA9685_RA_MODE1, &data) < 0) {
        Error() << "Can not finish initialization sequence, device communication error.";
        return -1;
    }

    return 0;
}

int PCA9685::setFrequency(float frequency)
{
    return setPrescale(roundf(_clock / 4096.f / frequency) - 1);
}

int PCA9685::setFrequencyMin(float frequency)
{
    return setPrescale(floorf(_clock / 4096.f / frequency) - 1);
}

int PCA9685::setFrequencyMax(float frequency)
{
    return setPrescale(ceilf(_clock / 4096.f / frequency) - 1);
}

float PCA9685::getFreqeuncy()
{
    return _frequency;
}

int PCA9685::setPrescale(float prescale)
{
    if (prescale < 0.f || prescale > 4095.f) {
        Error() << "Invalid prescale:" << prescale;
        return -1;
    }

    uint8_t prescale_data = prescale;
    uint8_t oldmode;
    _i2c->readByte(_address, PCA9685_RA_MODE1, &oldmode);

    uint8_t newmode = (oldmode ^ PCA9685_MODE1_FLAG_RESTART) | PCA9685_MODE1_FLAG_SLEEP;
    _i2c->writeByte(_address, PCA9685_RA_MODE1, &newmode);
    _i2c->writeByte(_address, PCA9685_RA_PRE_SCALE, &prescale_data);
    _i2c->writeByte(_address, PCA9685_RA_MODE1, &oldmode);
    oldmode |= PCA9685_MODE1_FLAG_AI;
    _i2c->writeByte(_address, PCA9685_RA_MODE1, &oldmode);

    _frequency = _clock / 4096.f / (prescale + 1);

    return 0;
}

int PCA9685::setPWM(uint8_t channel, uint16_t offset, uint16_t length)
{
    uint8_t data[4] = {0, 0, 0, 0};
    if (length == 0) {
        data[3] = 0x10; // always off
    } else if(length > 4095) {
        data[1] = 0x10; // always on
    } else {
        data[0] = offset & 0xFF;
        data[1] = offset >> 8;
        data[2] = length & 0xFF;
        data[3] = length >> 8;
    }
    return _i2c->writeBytes(_address, PCA9685_RA_LED_START + 4 * channel, 4, data);
}

int PCA9685::setPWM(uint8_t channel, uint16_t length)
{
    return setPWM(channel, 0, length);
}

int PCA9685::setPWMmS(uint8_t channel, float length_mS)
{
    return setPWM(channel, roundf((length_mS * 4096.f) / (1000.f / _frequency) - 1));
}

int PCA9685::setPWMuS(uint8_t channel, float length_uS)
{
    return setPWM(channel, roundf((length_uS * 4096.f) / (1000000.f / _frequency) - 1));
}

int PCA9685::setAllPWM(uint16_t offset, uint16_t length)
{
    uint8_t data[4] = {0, 0, 0, 0};
    if (length == 0) {
        data[3] = 0x10; // always off
    } else if (length > 4095) {
        data[1] = 0x10; // always on
    } else {
        data[0] = offset & 0xFF;
        data[1] = offset >> 8;
        data[2] = length & 0xFF;
        data[3] = length >> 8;
    }
    return _i2c->writeBytes(_address, PCA9685_RA_ALL_LED_START, 4, data);
}

int PCA9685::setAllPWM(uint16_t length)
{
    return setAllPWM(0, length);
}

int PCA9685::setAllPWMmS(float length_mS)
{
    return setAllPWM(roundf((length_mS * 4096.f) / (1000.f / _frequency) - 1));
}

int PCA9685::setAllPWMuS(float length_uS)
{
    return setAllPWM(roundf((length_uS * 4096.f) / (1000000.f / _frequency) - 1));
}
