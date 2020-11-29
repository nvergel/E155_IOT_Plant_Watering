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

void printData(uint8_t* str) {
    DMA1_STREAM6->CR.EN = 0;
    DMA1->HIFCR.CTCIF6 = 1;
    uint16_t strLen = strlen(str);
    DMA1_STREAM6->M0AR  = str;
    DMA1_STREAM6->NDTR  = strLen;
    DMA1_STREAM6->CR.EN = 1;
}

void sendData(uint8_t* cmd, USART_TypeDef* ESP_USART) {
    //disable stream 7
    DMA2_STREAM7->CR.EN = 0;
    //set clear transfer complete interrupt flag, clearing it
    DMA2->HIFCR.CTCIF7 = 1;

    // Set data to transfer
    uint16_t cmdLen = strlen(cmd);
    //set base address from which data will be read/written
    DMA2_STREAM7->M0AR  = cmd;
    //set number of data items to be transferred to cmdLen
    DMA2_STREAM7->NDTR  = cmdLen;
    //enable stream 7
    DMA2_STREAM7->CR.EN = 1;
    //while stream 7 transmit complete interrupt flag is low
    while(!DMA2->HISR.TCIF7);

    // Should print str but using to debug rn
    // DMA1_STREAM6->M0AR  = cmd;
    // DMA1_STREAM6->NDTR  = cmdLen;
    // DMA1_STREAM6->CR.EN = 1;
    // while(!DMA1->HISR.TCIF6);
    
    uint8_t str[512] = "";
    readString(ESP_USART, str);
    printData(str);
}