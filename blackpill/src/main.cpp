#include <stdint.h>
#include <stm32f411xe.h>
#include <core_cm4.h>

#include "gpio.h"
#include "rcc.h"
#include "systick.h"
#include "usart.h"
#include "I2CAS5600.h"
#include <cstring>
#include <alloca.h>

void SystemInit() {
	SCB->CPACR |= ((3UL << 10 * 2) | (3UL << 11 * 2)); // enable fpu
	FLASH->ACR |= 3UL | FLASH_ACR_PRFTEN;
	RCC->CR |= RCC_CR_HSEON;
	while (!(RCC->CR & RCC_CR_HSERDY_Msk));
	RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSE | PLL_P << RCC_PLLCFGR_PLLP_Pos | PLL_Q << RCC_PLLCFGR_PLLQ_Pos | PLL_N << RCC_PLLCFGR_PLLN_Pos | PLL_M << RCC_PLLCFGR_PLLM_Pos;
	RCC->CFGR |= HPRE << RCC_CFGR_HPRE_Pos | PPRE1 << RCC_CFGR_PPRE1_Pos | PPRE2 << RCC_CFGR_PPRE2_Pos | RTCPRE << RCC_CFGR_RTCPRE_Pos;
	RCC->CR |= RCC_CR_PLLON_Msk;
	while (!(RCC->CR & RCC_CR_PLLRDY_Msk));
	RCC->CFGR |= SW;
	systick_init(1000);
}

// Buffer Stuffer
constexpr uint8_t bufSize = 32;
char buffer[bufSize];
uint8_t bufIndex = 0;
uint16_t config[4]{};
std::bitset<4> configWriteStatus;

USART_TypeDef* uart = USART2;

I2CAS5600 encoder{I2C1};

const char* commands[] = {
	"set",
	"write",
	"read",
	"clear",
	"burn"
};

void parse_command() {
	uint8_t size = 0;

	char* token = strtok(buffer, " ");
	size = strlen(token);

	char log[100];

	uint8_t i;
	for (i = 0; i < sizeof(commands); i++) {
		if (token == nullptr) {
			i = sizeof(commands) + 1;
			break;
		}
		if (strlen(commands[i]) == size) {
			if (strncmp(commands[i], token, size)) {
				continue;
			}
			switch (i) {
				case 0:
					token = strtok(nullptr, " ");
					uint8_t regIndex = regs::nameToIndex.at(std::string_view{token, size});
					
					if (regIndex == 0 || regIndex > 4) {
						uart_write_buf(uart, "Error: invalid register, stopping", 23);
						i = sizeof(commands) + 2;
						continue;
					}
					token = strtok(nullptr, " ");
					char* temp;
					config[regIndex - 1] = (uint16_t)strtol(token, &temp, 2);
					if (*temp != '\0' || !strcmp(token, temp)) {
						uart_write_buf(uart, "Error, invalid number.\n", 23);
						i = sizeof(commands) + 2;
						continue;
					}
					sprintf(log, "Wrote %u to %s.\n", config[regIndex - 1], regs::nameArray[regIndex].first.data());
					uart_write_buf(uart, log, strlen(log));
					token = strtok(nullptr, " ");
					break;
				case 1:
					break;
				case 2:
					break;
				case 3:
					break;
				case 4:
					break;
			}
		}
		else {
			continue;
		}
	}

	if (i == sizeof(commands)) {
		uart_write_buf(uart, "Error: Couldn't recognize command.\n", 23);
	}
	else if (i == sizeof(commands + 1)) {
		uart_write_buf(uart, "Completed commands successfully.\n", 33);
	}

}

int main() {
	SystemInit();

	uint16_t ledB = PIN('C', 13);
	gpio_set_mode(ledB, GPIO_MODE_OUTPUT, GPIO_OTYPE_PUSH_PULL, GPIO_OSPEED_LOW_SPEED, GPIO_PUPD_NONE);
	bool ledState = true;
	gpio_write(ledB, !ledState);
	ledState = !ledState;
	
	uart_init(uart, 115200);

	uint32_t timer = 0, period = 2000;
	
	while (true) {
		if (timer_expired(&timer, period, s_ticks)) {
			gpio_write(ledB, !ledState);
			ledState = !ledState;
		}

		if (uart_read_ready(uart)) {
			buffer[bufIndex] = uart_read_byte(uart);
			if (buffer[bufIndex] == '\n') {
				buffer[bufIndex] = '\0';
			}
		}
	}
}