// Standard library includes.
#include <stdint.h>
#include "STM32F401RE_DMA.h"
#include "STM32F401RE_RCC.h"
#include "STM32F401RE_USART.h"
#include "STM32F401RE_TIM.h"

void setupTx(DMA_STREAM* DMAx_STREAMy, uint8_t PL) {
    DMAx_STREAMy->CR.EN = 0;

    // Channel 4
    DMAx_STREAMy->CR.CHSEL = 4;

    // Set byte size
    DMAx_STREAMy->CR.PSIZE = 0;
    DMAx_STREAMy->CR.MSIZE = 0;

    // memory to peripheral
    DMAx_STREAMy->CR.DIR = 1;
    DMAx_STREAMy->CR.CIRC = 0;

    // Memory pointer increment
    DMAx_STREAMy->CR.MINC = 1;

    // High priority level
    DMAx_STREAMy->CR.PL = PL;
    
}

void initDMA() {

    RCC->AHB1ENR.DMA1EN = 1;
    RCC->AHB1ENR.DMA2EN = 1;

    // Set USART to DMAT
    USART1->CR3 = (0b10 << 6);
    USART2->CR3 = (0b10 << 6);

    setupTx(DMA1_STREAM6, 1);
    setupTx(DMA2_STREAM7, 2);

    DMA1_STREAM6->PAR   = (uint32_t) &(USART2->DR);
    DMA2_STREAM7->PAR   = (uint32_t) &(USART1->DR);

    // Setup Rx for ESP
    // DMA2_STREAM5->CR.EN = 0;
    // // Channel 4
    // DMA2_STREAM5->CR.CHSEL = 4;
    // // Set byte size
    // DMA2_STREAM5->CR.PSIZE = 0;
    // DMA2_STREAM5->CR.MSIZE = 0;
    // // peripheral to memory (Rx)
    // DMA2_STREAM5->CR.DIR = 0;
    // DMA2_STREAM5->CR.CIRC = 0;
    // // Memory pointer increment
    // DMA2_STREAM5->CR.MINC = 0;
    // // High priority level
    // DMA2_STREAM5->CR.PL = 2;
}

void sendData(uint8_t* cmd, USART_TypeDef* ESP_USART) {
    DMA1_STREAM6->CR.EN = 0;
    DMA2_STREAM7->CR.EN = 0;
    DMA1->HIFCR.CTCIF6 = 1;
    DMA2->HIFCR.CTCIF7 = 1;

    // Set data to transfer
    uint16_t cmdLen = strlen(cmd);
    // DMA2_STREAM7->M0AR  = (uint32_t) &cmd;
    // DMA2_STREAM7->NDTR  = (uint16_t) cmdLen;
    // DMA2_STREAM7->CR.EN = 1;
    // USART1->CR1.TXEIE = 1;

    // while(!DMA2->HISR.TCIF7);
    // USART1->CR1.TXEIE = 0;

    // Should print str but using to debug rn
    DMA1_STREAM6->M0AR  = (uint32_t) &cmd;
    DMA1_STREAM6->NDTR  = (uint16_t) cmdLen;
    DMA1_STREAM6->CR.EN = 1;
    //USART2->CR1.TXEIE = 1;
    while(!DMA1->HISR.TCIF6);
    //USART2->CR1.TXEIE = 0;
    
    // uint8_t volatile str[512] = "";
    // readString(ESP_USART, str);
    // uint16_t strLen = strlen(str);
    // DMA1_STREAM6->M0AR  = (uint32_t) &str;
    // DMA1_STREAM6->NDTR  = (uint16_t) strLen;
    // DMA1_STREAM6->CR.EN = 1;
    // USART2->CR1.TXEIE = 1;
}