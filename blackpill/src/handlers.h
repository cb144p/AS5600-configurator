#ifndef HANDLERS_H
#define HANDLERS_H

#include "usart.h"

void initializeHandlerPriorities() {
	*(unsigned long int*)0xE000ED18 |= 0b0001 << 20 | 0b0010 << 12 | 0b0011 << 4; // Sets usage to 1, bus to 2 and memory to 3.
	//*(unsigned long int*)0xE000E100 |= 1UL << 6 | 1UL << 5 | 1UL << 4; // Enables interrupts 4, 5, and 6. I think these are irq related.
	*(unsigned long int*)0xE000ED24 |= 0b111 << 16; // Enables usage, bus, and memory fault handlers
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