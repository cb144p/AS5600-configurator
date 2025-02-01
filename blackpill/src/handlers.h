#ifndef HANDLERS_H
#define HANDLERS_H

#include <stm32f411xe.h>
#include <vector>
#include "usart.h"

constexpr uint8_t nvicRegCount = 85; // Number of additional interrupt spots on top of cm4 base 16.

void initializeBaseHandlers() {
	NVIC_SetPriority(IRQn_Type::UsageFault_IRQn, 1);
	NVIC_SetPriority(IRQn_Type::BusFault_IRQn, 2);
	NVIC_SetPriority(IRQn_Type::MemoryManagement_IRQn, 3);
	NVIC_EnableIRQ(IRQn_Type::UsageFault_IRQn);
	NVIC_EnableIRQ(IRQn_Type::BusFault_IRQn);
	NVIC_EnableIRQ(IRQn_Type::MemoryManagement_IRQn);
}

void Usage_Handler() {
	// FIX IF PERMANANT TO CHECK FOR ENABLED UART.
	uart_write_buf(USART_DEBUG, "Usage Fault.\n\r", 15);
}

void Bus_Handler() {
	// FIX IF PERMANANT TO CHECK FOR ENABLED UART.
	uart_write_buf(USART_DEBUG, "Bus Fault.\n\r", 13);
}

void Memory_Handler() {
	// FIX IF PERMANANT TO CHECK FOR ENABLED UART.
	uart_write_buf(USART_DEBUG, "Memory Fault.\n\r", 16);
}

void Hard_Handler() {
	// FIX IF PERMANANT TO CHECK FOR ENABLED UART.
	uart_write_buf(USART_DEBUG, "Hard Fault.\n\r", 14);
}

#endif // HANDLERS_H