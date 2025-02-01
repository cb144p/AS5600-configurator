#include <stdint.h>
#include <stm32f411xe.h>
#include <core_cm4.h>

#include "gpio.h"
#include "rcc.h"
#include "systick.h"
#include "usart.h"
#include "I2CAS5600.h"
#include "itimer.h"
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

I2CAS5600 encoder;

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
		if (strncmp(commands[i], token, size) == 0) {
			token = strtok(nullptr, " ");
			if (i != 3 && token == nullptr) {
				sprintf(log, "Error: %s requires a parameter", commands[i]);
				uart_write_buf(uart, log, 28 + strlen(commands[i]));
				i = sizeof(commands) + 2;
				continue;
			}
			else {
				size = strlen(token);
			}
			switch (i) {
				case 0:
					uint8_t regIndex = regs::nameToIndex.at(std::string_view{token, size});
					
					if (regIndex == 0 || regIndex > 4) {
						uart_write_buf(uart, "Error: invalid register, stopping", 23);
						i = sizeof(commands) + 2;
						continue;
					}
					token = strtok(nullptr, " ");
					char* temp;
					config[regIndex - 1] = (uint16_t)strtoul(token, &temp, 2);
					if (*temp != '\0' || !strcmp(token, temp)) {
						uart_write_buf(uart, "Error, invalid number.\n", 23);
						i = sizeof(commands) + 2;
						continue;
					}
					configWriteStatus[regIndex - 1] = 1;
					sprintf(log, "Wrote %u to %s.\n", config[regIndex - 1], regs::nameArray[regIndex].first.data());
					uart_write_buf(uart, log, strlen(log));
					token = strtok(nullptr, " ");
					i = 0;
					break;
				case 1:
					char* temp;
					uint16_t reg = (uint16_t)strtoul(token, &temp, 2);
					token = strtok(nullptr, " ");
					uint16_t val = (uint16_t)strtoul(token, &temp, 2);
					encoder.writeByte(reg, val);
					sprintf(log, "Wrote %u to %u.\n", val, reg);
					temp = log;
					while (*temp != '\0') {
						uart_write_byte(uart, *temp);
					}
					i = 0;
					break;
				case 2:
					uint8_t regIndex = regs::nameToIndex.at(std::string_view{token, size});
					
					if (regIndex == 0 || regIndex > 4) {
						uart_write_buf(uart, "Error: invalid register, stopping", 23);
						i = sizeof(commands) + 2;
						continue;
					}
					i2c_start(I2C1);
					i2c_addr(I2C1, AS5600ID);
					i2c_write(I2C1, regs::addrs[regIndex]);
					uint8_t vals[2];
					i2c_read(I2C1, AS5600ID, vals, regs::sizes[regIndex] + 1);
					sprintf(log, "Wrote %u to %u.\n", (uint16_t)vals[1] << 8 | vals[0], reg);
					char* temp;
					temp = log;
					while (*temp != '\0') {
						uart_write_byte(uart, *temp);
					}
					i = 0;
					break;
				case 3:
					configWriteStatus = 0;
					i = 0;
					break;
				case 4:
					if (strncmp(token, "setting", 8) == 0) {
						for (uint8_t i = 0; i < 4; i++) {
							if (configWriteStatus[i]) {
								encoder.writeByte(regs::addrs[i + 1], config[i]);
							}
						}
						if (configWriteStatus.any()) {
							encoder.writeByte(0xFF, 0x40);
						}
						sprintf(log, "Wrote %u configs and burned the settings.\n", configWriteStatus.count());
						uart_write_buf(uart, log, 41);
					} else if (strncmp(token, "angle", 8) == 0) {
						for (uint8_t i = 0; i < 4; i++) {
							if (configWriteStatus[i]) {
								encoder.writeByte(regs::addrs[i + 1], config[i]);
							}
						}
						if (configWriteStatus.any()) {
							encoder.writeByte(0xFF, 0x80);
						}
						sprintf(log, "Wrote %u configs and burned the angle.\n", configWriteStatus.count());
						uart_write_buf(uart, log, 38);
					} else {
						uart_write_buf(uart, "Error: invalid burn command. Valid options are angle and setting.\n", 67);
						i = sizeof(commands) + 2;
					}
					i = 0;
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

uint16_t ledB = PIN('C', 13);
bool ledState = true;

int main() {
	SystemInit();

	gpio_set_mode(ledB, GPIO_MODE_OUTPUT, GPIO_OTYPE_PUSH_PULL, GPIO_OSPEED_LOW_SPEED, GPIO_PUPD_NONE);
	gpio_write(ledB, !ledState);
	ledState = !ledState;
	NVIC_SetPriority(IRQn_Type::TIM2_IRQn, 4);
	NVIC_SetVector(IRQn_Type::TIM2_IRQn, (uint32_t)((void (*)(void))([](){
		ledState = !ledState;
		gpio_write(ledB, ledState);
	})));
	ITimer ledKeeper = ITimer::periodicInit(TIM2, 1);
	ledKeeper.enableResetInterrupt();
	ledKeeper.enableInterruptChannel(0);
	ledKeeper.addEmptyPWMChannel(0);
	ledKeeper.writePWMDuty(.5, 0);

	
	
	uart_init(uart, 115200);

	gpio_set_mode(PIN('B', 7), GPIO_MODE_AF, GPIO_OTYPE_OPEN_DRAIN, GPIO_OSPEED_HIGH_SPEED, GPIO_PUPD_UP); // SDA
	gpio_set_mode(PIN('B', 6), GPIO_MODE_AF, GPIO_OTYPE_OPEN_DRAIN, GPIO_OSPEED_HIGH_SPEED, GPIO_PUPD_UP); // SCL
	gpio_set_af(PIN('B', 7), 4);
	gpio_set_af(PIN('B', 6), 4);
	encoder = I2CAS5600(I2C1);
	
	while (true) {
		if (uart_read_ready(uart)) {
			buffer[bufIndex] = uart_read_byte(uart);
			if (buffer[bufIndex] == '\r') {
				buffer[bufIndex] = '\0';
				while (!uart_read_ready(uart));
				ledKeeper.setFrequency(60);
				parse_command();
				ledKeeper.writePWMDuty(0.5, 0);
			}
		}
	}
}