#ifndef ADS1115_H
#define ADS1115_H

#define ADS1115_I2C_ADDRESS 0x48

#include <stdint.h>
#include <functional>

class Poller;
class Timer;
class I2C;

class ADS1115
{
public:
    enum Mux {
        MD01 = 0b000, /** Differential AINP = AIN0 and AINN = AIN1 (default) */
        MD03 = 0b001, /** Differential AINP = AIN0 and AINN = AIN3 */
        MD13 = 0b010, /** Differential AINP = AIN1 and AINN = AIN3 */
        MD23 = 0b011, /** Differential AINP = AIN2 and AINN = AIN3 */
        MS0G = 0b100, /** Single AINP = AIN0 and AINN = GND */
        MS1G = 0b101, /** Single AINP = AIN1 and AINN = GND */
        MS2G = 0b110, /** Single AINP = AIN2 and AINN = GND */
        MS3G = 0b111, /** Single AINP = AIN3 and AINN = GND */
    };

    enum Gain {
        G6144 = 0b000, /** scale is 6.144V. Analog input max voltage is VDD + 0.3V */
        G4096 = 0b001, /** scale is 4.096V. Analog input max voltage is VDD + 0.3V */
        G2048 = 0b010, /** scale is 2.048V. */
        G1024 = 0b011, /** scale is 1.024V. */
        G0512 = 0b100, /** scale is 0.512V. */
        G0256 = 0b101, /** scale is 0.256V. */
    };

    enum SampleRate {
        SR8  = 0b000,   /** 8 samples per second */
        SR16 = 0b001,   /** 16 samples per second */
        SR32 = 0b010,   /** 32 samples per second */
        SR64 = 0b011,   /** 64 samples per second */
        SR128 = 0b100,  /** 128 samples per second */
        SR250 = 0b101,  /** 250 samples per second */
        SR475 = 0b110,  /** 475 samples per second */
        SR860 = 0b111   /** 860 samples per second */
    };

    enum State {
        NotReady,
        Ready,
        SamplingSingleShot,
        SamplingContiniously
    };

    std::function<void(float)> onData;

    ADS1115();
    ADS1115(uint8_t address, I2C *bus, Poller *event_poller);
    ADS1115(const ADS1115& that) = delete; /**< Copy contructor is not allowed. */
    ~ADS1115();

    /** Initialize sensor and read calibration data.
     * @return 0 on success or negative value on error
     */
    int initialize();

    /** Start data sampling.
     * @param mux - input muxing.
     * @param gain - input gain.
     * @param sample_rate - device sample rate.
     * @param single_shot - sampling strategy. Set to false if you need continious sampling.
     * @return 0 on success or negative value on error
     */
    int startSampling(Mux mux=MD01, Gain gain=G2048, SampleRate sample_rate=SR128, bool single_shot=true);

    /** Stop data sampling.
     * @return 0 on success or negative value on error
     */
    int stopSampling();

private:
    I2C *_i2c;
    Timer *_timer;
    uint8_t _address;
    State _state;
    uint8_t _gain;

    void _getSample();
};

#endif // ADS1115_H
