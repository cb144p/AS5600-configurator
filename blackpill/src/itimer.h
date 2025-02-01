#ifndef TIMERHELPER_H
#define TIMERHELPER_H

#include <stdint.h>
#include <stm32f411xe.h>
#include <cmath>

#include "rcc.h"
#include "gpio.h"

class ITimer {
public:
    TIM_TypeDef* tim = nullptr;
    uint32_t arv = 0;
    uint16_t psc = 0; // Real prescaler value, not just the one in the register.
    uint8_t status = 0; // TODO: do something with this bit

    ITimer() {}
    ~ITimer() {}

    ITimer(TIM_TypeDef* _tim, uint16_t _psc, uint32_t _arv) {
        tim = _tim;
        psc = _psc;
        arv = _arv;

        tim->PSC = psc - 1;
        tim->ARR = arv;
    }

    static ITimer PWM(TIM_TypeDef* _tim, uint16_t pin, uint8_t channel, uint32_t frequency) {
        ITimer iTim{};
        iTim.tim = _tim;
        gpio_set_mode(pin, GPIO_MODE_AF, GPIO_OTYPE_OPEN_DRAIN, GPIO_OSPEED_HIGH_SPEED, GPIO_PUPD_NONE);
        
        if ((uint32_t)iTim.tim < TIM1_BASE) {
            uint8_t bs = (uint32_t)(iTim.tim - TIM2_BASE)/0x400;

            RCC->APB1ENR |= RCC_APB1ENR_TIM2EN << bs;
        }
        else if ((uint32_t)iTim.tim == TIM1_BASE) {
            RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
        }
        else {
            RCC->APB2ENR |= RCC_APB2ENR_TIM9EN << ((uint32_t)(iTim.tim - TIM9_BASE)/0x400 + RCC_APB2ENR_TIM9EN_Pos);
        }

        iTim.tim->SMCR &= ~TIM_SMCR_SMS_Msk;
        iTim.tim->PSC = (frequency <= AHB_FREQUENCY / 65536) ? 65535 : AHB_FREQUENCY / frequency;
        iTim.tim->ARR = AHB_FREQUENCY / (iTim.tim->PSC + 1) / frequency;
        iTim.psc = iTim.tim->PSC + 1;
        iTim.arv = iTim.tim->ARR;
        iTim.addPWMChannel(pin, channel);
        iTim.tim->CR1 |= 0b1;
    }

    static ITimer periodicInit(TIM_TypeDef* _tim, float frequency) {
        ITimer iTim{};
        iTim.tim = _tim;
        
        if ((uint32_t)iTim.tim < TIM1_BASE) {
            uint8_t bs = (uint32_t)(iTim.tim - TIM2_BASE)/0x400;

            RCC->APB1ENR |= RCC_APB1ENR_TIM2EN << bs;
        }
        else if ((uint32_t)iTim.tim == TIM1_BASE) {
            RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
        }
        else {
            RCC->APB2ENR |= RCC_APB2ENR_TIM9EN << ((uint32_t)(iTim.tim - TIM9_BASE)/0x400 + RCC_APB2ENR_TIM9EN_Pos);
        }

        iTim.tim->SMCR &= ~TIM_SMCR_SMS_Msk;
        iTim.setFrequency(frequency);
        iTim.tim->CR1 |= 0b1;
    }

    /**
     * Approximation of frequency, cannot be guarunteed to be very accurate.
     */
    void setFrequency(float frequency) {
        tim->PSC = (uint16_t)ceilf((float)AHB_FREQUENCY / 65536.0f / frequency) - 1;
        tim->ARR = uint16_t((float)AHB_FREQUENCY / (tim->PSC + 1) / frequency);
        psc = tim->PSC + 1;
        arv = tim->ARR;
    }

    // RM uses 1 based indexing, but I am using 0.
    
    void addPWMChannel(uint16_t pin, uint8_t channel) {
        if ((uint32_t)tim == TIM1_BASE || (uint32_t)TIM2_BASE) {
            gpio_set_af(pin, 1);
        }
        else if ((uint32_t)tim >= TIM9_BASE) {
            gpio_set_af(pin, 3);
        }
        else {
            gpio_set_af(pin, 2);
        }

        if (channel <= 1) {
            tim->CCMR1 |= (0b110 << TIM_CCMR1_OC1M_Pos | TIM_CCMR1_OC1PE) << channel * 8;
        }
        else {
            tim->CCMR2 |= (0b110 << TIM_CCMR2_OC3M_Pos | TIM_CCMR2_OC3PE) << (channel - 2) * 8;
        }

        tim->CCER |= 0b11 << channel;
    }

    void addEmptyPWMChannel(uint8_t channel) {
        if (channel <= 1) {
            tim->CCMR1 |= (0b110 << TIM_CCMR1_OC1M_Pos | TIM_CCMR1_OC1PE) << channel * 8;
        }
        else {
            tim->CCMR2 |= (0b110 << TIM_CCMR2_OC3M_Pos | TIM_CCMR2_OC3PE) << (channel - 2) * 8;
        }

        tim->CCER |= 0b11 << channel;
    }

    void writePWMCount(uint16_t count, uint8_t channel) {
        (&(tim->CCR1))[channel] = count;
    }

    // 0 to 1
    void writePWMDuty(float dutycycle, uint8_t channel) {
        (&(tim->CCR1))[channel] = arv * dutycycle;
    }

    void writePWMS(float seconds, uint8_t channel) {
        (&(tim->CCR1))[channel] = (uint32_t)(seconds * AHB_FREQUENCY / psc);
    }

    inline void enableResetInterrupt() {
        tim->DIER |= TIM_DIER_UIE;
    }

    inline void enableInterruptChannel(uint8_t channel) {
        tim->DIER |= TIM_DIER_CC1IE << channel;
    }

};

#endif // TIMERHELPER_H