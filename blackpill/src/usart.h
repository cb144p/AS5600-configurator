#ifndef USART_H
#define USART_H

#include <stdint.h>
#include <stm32f411xe.h>

#include "rcc.h"
#include "gpio.h"

#define USART_DEBUG USART2


inline void uart_init(USART_TypeDef* uart, uint32_t baud) { // TODO: Add support for selectable pins and consolidate the initialization functions.

	uint8_t af = 7; // USARTs, CAN, and GPCOMP6???
	uint16_t tx, rx;
	uint32_t freq = 0; // Bus Freq, not baud rate.



	if (uart == USART1) {
		freq = APB2_FREQUENCY;
		tx = PIN('A', 9);
		rx = PIN('A', 10);
		RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	} else if (uart == USART2) {
		freq = APB1_FREQUENCY;
		tx = PIN('A', 2);
		rx = PIN('A', 3);
		RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
        
	} else if (uart == USART6) {
		freq = APB2_FREQUENCY;
		tx = PIN('A', 11);
		rx = PIN('A', 12);
		RCC->APB2ENR |= RCC_APB2ENR_USART6EN;
	}

	gpio_set_mode(tx, GPIO_MODE_AF, GPIO_OTYPE_PUSH_PULL, GPIO_OSPEED_HIGH_SPEED, GPIO_PUPD_NONE);
	gpio_set_af(tx, af);
	gpio_set_mode(rx, GPIO_MODE_AF, GPIO_OTYPE_PUSH_PULL, GPIO_OSPEED_HIGH_SPEED, GPIO_PUPD_NONE);
	gpio_set_af(rx, af);

	uart->CR1 &= ~(USART_CR1_UE);
	uart->BRR = (uint16_t)(freq/(16*baud))<<USART_BRR_DIV_Mantissa_Pos | freq/baud % 16;
	uart->CR1 |= USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

inline void transfer_only_init(USART_TypeDef* uat, uint32_t baud) {

	uint8_t af = 7; // USARTs, CAN, and GPCOMP6???
	uint16_t tx;
	uint32_t freq = 0; // Bus Freq, not baud rate.

	if (uat == USART1) {
		freq = APB2_FREQUENCY;
		tx = PIN('A', 9);
		RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	} else if (uat == USART2) {
		freq = APB1_FREQUENCY;
		tx = PIN('A', 2);
		RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
        
	} else if (uat == USART6) {
		freq = APB2_FREQUENCY;
		tx = PIN('A', 11);
		RCC->APB2ENR |= RCC_APB2ENR_USART6EN;
	}

	gpio_set_mode(tx, GPIO_MODE_AF, GPIO_OTYPE_PUSH_PULL, GPIO_OSPEED_HIGH_SPEED, GPIO_PUPD_NONE);
	gpio_set_af(tx, af);

	uat->CR1 &= ~(USART_CR1_UE);
	uat->BRR = (uint16_t)(freq/(16*baud))<<USART_BRR_DIV_Mantissa_Pos | freq/baud % 16;
	uat->CR1 |= USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

inline uint32_t uart_read_ready(USART_TypeDef* uart) {
	return uart->DR & USART_SR_RXNE_Msk;
}

inline uint8_t uart_read_byte(USART_TypeDef* uart) {
	return (uint8_t) (uart->DR & 255);
}

inline void uart_write_byte(USART_TypeDef* uart, uint8_t byte) {
	uart->DR = byte;
	while (!(uart->SR & USART_SR_TXE_Msk)) (void) 0;
}

inline void uart_write_buf(USART_TypeDef* uart, const char* buf, unsigned int length) {
	while (length-- > 0) {
		uart_write_byte(uart, *(uint8_t*) buf++);
	}
}


#endif // USART_H