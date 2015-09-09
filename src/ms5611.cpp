#include "ms5611.h"
#include "i2c.h"
#include "timer.h"
#include "log.h"

#include <unistd.h>
#include <cassert>
#include <cmath>

#define MS5611_REG_ADC          0x00
#define MS5611_REG_PROM         0xA0
#define MS5611_REG_TEMPERATURE  0x40
#define MS5611_REG_PRESSURE     0x50
#define MS5611_REG_RESET        0x1E

static const float _delays_ms[] = { 1, 2, 3, 5, 10 };

MS5611::MS5611():
    MS5611(MS5611_I2C_ADDRESS, I2C::getDefault(), Poller::getDefault())
{
}

MS5611::~MS5611()
{
    delete _timer; _timer = nullptr;
}

MS5611::MS5611(uint8_t address, I2C *bus, Poller *event_poller):
    _state(NotReady), _i2c(bus), _timer(new Timer(event_poller)),
    _address(address), _oversampling(MS5611_OVERSAMPLING_1024),
    _temperature(0), _pressure(0)
{
    assert(_i2c != nullptr);
    _timer->onTimeout = [this]() {
        assert(_state != NotReady);
        assert(_state != Ready);
        if (_state == ReadingTemperature) {
            if (_readTemperatureADC() < 0) {
                Error() << "_readTemperatureADC error";
                if (onError) onError();
                _state = Ready;
                return;
            }
            _calculate();
            if (onTemperature) onTemperature(_temperature);
            _state = Ready;
        } else if (_state == ReadingComboTemperature) {
            if (_readTemperatureADC() < 0) {
                Error() << "_readTemperatureADC error";
                if (onError) onError();
                _state = Ready;
                return;
            }
            if (_i2c->write(_address, MS5611_REG_PRESSURE | (_oversampling << 1)) < 0) {
                Error() << "Unable to send pressure read command";
                if (onError) onError();
                _state = Ready;
                return;
            }
            _state = ReadingComboPressure;
            _timer->singleShot(_delays_ms[_oversampling]);
        } else if (_state == ReadingComboPressure) {
            if (_readPressureADC() < 0) {
                Error() << "_readPressureADC error";
                if (onError) onError();
            }
            _calculate();
            if (onTemperatureAndPressure) onTemperatureAndPressure(_temperature, _pressure);
            _state = Ready;
        }
    };
}

int MS5611::initialize()
{
    if (_state != NotReady) {
        Error() << "Unable to initialize device that in use";
        return -1;
    }
    // ensure that PROM data properly propogated by the chip.
    if (reset() < 0) {
        Error() << "Unable to reset device";
        return -1;
    }
    // read PROM data from device
    uint8_t buff[16];
    for (size_t i=0; i<16; i+=2) {
        if (_i2c->readBytes(_address, MS5611_REG_PROM + i, 2, buff + i) < 0) {
            Error() << "Unable to read calibration data";
            return -1;
        }
    }
    // first 2 bytes reserved for manufacturer
    _c1 = buff[2]<<8 | buff[3];
    _c2 = buff[4]<<8 | buff[5];
    _c3 = buff[6]<<8 | buff[7];
    _c4 = buff[8]<<8 | buff[9];
    _c5 = buff[10]<<8 | buff[11];
    _c6 = buff[12]<<8 | buff[13];
    // last 3 bits of last 2 bytes is CRC4
    // TODO: implement crc check

    _state = Ready;

    return 0;
}

int MS5611::setOversampling(uint8_t oversampling)
{
    if (oversampling > MS5611_OVERSAMPLING_4096) {
        Error() << "Invalid sampling rate" << oversampling;
        return -1;
    }

    _oversampling = oversampling;
    return 0;
}

int MS5611::getTemperature()
{
    if (_state != Ready) {
        Error() << "Device is not ready";
        return -1;
    }

    if (_i2c->write(_address, MS5611_REG_TEMPERATURE | (_oversampling << 1)) < 0) {
        Error() << "Can not start conversion";
        return -1;
    }

    _timer->singleShot(_delays_ms[_oversampling]);
    _state = ReadingTemperature;

    return 0;
}

int MS5611::getTemperatureAndPressure()
{
    if (_state != Ready) {
        Error() << "Device is not ready";
        return -1;
    }

    if (_i2c->write(_address, MS5611_REG_TEMPERATURE | (_oversampling << 1)) < 0) {
        Error() << "Can not start conversion";
        return -1;
    }

    _timer->singleShot(_delays_ms[_oversampling]);
    _state = ReadingComboTemperature;

    return 0;
}

int MS5611::reset() {
    int ret = _i2c->write(_address, MS5611_REG_RESET);
    if (ret < 0) return ret;
    usleep(2800);
    return 0;
}

int MS5611::_readTemperatureADC()
{
    uint8_t buffer[3];
    if (_i2c->readBytes(_address, MS5611_REG_ADC, 3, buffer) < 0) {
        Error() << "Unable to read ADC data";
        return -1;
    }
    _raw_temperature = (buffer[0] << 16) | (buffer[1] << 8) | buffer[2];
    return 0;
}

int MS5611::_readPressureADC()
{
    uint8_t buffer[3];
    if (_i2c->readBytes(_address, MS5611_REG_ADC, 3, buffer) < 0) {
        Error() << "Unable to read ADC data";
        return -1;
    }
    _raw_pressure = (buffer[0] << 16) | (buffer[1] << 8) | buffer[2];
    return 0;
}

void MS5611::_calculate()
{
    float dT = _raw_temperature - _c5 * powf(2, 8);
    _temperature = (2000 + ((dT * _c6) / powf(2, 23)));
    float OFF = _c2 * powf(2, 16) + (_c4 * dT) / powf(2, 7);
    float SENS = _c1 * powf(2, 15) + (_c3 * dT) / powf(2, 8);

    float T2, OFF2, SENS2;

    if (_temperature >= 2000) {
        T2 = 0;
        OFF2 = 0;
        SENS2 = 0;
    } else {
        T2 = dT * dT / powf(2, 31);
        OFF2 = 5 * powf(_temperature - 2000, 2) / 2;
        SENS2 = OFF2 / 2;

        if (_temperature < -1500) {
            OFF2 = OFF2 + 7 * powf(_temperature + 1500, 2);
            SENS2 = SENS2 + 11 * powf(_temperature + 1500, 2) / 2;
        }
    }

    _temperature = _temperature - T2;
    OFF = OFF - OFF2;
    SENS = SENS - SENS2;

    _pressure = ((_raw_pressure * SENS) / powf(2, 21) - OFF) / powf(2, 15) / 100;
    _temperature = _temperature / 100;
}
