#include "ssd1306.h"
#include "i2c.h"

SSD1306::SSD1306():
    SSD1306(SSD1306_I2C_ADDRESS, I2C::getDefault())
{
}

SSD1306::SSD1306(uint8_t address, I2C *bus)
{

}

SSD1306::~SSD1306()
{

}
