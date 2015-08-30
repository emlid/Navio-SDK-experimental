#ifndef SSD1306_H
#define SSD1306_H

#define SSD1306_I2C_ADDRESS     0x3C

#define COLOR_BLACK             0x00
#define COLOR_WHITE             0x01
#define COLOR_INVERT            0x02

#define FONT_TERMINUS_v12n      0x00
#define FONT_TERMINUS_v14b      0x01
#define FONT_TERMINUS_v14n      0x02
#define FONT_TERMINUS_v14v      0x03
#define FONT_TERMINUS_v16b      0x04
#define FONT_TERMINUS_v16n      0x05
#define FONT_TERMINUS_v16v      0x06
#define FONT_TERMINUS_v18b      0x07
#define FONT_TERMINUS_v18n      0x08
#define FONT_TERMINUS_v20b      0x09
#define FONT_TERMINUS_v20n      0x0A
#define FONT_TERMINUS_v22b      0x0B
#define FONT_TERMINUS_v22n      0x0C
#define FONT_TERMINUS_v24b      0x0D
#define FONT_TERMINUS_v24n      0x0E
#define FONT_TERMINUS_v28b      0x0F
#define FONT_TERMINUS_v28n      0x10
#define FONT_TERMINUS_v32b      0x11
#define FONT_TERMINUS_v32n      0x12

#include <stdint.h>
#include <functional>

class Poller;
class Timer;
class I2C;

class SSD1306
{
public:
    SSD1306();
    SSD1306(uint8_t address, I2C *bus, bool ext_vcc);
    SSD1306(const SSD1306& that) = delete; /**< Copy contructor is not allowed. */
    ~SSD1306();

    /** Initialize sensor and read calibration data.
     * @return 0 on success or negative value on error
     */
    int initialize();

    int setInverted(bool inverted);

    size_t width();
    size_t height();

    int commit();

    void clear();
    void fill();

    void drawPixel(uint8_t x, uint8_t y, uint8_t color=COLOR_WHITE);
    void drawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color=COLOR_WHITE);
    void drawText(uint8_t x, uint8_t y, std::string text, uint8_t color=COLOR_WHITE, uint8_t font=FONT_TERMINUS_v12n);
    void drawText(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, std::string text,
                  uint8_t color=COLOR_WHITE, uint8_t font=FONT_TERMINUS_v12n);

private:
    I2C *_i2c;
    uint8_t _address;
    bool _ext_vcc;
    uint8_t *_buffer;

    int _sendCommand(uint8_t command);
};

#endif // SSD1306_H
