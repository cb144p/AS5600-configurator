#ifndef I2CAS5600_H
#define I2CAS5600_H

#include <stdint.h>
#include <stm32f411xe.h>

#include "i2c.h"
#include "itimer.h"

constexpr uint8_t AS5600ID = 0x36;
constexpr uint8_t ANGLEID = 0x0E;
constexpr uint8_t RAWANGLEID = 0x0C;
constexpr uint8_t STATUSID = 0x0B;

// Only one per bus
class I2CAS5600 {
    
public:
    I2C_TypeDef* i2c;
    uint8_t status;

private:

    __always_inline void i2c_startup(uint8_t reg) {
        i2c_start(i2c);
        i2c_addr(i2c, AS5600ID);
        i2c_write(i2c, reg);
    }

    I2CAS5600() {}

public:

    I2CAS5600(I2C_TypeDef* _i2c) {
        i2c = _i2c;
        i2c_init(i2c, 400000);
    }

    static I2CAS5600 fromInitBus(I2C_TypeDef* _i2c) {
        I2CAS5600 encoder;
        encoder.i2c = _i2c;
        return encoder;
    }

    uint16_t readRawAngle() {
        i2c_startup(RAWANGLEID);
        uint8_t pos[2];
        
        i2c_read(i2c, AS5600ID, pos, 2);
        return pos[1] << 8 | pos[0];
    }

    uint16_t readAngle() {
        i2c_startup(ANGLEID);
        uint8_t pos[2];
        
        i2c_read(i2c, AS5600ID, pos, 2);
        return pos[1] << 8 | pos[0];
    }

    void readStatus() {
        i2c_startup(STATUSID);
        status = i2c_read(i2c, AS5600ID);
    }

    __always_inline void writeByte(uint8_t reg, uint8_t byte) {
        i2c_startup(reg);
        i2c_write(i2c, byte);
        i2c_stop(i2c);
    }


};

#endif // I2CAS5600_H