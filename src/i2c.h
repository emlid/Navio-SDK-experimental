#ifndef I2C_H
#define I2C_H

#include <linux/i2c-dev.h>
#include <stdint.h>

#define I2C_M_WR            0x00    /**< Write flag. */
#define I2C_M_RD            0x01    /**< Read flag. */
#define I2C_M_TEN           0x10    /**< 10-bit address space. */
#define I2C_M_NOSTART       0x4000  /**< TODO:  */
#define I2C_M_REV_DIR_ADDR	0x2000  /**< TODO:  */
#define I2C_M_IGNORE_NAK	0x1000  /**< TODO:  */
#define I2C_M_NO_RD_ACK		0x0800  /**< TODO:  */

/** Linux i2c message.
 * This sctructure used together with i2c_rdwr_ioctl_data to send and recive multiple messages in one syscall.
 * Read linux i2c documentation if you want to use it.
 */
struct i2c_msg {
    uint16_t addr;  /**< slave address. */
    uint16_t flags; /**< I2C_M_ flags. */
    int16_t len;    /**< data length. */
    uint8_t *buf;   /**< data pointer. */
};

/** Linux i2c bus driver.
 * Class provides i2c buss intercation primitives like read, write and multi-message read+write.
 */
class I2C
{
public:
    I2C();
    I2C(const I2C& that) = delete;  /**< Copy contructor not allowed because of file descriptor. */
    ~I2C();

    /** Open i2c block device.
     * @param dev_path - path to dev
     * @return 0 on success or negative value on error
     */
    int openDevice(const char *dev_path);

    /** Read byte from i2c device register.
     * @param device_address - i2c device address
     * @param register_address - i2c device register
     * @param data - data pointer
     * @return 0 on success or negative value on error
     */
    int readByte(const uint8_t device_address, const uint8_t register_address, uint8_t &data);

    /** Read bytes from i2c device register.
     * @param device_address - i2c device address
     * @param register_address - i2c device register
     * @param size - variable size
     * @param data - data pointer
     * @return 0 on success or negative value on error
     */
    int readBytes(const uint8_t device_address, const uint8_t register_address, const uint8_t size, uint8_t data[]);

    /** Touch i2c device register.
     * @param device_address - i2c device address
     * @param register_address - i2c device register
     * @return 0 on success or negative value on error
     */
    int write(const uint8_t device_address, const uint8_t register_address);

    /** Write bytes to the i2c device register.
     * @param device_address - i2c device address
     * @param register_address - i2c device register
     * @param data - data catched by reference
     * @return 0 on success or negative value on error
     */
    int writeByte(const uint8_t device_address, const uint8_t register_address, const uint8_t &data);

    /** Write bytes to the i2c device register.
     * @param device_address - i2c device address
     * @param register_address - i2c device register
     * @param size - variable size
     * @param data - data to write
     * @return 0 on success or negative value on error
     */
    int writeBytes(const uint8_t device_address, const uint8_t register_address, const uint8_t size, const uint8_t data[]);

    /** Write data to device.
     * @param device_address - i2c device address
     * @param size - data array size
     * @param data - data array
     * @return 0 on success or negative value on error
     */
    int writeBatch(const uint8_t device_address, const uint8_t size, const uint8_t data[]);

    /** Send i2c messages bundle.
     * @param messages - i2c_rdwr_ioctl_data message pack. Read linux i2c documentation if you want to use it.
     * @return 0 on success or negative value on error
     */
    int readWrite(i2c_rdwr_ioctl_data &messages);

    /** Get default instance.
     * @return I2C default instance.
     */
    static I2C* getDefault();

private:
    int _fd;
};

#endif
