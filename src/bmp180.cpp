#include "bmp180.h"
#include "i2c.h"
#include "timer.h"
#include "log.h"

#include <unistd.h>
#include <cassert>

#define BMP180_ID_REG               0xD0
#define BMP180_ID_REF               0x55

#define BMP180_PROM_START_ADDR      0xAA
#define BMP180_PROM_DATA_LEN        22

#define BMP180_CTRL_MEAS_REG        0xF4
#define BMP180_ADC_OUT_REG          0xF6

#define BMP180_SOFT_RESET_REG       0xE0
#define BMP180_SOFT_RESET_REF       0xB6 // Power on reset sequence

#define BMP180_COMMAND_TEMPERATURE  0x2E // temperature measurent
#define BMP180_COMMAND_PRESSURE     0x34 // pressure measurement

#define BMP180_DELAY                4500
#define BMP180_RETRY                2000


BMP180::BMP180():
    BMP180(BMP180_I2C_DEFAULT_ADDR, I2C::getDefault(), Poller::getDefault())
{
}

BMP180::BMP180(uint8_t address, I2C *bus, Poller *event_poller):
    _state(NotReady), _i2c(bus), _timer(new Timer(event_poller)),
    _address(address),  _id(0), _oversampling(BMP180_OVERSAMPLING_SINGLE),
    _ac1(0), _ac2(0), _ac3(0), _ac4(0), _ac5(0), _ac6(0),
    _b1(0), _b2(0), _b5(0), _mb(0), _mc(0), _md(0),
    _temperature(0), _pressure(0)
{
    assert(_i2c != nullptr);
    _timer->callback = [&]() {
        assert(_state != NotReady);
        assert(_state != Ready);
        if (_state == ReadingTemperature) {
            if (_readTemperatureADC() < 0) {
                Error() << "_readTemperatureADC error";
                if (onError) onError();
                _state = Ready;
                return;
            }
            if (onTemperature) onTemperature(_temperature);
            _state = Ready;
        } else if (_state == ReadingComboTemperature) {
            if (_readTemperatureADC() < 0) {
                Error() << "_readTemperatureADC error";
                if (onError) onError();
                _state = Ready;
                return;
            }
            if (_writeCommand(BMP180_COMMAND_PRESSURE | (_oversampling << 6)) < 0) {
                Error() << "Unable to send pressure read command";
                if (onError) onError();
                _state = Ready;
                return;
            }
            _state = ReadingComboPressure;
            _timer->start(46, 0);
        } else if (_state == ReadingComboPressure) {
            if (_readPressureADC() < 0) {
                Error() << "_readPressureADC error";
                if (onError) onError();
            }
            if (onTemperatureAndPressure) onTemperatureAndPressure(_temperature, _pressure);
            _state = Ready;
        }
    };
}

BMP180::~BMP180()
{
    delete _timer; _timer = nullptr;
}

int BMP180::initialize()
{
    // chip id check
    if (_i2c->readByte(_address, BMP180_ID_REG, &_id) < 0) {
        Error() << "Unable to read device id";
        return -1;
    }
    if (_id != BMP180_ID_REF) {
        Error() << "Unexpected device id:" << _id
                << "Should be" << BMP180_ID_REF;
        return -1;
    }

    // read and validate chip calibration data
    uint8_t data[22];
    if (_i2c->readBytes(_address, BMP180_PROM_START_ADDR, BMP180_PROM_DATA_LEN, data) < 0) {
        Error() << "Unable to read calibration data";
        return -1;
    }

    for (int i=0; i < BMP180_PROM_DATA_LEN/2; i++) {
        uint16_t word = ((uint16_t*)data)[i];
        if (word == 0xFFFF || word == 0x0000) {
            Error() << "Damaged calibration data or communication error."
                    << "Word" << i << "is" << word;
            return -1;
        }
    }

    _ac1 = (data[0] << 8) | data[1];
    _ac2 = (data[2] << 8) | data[3];
    _ac3 = (data[4] << 8) | data[5];
    _ac4 = (data[6] << 8) | data[7];
    _ac5 = (data[8] << 8) | data[9];
    _ac6 = (data[10] << 8) | data[11];

    _b1 = (data[12] << 8) | data[13];
    _b2 = (data[14] << 8) | data[15];
    _mb = (data[16] << 8) | data[17];
    _mc = (data[18] << 8) | data[19];
    _md = (data[20] << 8) | data[21];

    _state = Ready;

    return 0;
}

int BMP180::setOversampling(uint8_t ovesampling)
{
    if (_state != Ready) {
        Error() << "Device is not ready";
        return -1;
    }

    if (ovesampling != (ovesampling & 0x03)) {
        Error() << "Wrong oversampling value";
        return -1;
    }
    _oversampling = ovesampling;
    return 0;
}

int BMP180::getTemperature()
{
    if (_state != Ready) {
        Error() << "Device is not ready";
        return -1;
    }

    if (_writeCommand(BMP180_COMMAND_TEMPERATURE) < 0) {
        Error() << "Unable to send temperature read command";
        return -1;
    }
    _state = ReadingTemperature;
    _timer->start(5, 0);
    return 0;
}

int BMP180::getTemperatureAndPressure()
{
    if (_state != Ready) {
        Error() << "Device is not ready";
        return -1;
    }

    if (_writeCommand(BMP180_COMMAND_TEMPERATURE) < 0) {
        Error() << "Unable to send temperature read command";
        return -1;
    }
    _state = ReadingComboTemperature;
    _timer->start(5, 0);
    return 0;
}

void BMP180::reset()
{
    _timer->stop();
    uint8_t data = BMP180_SOFT_RESET_REF;
    _i2c->writeByte(_address, BMP180_SOFT_RESET_REG, &data);
    _state = NotReady;
}

int BMP180::_readTemperatureADC()
{
    uint8_t data[2];
    if (_i2c->readBytes(_address, BMP180_ADC_OUT_REG, 2, data) < 0) {
        Error() << "Unable to obtain data from device";
        return -1;
    }
    uint32_t raw_temperature = ((data[0]<<8)|data[1]);

    int32_t x1, x2;
    x1 = (((int32_t)raw_temperature - (int32_t)_ac6) * (int32_t)_ac5) >> 15;
    x2 = ((int32_t)_mc << 11) / (x1 + _md);
    _b5 = x1 + x2;

    float result = (_b5 + 8) >> 4;
    _temperature = result / 10;

    return 0;
}

int BMP180::_readPressureADC()
{
    uint8_t data[3];
    if (_i2c->readBytes(_address, BMP180_ADC_OUT_REG, 3, data) < 0) {
        Error() << "Unable to obtain data from device";
        return -1;
    }
    uint32_t raw_pressure = (((uint32_t) data[0] << 16) | ((uint32_t) data[1] << 8) | (uint32_t) data[2]) >> (8 - _oversampling);

    int32_t x1, x2, x3, b3, b6, p;
    uint32_t b4, b7;

    b6 = _b5 - 4000;
    x1 = (_b2 * (b6 * b6) >> 12) >> 11;
    x2 = (_ac2 * b6) >> 11;
    x3 = x1 + x2;
    b3 = (((((int32_t)_ac1) * 4 + x3) << _oversampling) + 2)>>2;

    x1 = (_ac3 * b6)>>13;
    x2 = (_b1 * ((b6 * b6)>>12))>>16;
    x3 = ((x1 + x2) + 2)>>2;
    b4 = (_ac4 * (unsigned int)(x3 + 32768))>>15;

    b7 = ((uint32_t)(raw_pressure - b3) * (50000 >> _oversampling));
    if (b7 < 0x80000000) {
        p = (b7<<1)/b4;
    } else {
        p = (b7/b4)<<1;
    }

    x1 = (p>>8) * (p>>8);
    x1 = (x1 * 3038)>>16;
    x2 = (-7357 * p)>>16;
    p += (x1 + x2 + 3791)>>4;

    _pressure = (float)p / 100;

    return 0;
}

int BMP180::_writeCommand(uint8_t command)
{
    return _i2c->writeByte(_address, BMP180_CTRL_MEAS_REG, &command);
}
