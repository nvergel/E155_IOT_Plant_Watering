// STM32F401RE_ADC.c
// ADC function declarations

#include "STM32F401RE_ADC.h"
#include "STM32F401RE_RCC.h"
#include "STM32F401RE_GPIO.h"

void ADCInit() {
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

void ADCmeasure(uint16_t* array, int length) {
    ADC1->CR2.ADON = 1;
    
    ADC1->CR2.CONT = 1;
    ADC1->CR2.SWSTART = 1;

    for (int i = 0; i < length; ++i) {
        while(!ADC1->SR.EOC);
        array[i] = ADC1->DR.DR;
    }
    
    ADC1->CR2.CONT = 0;
    ADC1->CR2.ADON = 0;
}

uint16_t ADCsingle_measure() {
    uint16_t moisture;
    ADC1->CR2.ADON = 1;
    
    //ADC1->CR2.CONT = 1;
    ADC1->CR2.SWSTART = 1;

    while(!ADC1->SR.EOC);
    
    moisture = ADC1->DR.DR;

    
    //ADC1->CR2.CONT = 0;
    ADC1->CR2.ADON = 0;
    return moisture;
}
void moisturePercentage(uint16_t moisture){

}