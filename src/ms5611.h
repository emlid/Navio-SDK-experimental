#ifndef MS5611_H
#define MS5611_H

#define MS5611_I2C_ADDRESS          0x77

#define MS5611_OVERSAMPLING_256     0x00
#define MS5611_OVERSAMPLING_512     0x01
#define MS5611_OVERSAMPLING_1024    0x02
#define MS5611_OVERSAMPLING_2048    0x03
#define MS5611_OVERSAMPLING_4096    0x04


#include <stdint.h>
#include <functional>

class Poller;
class Timer;
class I2C;

/** MEAS MS5611 pressure sensor.
 * Class provides methods and callbacks to read temperature and pressure from MS5611 sensor.
 */
class MS5611
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
    MS5611();

    /** Constructor.
     * @param event_poller - event poller instance, which will be used for event polling.
     * @param bus - but instance, where sensor located.
     * @param address - device address.
     */
    MS5611(uint8_t address, I2C *bus, Poller *event_poller);
    MS5611(const MS5611& that) = delete; /**< Copy contructor not allowed because of sensor state and timers. */
    ~MS5611();

    /** Initialize sensor and read calibration data.
     * @return 0 on success or negative value on error
     */
    int initialize();

    /** Set sensor oversampling.
     * Set how many samples should be used for approximation by sensor.
     * Use predefined constants: BMP180_OVERSAMPLING_*.
     * @return 0 on success or negative value on error
     */
    int setOversampling(uint8_t oversampling);

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

    /** Perform device soft reset.
     * @return 0 on success or negative value on error
     */
    int reset();

private:
    State _state;
    I2C *_i2c;
    Timer *_timer;
    uint8_t _address;
    uint8_t _oversampling;

    uint16_t _c1; /** SENST1 - Pressure sensitivity */
    uint16_t _c2; /** OFFT1 - Pressure offset */
    uint16_t _c3; /** TCS - Temperature coefficient of pressure sensitivity */
    uint16_t _c4; /** TCO - Temperature coefficient of pressure offset */
    uint16_t _c5; /** TREF - Reference temperature */
    uint16_t _c6; /** TEMPSENS - Temperature coefficient of the temperature */

    uint32_t _raw_temperature;
    uint32_t _raw_pressure;
    float _temperature;
    float _pressure;

    int _readTemperatureADC();
    int _readPressureADC();
    void _calculate();
};

#endif // MS5611_H
