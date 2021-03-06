// STM32F401RE_ADC.c
// ADC function declarations

#include "STM32F401RE_ADC.h"
#include "STM32F401RE_RCC.h"
#include "STM32F401RE_GPIO.h"

void initADC() {
    // Turn on GPIOA clock domains
    RCC->AHB1ENR.GPIOAEN = 1;
    
    RCC->APB2ENR |= (1 << 8); // Turn on ADC1 clock domain (ADC1EN bit in APB2ENR)

    // Initially assigning ADC pins
    pinMode(GPIOA, 0, GPIO_ANALOG); // PA0

    // set clock divider to 6
    ADC1_CCR->ADCPRE = 0b10;

    ADC1->SQR1 = 0;
    ADC1->SQR2 = 0;
    ADC1->SQR3 = 0;
}

uint16_t ADCmeasure() {
    ADC1->CR2.ADON = 1;
    ADC1->CR2.SWSTART = 1;

    while(!ADC1->SR.EOC);
    uint16_t moisture = ADC1->DR.DR;

    ADC1->CR2.ADON = 0;
    return moisture;
}