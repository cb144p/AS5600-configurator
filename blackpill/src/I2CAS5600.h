#ifndef I2CAS5600_H
#define I2CAS5600_H

#pragma GCC optimize ("O3")

#include <stdint.h>
#include <stm32f411xe.h>
#include <array>
#include <algorithm>
#include <string_view>
#include <bitset>

#include "i2c.h"
#include "itimer.h"

#define SAFE_FMAP
template<typename Key, typename Value, uint8_t size>
struct fMap {
	std::array<std::pair<Key, Value>, size> iMap;
	__attribute__((optimize ("03"))) constexpr Value at(const Key &key) const {
    const auto itr =
        std::find_if(std::begin(iMap), std::end(iMap),
                     [&key](const auto &v) { return v.first == key; });
    	#ifdef SAFE_FMAP
			if (itr != std::end(data)) {
    			return itr->second;
    		} else {
    			return 255;
    		}
		#else
			return itr->second;
		#endif // SAFE_FMAP
  	}
};

constexpr uint8_t AS5600ID = 0x36;

namespace regs {
    enum {
        ZMCO = 0x00,
        ZPOS = 0x01,
        MPOS = 0x03,
        MANG = 0x05,
        CONF = 0x07,
        RAWANGLE = 0x0C,
        ANGLE = 0x0E,
        STATUS = 0x0B,
        AGC = 0x1A,
        MAGNITUDE = 0x1B,
        BURN = 0xFF
    };

    constexpr uint8_t length = 11;
    // Lazy, but it works
    std::array<std::pair<const std::string_view, const uint8_t>, regs::length> nameArray{{
        {"ZMCO", 0},
        {"ZPOS", 1},
        {"MPOS", 2},
        {"MANG", 3},
        {"CONF", 4},
        {"RAWANGLE", 5},
        {"ANGLE", 6},
        {"STATUS", 7},
        {"AGC", 8},
        {"MAGNITUDE", 9},
        {"BURN", 10}
    }};

    fMap<const std::string_view, const uint8_t, length> nameToIndex{{nameArray}};

    constexpr std::bitset<length> sizes{0b01001111110};

    constexpr uint8_t addrs[length]{
        ZMCO,
        ZPOS,
        MPOS,
        MANG,
        CONF,
        RAWANGLE,
        ANGLE,
        STATUS,
        AGC,
        MAGNITUDE,
        BURN
    };

}

namespace conf {
    enum {
        PM = 0x0,
        HYST = 0x2,
        OUTS = 0x4,
        PWMF = 0x6,
        SF = 0x8,
        FTH = 0xA,
        WD = 0xD
    };
    constexpr uint8_t length = 7;

    std::array<std::pair<const std::string_view, const uint8_t>, length> nameArray = {{
        {"PM", 0},
        {"HYST", 1},
        {"OUTS", 2},
        {"PWMF", 3},
        {"SF", 4},
        {"FTH", 5},
        {"WD", 6}
    }};

    fMap<const std::string_view, const uint8_t, length> nameToIndex{{nameArray}};
}

namespace status {
    enum {
        MH = 0x3,
        ML = 0x4,
        MD = 0x5
    };
    constexpr uint8_t length = 3;

    std::array<std::pair<const std::string_view, const uint8_t>, length> nameArray = {{
        {"MD", 0},
        {"ML", 1},
        {"MD", 2}
    }};

    fMap<const std::string_view, const uint8_t, length> nameToIndex{{nameArray}}; 
}

struct AS5600Configs {
    uint16_t ZPOS;
    uint16_t MPOS;
    uint16_t MANG;
    uint16_t CONF;
};

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
        i2c_startup(regs::RAWANGLE);
        uint8_t pos[2];
        
        i2c_read(i2c, AS5600ID, pos, 2);
        return pos[1] << 8 | pos[0];
    }

    uint16_t readAngle() {
        i2c_startup(regs::ANGLE);
        uint8_t pos[2];
        
        i2c_read(i2c, AS5600ID, pos, 2);
        return pos[1] << 8 | pos[0];
    }

    void readStatus() {
        i2c_startup(regs::STATUS);
        status = i2c_read(i2c, AS5600ID);
    }

    __always_inline void writeByte(uint8_t reg, uint8_t byte) {
        i2c_startup(reg);
        i2c_write(i2c, byte);
        i2c_stop(i2c);
    }


};

#endif // I2CAS5600_H