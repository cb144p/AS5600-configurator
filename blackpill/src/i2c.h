#ifndef I2C_H
#define I2C_H

#include <stdint.h>
#include <algorithm>
#include <stm32f411xe.h>

#include "rcc.h"
#include "gpio.h"

// TODO: Add proper error handling.
// Must init the pins before the i2c
// Limited to 400khz, for now...
static void i2c_init(I2C_TypeDef* i2c, uint32_t baud) {
    uint8_t bs = (uint32_t)(i2c - I2C1_BASE) / 0x400 + RCC_APB1ENR_I2C1EN_Pos;
    RCC->APB1RSTR |= 0b1 << bs;
    RCC->APB1RSTR &= ~(0b1 << bs);
    RCC->APB1ENR |= 0b1 << bs;
    i2c->CR2 |= 0b110000;
    // TODO: Figure out how to enable fast duty
    if (baud <= 100000) {
        i2c->CCR |= APB1_FREQUENCY / (2 * baud);
        i2c->TRISE = APB1_FREQUENCY / 100000; // 1000 ns max rise time per i2c sm standards.
    }
    else {
        i2c->CCR |= I2C_CCR_FS_Msk | I2C_CCR_DUTY_Msk | APB1_FREQUENCY * 2 / (5 * baud);
        i2c->TRISE |= APB1_FREQUENCY * 3 / 10000000; // 300 ns max rise per i2c fm standards.
    }
    
    i2c->CR1 |= I2C_CR1_PE | I2C_CR1_ACK;

}



inline void i2c_start(I2C_TypeDef* i2c) {
    i2c->CR1 |= I2C_CR1_START;
    while (!(i2c->SR1 & I2C_SR1_SB));
}

inline void i2c_addr(I2C_TypeDef* i2c, uint8_t addr) {
    i2c->DR = addr << 1;
    while (!(i2c->SR1 & I2C_SR1_ADDR));
    i2c->SR2; // Check if optimised away.
}

inline void i2c_write(I2C_TypeDef* i2c, uint8_t byte) {
    while (!(i2c->SR1 & I2C_SR1_TXE));
    i2c->DR = byte;
    while (!(i2c->SR1 & I2C_SR1_BTF));
}

inline void i2c_write(I2C_TypeDef* i2c, uint8_t* data, uint8_t size) {
    uint8_t index = 0;
    while (size > index) {
        while (!(i2c->SR1 & I2C_SR1_TXE));
        i2c->DR = data[index];
        while (!(i2c->SR1 & I2C_SR1_BTF));
        index++;
    }
}

inline void i2c_stop(I2C_TypeDef* i2c) {
    i2c->CR1 |= I2C_CR1_STOP;
}

inline uint8_t i2c_read(I2C_TypeDef* i2c, uint8_t addr) {
    i2c->DR = addr << 1 | 1U;
    i2c->CR1 &= ~(I2C_CR1_ACK);
    while (!(i2c->SR1 & I2C_SR1_ADDR));
    i2c->SR2;
    i2c->CR1 |= I2C_CR1_STOP;
    while (!(i2c->SR1 & I2C_SR1_RXNE));
    return i2c->DR;
}

inline void i2c_read(I2C_TypeDef* i2c, uint8_t addr, uint8_t* buf, uint8_t size) {
    if (size == 1) {
        buf[0] = i2c_read(i2c, addr);
        return;
    }

    i2c->DR = addr << 1 | 1U;
    i2c->CR1 |= I2C_CR1_ACK;
    while (!(i2c->SR1 & I2C_SR1_ADDR));
    i2c->SR2;

    uint8_t index = 0;
    while (index < size - 2) {
        while (!(i2c->SR1 & I2C_SR1_RXNE));
        buf[index] = i2c->DR;
        i2c->CR1 |= I2C_CR1_ACK;
        index++;
    }

    while (!(i2c->SR1 & I2C_SR1_RXNE));
    buf[index++] = i2c->DR;
    i2c->CR1 &= ~(I2C_CR1_ACK); // Send nack for last byte
    i2c->CR1 |= I2C_CR1_STOP;
    while (!(i2c->SR1 & I2C_SR1_RXNE));
    buf[index] = i2c->DR;
}

#endif // I2C_H