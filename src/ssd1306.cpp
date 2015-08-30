#include "ssd1306.h"
#include "i2c.h"
#include "log.h"
#include <string.h>
#include <assert.h>

#include "fonts/fonts.inc"

#define SSD1306_SETCONTRAST 0x81
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON 0xA5
#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF

#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETCOMPINS 0xDA

#define SSD1306_SETVCOMDETECT 0xDB

#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE 0xD9

#define SSD1306_SETMULTIPLEX 0xA8

#define SSD1306_SETLOWCOLUMN 0x00
#define SSD1306_SETHIGHCOLUMN 0x10

#define SSD1306_SETSTARTLINE 0x40

#define SSD1306_MEMORYMODE 0x20
#define SSD1306_COLUMNADDR 0x21
#define SSD1306_PAGEADDR   0x22

#define SSD1306_COMSCANINC 0xC0
#define SSD1306_COMSCANDEC 0xC8

#define SSD1306_SEGREMAP 0xA0

#define SSD1306_CHARGEPUMP 0x8D

#define SSD1306_EXTERNALVCC 0x1
#define SSD1306_SWITCHCAPVCC 0x2

#define SSD1306_TRANSACTION_SIZE 128

SSD1306::SSD1306():
    SSD1306(SSD1306_I2C_ADDRESS, I2C::getDefault(), false)
{
}

SSD1306::SSD1306(uint8_t address, I2C *bus, bool ext_vcc):
    _i2c(bus), _address(address), _ext_vcc(ext_vcc), _buffer(nullptr)
{
    _buffer = new uint8_t [width()*height()/8];
}

SSD1306::~SSD1306()
{
    delete [] _buffer; _buffer = nullptr;
}

int SSD1306::initialize()
{
    // TODO: remove magic
    _sendCommand(SSD1306_DISPLAYOFF);

    _sendCommand(SSD1306_SETDISPLAYCLOCKDIV);
    _sendCommand(0xF0);

    _sendCommand(SSD1306_SETMULTIPLEX);
    _sendCommand(0x3F);

    _sendCommand(SSD1306_SETDISPLAYOFFSET);
    _sendCommand(0x00);

    _sendCommand(SSD1306_SETSTARTLINE | 0x0);

    _sendCommand(SSD1306_CHARGEPUMP);
    _sendCommand(_ext_vcc ? 0x10 : 0x14);

    _sendCommand(SSD1306_MEMORYMODE);
    _sendCommand(0x00);

    _sendCommand(SSD1306_SEGREMAP | 0x1);
    _sendCommand(SSD1306_COMSCANDEC);

    _sendCommand(SSD1306_SETCOMPINS);
    _sendCommand(0x12);

    _sendCommand(SSD1306_SETCONTRAST);
    _sendCommand(_ext_vcc ? 0x9F : 0xCF);

    _sendCommand(SSD1306_SETPRECHARGE);
    _sendCommand(_ext_vcc ? 0x22 : 0xF1);

    _sendCommand(SSD1306_SETVCOMDETECT);
    _sendCommand(0x40);

    _sendCommand(SSD1306_DISPLAYALLON_RESUME);
    _sendCommand(SSD1306_NORMALDISPLAY);
    _sendCommand(SSD1306_DISPLAYON);

    return 0;
}

int SSD1306::setInverted(bool inverted)
{
    return _sendCommand(inverted ? SSD1306_INVERTDISPLAY : SSD1306_NORMALDISPLAY);
}

size_t SSD1306::width()
{
    return 128;
}

size_t SSD1306::height()
{
    return 64;
}

int SSD1306::commit()
{
    _sendCommand(SSD1306_COLUMNADDR);
    _sendCommand(0);    // Column start address (0 = reset)
    _sendCommand(127);  // Column end address (127 = reset)

    _sendCommand(SSD1306_PAGEADDR);
    _sendCommand(0);    // Page start address (0 = reset)
    _sendCommand(7);    // Page end address

    uint8_t data[SSD1306_TRANSACTION_SIZE+1];
    for (size_t i=0; i < width()*height()/8/SSD1306_TRANSACTION_SIZE; i++) {
        data[0] = 0x40;
        memcpy(data+1, _buffer+i*SSD1306_TRANSACTION_SIZE, SSD1306_TRANSACTION_SIZE);
        _i2c->writeBatch(_address, sizeof(data), data);
    }

    return 0;
}

void SSD1306::fill()
{
    memset(_buffer, 0xFF, width()*height()/8);
}

void SSD1306::clear()
{
    memset(_buffer, 0x00, width()*height()/8);
}

void SSD1306::drawPixel(uint8_t x, uint8_t y, uint8_t color)
{
    if (x > 127 || y > 63) {
        Warn() << "Out of canvas drawing:" << x << y;
        return;
    }

    uint8_t page = y / 8;
    uint8_t page_offset = y % 8;
    uint8_t segment = x;

    if (color == COLOR_WHITE) {
        _buffer[page*width()+segment] = _buffer[page*width()+segment] | (1 << page_offset);
    } else if (color == COLOR_BLACK) {
        _buffer[page*width()+segment] = _buffer[page*width()+segment] & ~(1 << page_offset);
    } else if (color == COLOR_INVERT) {
        _buffer[page*width()+segment] = _buffer[page*width()+segment] ^ (1 << page_offset);
    }
}

void SSD1306::drawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color)
{
    if (x2<x1) std::swap(x1, x2);
    if (y2<y1) std::swap(y1, y2);

    if (x2 == x1) {
        uint8_t y=y1;
        do {
            drawPixel(x1, y, color);
            y++;
        } while(y<y2);
    } else if (y2==y1) {
        uint8_t x=x1;
        do {
            drawPixel(x, y1, color);
            x++;
        } while(x<x2);
    } else {
        uint8_t x=0;
        do {
            uint16_t y = y1 + x * (y2-y1) / (x2-x1);
            drawPixel(x1+x, y, color);
            x++;
        } while(x<(x2-x1));
    }
}

void SSD1306::drawText(uint8_t x, uint8_t y, std::string text, uint8_t color, uint8_t font)
{
    drawText(x, y, 127, 63, text, color, font);
}

void SSD1306::drawText(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, std::string text, uint8_t color, uint8_t font)
{
    if (x2<x1) std::swap(x1, x2);
    if (y2<y1) std::swap(y1, y2);

    const uint8_t* psf_font = getFont(font);
    const uint32_t* header = reinterpret_cast<const uint32_t*>(psf_font);

    uint32_t magic          = header[0];
    uint32_t version        = header[1];
    uint32_t header_size    = header[2];
    uint32_t flags          = header[3];
    uint32_t chars_count    = header[4];
    uint32_t char_length    = header[5];
    uint32_t char_height    = header[6];
    uint32_t char_width     = header[7];

    assert(magic == 0x864ab572);
    assert(version == 0);
    assert(flags == 0x1);
    assert(chars_count > 255);

    psf_font += header_size;

    uint32_t width = x2 - x1;
    uint32_t columns = width / char_width;

    int pos=0;
    for (std::string::iterator it=text.begin(); it!=text.end(); ++it) {
        switch (*it) {
        case '\n':
            pos += columns - pos % columns;
            continue;
        case '\t':
            pos += 2;
            break;
        }

        uint16_t text_x = x1 + (pos % columns) * char_width;
        uint16_t text_y = y1 + (pos / columns) * char_height;
        uint16_t real_width = char_width + (8 - char_width % 8) % 8; // 8bit aligning
        const uint8_t * font_char = psf_font + char_length * (*it);

        if (text_y < y2) {
            for (uint8_t ih=0; ih<char_height; ih++) {
                for (uint8_t iw=0; iw<real_width; iw++) {
                    size_t offset = ih * real_width / 8 + iw / 8;
                    uint8_t probe_mask = 0b10000000 >> (iw % 8);
                    if (*(font_char + offset) & probe_mask) {
                        drawPixel(text_x + iw, text_y + ih, color);
                    }
                }
            }
        }

        pos++;
    }
}

int SSD1306::_sendCommand(uint8_t command)
{
    uint8_t batch[] = { 0x00, command };
    return _i2c->writeBatch(_address, sizeof(batch), batch);
}
