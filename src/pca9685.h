#ifndef PCA9685_H
#define PCA9685_H

#include <stdint.h>

#define PCA9685_I2C_DEFAULT_ADDR    0x40

class I2C;

/** NXP pca9685 pwm driver.
 * Class provides methods to control pwm output.
 * However only limited set of feature was implemented.
 * TODO: external clock, sub addresses, mode2 register.
 */
class PCA9685
{
public:
    /** Constructor.
     * @param i2c - pointer to i2c driver instance.
     * @param address - pca9685 i2c address.
     */
    PCA9685(I2C *i2c, uint8_t address=PCA9685_I2C_DEFAULT_ADDR);
    PCA9685(const PCA9685& that) = delete; /**< Copy contructor not allowed because of sensor state. */
    ~PCA9685();

    /** Read current pca9685 pwm frequency, perform startup sequence.
     * @param ext_clock - clock frequency if oscillator installed, 0 if you want to use internal oscillator.
     * @return 0 on success, negative value on error.
     */
    int initialize(float ext_clock=0.0f);

    /** Set pwm frequency.
     * Doesn't guarantee that it will be exactly the same.
     * But as close to requested value as possible.
     * @param frequency - frequency in Hz.
     * @return 0 on success, negative value on error.
     */
    int setFrequency(float frequency);

    /** Set pwm frequency.
     * Doesn't guarantee that it will be exactly the same.
     * But higher then requested value.
     * @param frequency - frequency in Hz.
     * @return 0 on success, negative value on error.
     */
    int setFrequencyMin(float frequency);

    /** Set pwm frequency.
     * Doesn't guarantee that it will be exactly the same.
     * But lower then requested value.
     * @param frequency - frequency in Hz.
     * @return 0 on success, negative value on error.
     */
    int setFrequencyMax(float frequency);

    /** Get pwm frequency.
     * @return frequency in Hz. The real one, could be different from what you have requested.
     */
    float getFreqeuncy();

    /** Set clock prescale.
     * @param prescale value in range 0 to 4095.
     * @return 0 on success, negative value on error.
     */
    int setPrescale(float prescale);

    /** Set channel pwm value.
     * @param channel - channel number.
     * @param offset - pulse start offset.
     * @param length - pulse length.
     * @return 0 on success, negative value on error.
     */
    int setPWM(uint8_t channel, uint16_t offset, uint16_t length);

    /** Set channel pwm value with zero offset.
     * @param channel - channel number.
     * @param length - pulse length.
     * @return 0 on success, negative value on error.
     */
    int setPWM(uint8_t channel, uint16_t length);

    /** Set channel pwm pulse length in ms.
     * @param channel - channel number.
     * @param length_mS - pulse length in ms.
     * @return 0 on success, negative value on error.
     */
    int setPWMmS(uint8_t channel, float length_mS);

    /** Set channel pwm pulse length in us.
     * @param channel - channel number.
     * @param length_uS - pulse length in us.
     * @return 0 on success, negative value on error.
     */
    int setPWMuS(uint8_t channel, float length_uS);

    /** Set all channel pwm value.
     * @param offset - pulse start offset.
     * @param length - pulse length.
     * @return 0 on success, negative value on error.
     */
    int setAllPWM(uint16_t offset, uint16_t length);

    /** Set all channel pwm value with zero offset.
     * @param length - pulse length.
     * @return 0 on success, negative value on error.
     */
    int setAllPWM(uint16_t length);

    /** Set all channel pwm pulse length in ms.
     * @param length_mS - pulse length in ms.
     * @return 0 on success, negative value on error.
     */
    int setAllPWMmS(float length_mS);

    /** Set all channel pwm pulse length in us.
     * @param length_uS - pulse length in us.
     * @return 0 on success, negative value on error.
     */
    int setAllPWMuS(float length_uS);

 private:
    I2C *_i2c;          /**< i2c bus driver. */
    uint8_t _address;   /**< PCA9685 i2c address. */
    float _frequency;   /**< pwm frequency. */
    float _clock;       /**< oscillator frequency. */
};

#endif
