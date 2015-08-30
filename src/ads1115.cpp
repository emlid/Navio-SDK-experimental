#include "ads1115.h"
#include "i2c.h"
#include "poller.h"
#include "timer.h"
#include "log.h"

#define ADS1115_REGISTER_CONVERSION 0x00
#define ADS1115_REGISTER_CONFIG     0x01
#define ADS1115_REGISTER_LO_THRESH  0x02
#define ADS1115_REGISTER_HI_THRESH  0x03

static const uint64_t _rates[] = { 8, 16, 32, 64, 128, 250, 475, 860 };
static const float _gains[] = { 6.144, 4.096, 2.048, 1.024, 0.512, 0.256 };

ADS1115::ADS1115():
    ADS1115(ADS1115_I2C_ADDRESS, I2C::getDefault(), Poller::getDefault())
{
}

ADS1115::ADS1115(uint8_t address, I2C *bus, Poller *event_poller):
    _i2c(bus), _timer(new Timer(event_poller)), _address(address), _state(NotReady), _gain(0)
{
    _timer->onTimeout = [this]() {
        switch (_state) {
        case SamplingSingleShot:
            _getSample();
            _timer->stop();
            _state = Ready;
            break;
        case SamplingContiniously:
            _getSample();
            break;
        default:
            Error() << "State programming error" << _state << "is not Sampling.";
            break;
        }
    };
}

ADS1115::~ADS1115()
{
    delete _timer; _timer = nullptr;
}

int ADS1115::initialize()
{
    if (_state != NotReady) {
        Error() << "Device already initialized";
        return -1;
    }

    uint8_t data[2];
    data[0] = 0b00000001; // Power down device.
    data[1] = 0b00000011; // Comparator magic: disabled.

    if (_i2c->writeBytes(_address, ADS1115_REGISTER_CONFIG, 2, data) < 0) {
        Error() << "Failed to write config to device";
        return -1;
    }

    _state = Ready;
    return 0;
}

int ADS1115::startSampling(Mux mux, Gain gain, SampleRate sample_rate, bool single_shot)
{
    if (_state != Ready) {
        Error() << "Not ready";
        return -1;
    }

    uint8_t config[2];
    config[0] = (single_shot << 7) | (mux << 4) | (gain << 1) | (single_shot);
    config[1] = (sample_rate << 5) | 0b00011; // Comparator magic: disabled.

    if (_i2c->writeBytes(_address, ADS1115_REGISTER_CONFIG, 2, config) < 0) {
        Error() << "Failed to write config to device";
        return -1;
    }

    _gain = gain;

    timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 1111111111 / _rates[sample_rate];  // according to datasheet max data rate variation is 10%
                                                    // so let's make timer interval a little bit bigger.
    _timer->start(ts);

    if (single_shot) {
        _state = SamplingSingleShot;
    } else {
        _state = SamplingContiniously;
    }

    return 0;
}

int ADS1115::stopSampling() {
    if (_state != SamplingContiniously) {
        Error() << "State programming error" << _state << "is not SamplingContiniously";
        return -1;
    }
    _timer->stop();
    uint8_t data[2];
    data[0] = 0b00000001; // Power down device.
    data[1] = 0b00000011; // Comparator magic: disabled.

    if (_i2c->writeBytes(_address, ADS1115_REGISTER_CONFIG, 2, data) < 0) {
        Error() << "Failed to write config to device";
        return -1;
    }

    _state = Ready;
    return 0;
}

void ADS1115::_getSample()
{
    uint8_t data[2];
    if (_state == SamplingSingleShot) {
        if (_i2c->readBytes(_address, ADS1115_REGISTER_CONFIG, 2, data) < 0) {
            Error() << "Unable to read status register";
            return;
        }

        if (!(data[0] & 0b10000000)) {
            Error() << "device is in sampling state, looks like data is not ready";
            return;
        }
    }

    if (_i2c->readBytes(_address, ADS1115_REGISTER_CONVERSION, 2, data) < 0) {
        Error() << "Unable to read conversion register";
        return;
    }

    int16_t value = data[0] << 8 | data[1];
    float valuef = (float)value * _gains[_gain] / 32768.0;
    onData(valuef);
}
