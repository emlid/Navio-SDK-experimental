#ifndef VZ89_H
#define VZ89_H

#define VZ89_I2C_ADDRESS 0x70

#include <stdint.h>

class Poller;
class I2C;

class VZ89
{
public:
    /** Constructor with default address, i2c bus and event loop. */
    VZ89();

    /** Constructor.
     * @param bus - but instance, where sensor located.
     * @param address - device address.
     */
    VZ89(uint8_t address, I2C *bus);
    VZ89(const VZ89& that) = delete; /**< Copy contructor is not allowed. */
    ~VZ89();

    /** Get sensor readings.
     * Couple of things to remember: there is always no data on first readings, reactivity can vary and values can be spoiled.
     * Also keep in mind that sensor internal cycle is 1 second.
     * @return 0 on success or negative value on error
     */
    int getStatus(float &co2, uint8_t &reactivity, float &tvoc);

private:
    I2C *_i2c;
    uint8_t _address;

};

#endif // VZ89_H
