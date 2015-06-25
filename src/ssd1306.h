#ifndef SSD1306_H
#define SSD1306_H

#define SSD1306_I2C_ADDRESS   0x3C

#include <stdint.h>
#include <functional>

class Poller;
class Timer;
class I2C;

class SSD1306
{
public:
    SSD1306();
    SSD1306(uint8_t address, I2C *bus);
    SSD1306(const SSD1306& that) = delete; /**< Copy contructor is not allowed. */
    ~SSD1306();

private:
    I2C *_i2c;

};

#endif // SSD1306_H
