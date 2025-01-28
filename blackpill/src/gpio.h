#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

#include <stm32f411xe.h>


#define GPIO(bank) ((GPIO_TypeDef *) (GPIOA + 0x400 * bank))
#define PIN(bank, num) (((bank - 'A') << 8) | num)
#define PINNO(pin) (pin & 255)
#define PINBANK(pin) (pin >> 8)

enum {GPIO_MODE_INPUT, GPIO_MODE_OUTPUT, GPIO_MODE_AF, GPIO_MODE_ANALOG};
enum {GPIO_OTYPE_PUSH_PULL, GPIO_OTYPE_OPEN_DRAIN};
enum {GPIO_OSPEED_LOW_SPEED, GPIO_OSPEED_MEDIUM_SPEED, GPIO_OSPEED_FAST_SPEED, GPIO_OSPEED_HIGH_SPEED};
enum {GPIO_PUPD_NONE, GPIO_PUPD_UP, GPIO_PUPD_DOWN};

inline void gpio_set_mode(uint16_t pin, uint8_t mode, uint8_t type, uint8_t speed, uint8_t pupd) {
	GPIO_TypeDef* gpio = GPIO(PINBANK(pin));
	uint8_t n = PINNO(pin);
	RCC->AHB1ENR |= (1U << PINBANK(pin));
	gpio->MODER &= ~(3U << (n* 2)); // Clear bits
	gpio->MODER |= (mode & 3) << (n * 2); // Set bits
	gpio->OTYPER &= ~(3U << (n));
	gpio->OTYPER |= (type & 3) << (n);
	gpio->OSPEEDR &= ~(3U << (n* 2));
	gpio->OSPEEDR |= (speed & 3) << (n * 2);
	gpio->PUPDR &= ~(3U << (n* 2));
	gpio->PUPDR |= (pupd & 3) << (n * 2);
}

// memcpy replacement for registers as they do not work with memcpy.
inline void regcpy(volatile void *dest, const void *src, uint32_t size) {
	for (uint32_t i = 0; i < size / 4; i++) {
		((uint32_t *)dest)[i] = ((const uint32_t *)src)[i];
	}
	if (size % 4) {
		uint32_t fourMult = (size & ~((uint32_t)0b11));
		((uint32_t *)dest)[size / 4] = ((const uint8_t *)src)[fourMult];
		for (uint32_t i = 1; i < size % 4; i++) {
			((uint32_t *)dest)[size / 4] |= ((const uint8_t *)src)[fourMult + i] << (i * 8);
		}
	}
}                                                                                                                                                                                                                                       

inline void regcpy(void *dest, const volatile void *src, uint32_t size) {
	for (uint32_t i = 0; i < size / 4; i++) {
		((uint32_t *)dest)[i] = ((const uint32_t *)src)[i];
	}
	if (size % 4) {
		uint32_t fourMult = (size & ~((uint32_t)0b11));
		((uint32_t *)dest)[size / 4] = ((const uint8_t *)src)[fourMult];
		for (uint32_t i = 1; i < size % 4; i++) {
			((uint32_t *)dest)[size / 4] |= ((const uint8_t *)src)[fourMult + i] << (i * 8);
		}
	}
}

static inline void gpio_set_af(uint16_t pin, uint8_t af_num) {
	GPIO_TypeDef* gpio = GPIO(PINBANK(pin));
	uint8_t n = PINNO(pin);
	gpio->AFR[n >> 3] &= ~(15UL << ((n & 7) * 4));
	gpio->AFR[n >> 3] |= ((uint32_t)af_num) << ((n & 7) * 4);
}

static inline void gpio_write(uint16_t pin, bool val) {
	GPIO_TypeDef* gpio = GPIO(PINBANK(pin));
	gpio->BSRR = val << PINNO(pin);
}

static inline bool gpio_read(uint16_t pin) {
	return GPIO(PINBANK(pin))->IDR & (1UL << PINNO(pin));
}

#endif // GPIO_H