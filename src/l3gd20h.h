#ifndef L3GD20H_H
#define L3GD20H_H

#define L3GD20H_DEFAULT_ADDRESS 0x6B

#define L3GD20H_RATE_NORMAL     0x00
#define L3GD20H_RATE_DOUBLE     0x01
#define L3GD20H_RATE_QUAD       0x02
#define L3GD20H_RATE_OCTA       0x03

// Read AN4506 Application note for detailed explanation.
#define L3GD20H_BANDWITH_A      0x00
#define L3GD20H_BANDWITH_B      0x01
#define L3GD20H_BANDWITH_C      0x02
#define L3GD20H_BANDWITH_D      0x03

#define L3GD20H_RANGE_245       0x00
#define L3GD20H_RANGE_500       0x01
#define L3GD20H_RANGE_2000      0x02


#include <stdint.h>
#include <functional>

class EventPoller;
class I2C;
class Timer;

class L3GD20H
{
    enum State {
        NotReady,
        Ready,
        Running
    };

public:
    std::function<void(float, float, float)> onData;

    L3GD20H();
    L3GD20H(uint8_t address, I2C *bus, EventPoller *event_poller);
    ~L3GD20H();

    int initialize();

    int setRange(uint8_t range=L3GD20H_RANGE_245);
    int start(uint8_t rate=L3GD20H_RATE_NORMAL, uint8_t bandwidth=L3GD20H_BANDWITH_A);
    int stop();

private:
    State _state;
    I2C *_i2c;
    Timer *_timer;
    uint8_t _address;
    uint8_t _range;

    void _readData();
};

#endif
