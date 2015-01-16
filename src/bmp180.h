#ifndef BMP180_H
#define BMP180_H

#define BMP180_I2C_DEFAULT_ADDR     0x77
#define BMP180_OVERSAMPLING_SINGLE  0
#define BMP180_OVERSAMPLING_DOUBLE  1
#define BMP180_OVERSAMPLING_QUAD    2
#define BMP180_OVERSAMPLING_OCTA    3

#include <stdint.h>
#include <functional>

class EventPoller;
class Timer;
class I2C;

/** Bosch bmp180 pressure sensor.
 * Class provides methods and callbacks to read temperature and pressure from bmp180 sensor.
 */
class BMP180
{
    enum State {
        NotReady,
        Ready,
        ReadingTemperature,
        ReadingComboTemperature,
        ReadingComboPressure
    };

public:
    /** This callback will be called on error. */
    std::function<void(void)> onError;

    /** This callback will be called when temperature is ready.
     * Also keep in mind that this callback will not be called if you have requested pressure.
     * @param float temperature in Celsius.
     */
    std::function<void(float)> onTemperature;

    /** This callback will be called when temperature and pressure is ready.
     * @param float temperature in Celsius.
     * @param float pressure in hPa.
     */
    std::function<void(float, float)> onTemperatureAndPressure;

    /** Constructor with default address, i2c bus and event loop. */
    BMP180();

    /** Constructor.
     * @param event_poller - event poller instance, which will be used for event polling.
     * @param bus - but instance, where sensor located.
     * @param address - device address.
     */
    BMP180(uint8_t address, I2C *bus, EventPoller *event_poller);
    BMP180(const BMP180& that) = delete; /**< Copy contructor not allowed because of sensor state and timers. */
    ~BMP180();

    /** Initialize sensor and read calibration data.
     * @return 0 on success or negative value on error
     */
    int initialize();

    /** Set sensor oversampling.
     * Set how many samples should be used for approximation by sensor.
     * Use predefined constants: BMP180_OVERSAMPLING_*.
     * @return 0 on success or negative value on error
     */
    int setOversampling(uint8_t ovesampling);

    /** Start temperature ADC sequence.
     * Only temperature will be collected.
     * After operation completion onTemperature callback will be called.
     * @return 0 on success or negative value on error
     */
    int getTemperature();

    /** Start temperature and pressure ADC sequence.
     * After operation completion onTemperatureAndPressure callback will be called.
     * @return 0 on success or negative value on error
     */
    int getTemperatureAndPressure();

    /** Perform device soft reset. */
    void reset();

private:
    State _state;
    I2C *_i2c;
    Timer *_timer;
    uint8_t _address;
    uint8_t _id;
    uint8_t _oversampling;

    int16_t _ac1;
    int16_t _ac2;
    int16_t _ac3;
    uint16_t _ac4;
    uint16_t _ac5;
    uint16_t _ac6;

    int16_t _b1;
    int16_t _b2;
    int32_t _b5;

    int16_t _mb;
    int16_t _mc;
    int16_t _md;

    float _temperature;
    float _pressure;

    int _readTemperatureADC();
    int _readPressureADC();
    int _writeCommand(uint8_t command);
};

#endif
