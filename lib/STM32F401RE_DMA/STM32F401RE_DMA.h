// STM32F401RE_DMA.h
// Header for DMA functions

#ifndef STM32F4_DMA_H
#define STM32F4_DMA_H

#include <stdint.h>

///////////////////////////////////////////////////////////////////////////////
// Definitions
///////////////////////////////////////////////////////////////////////////////

#define __IO volatile

// Base addresses
#define DMA2_BASE (0x40026400UL) // base address of DMA2

///////////////////////////////////////////////////////////////////////////////
// Bitfield structs
///////////////////////////////////////////////////////////////////////////////
typedef struct {
    __IO uint32_t CFEIF4  : 1;
    __IO uint32_t         : 1;
    __IO uint32_t CDMEIF4 : 1;
    __IO uint32_t CTEIF4  : 1;
    __IO uint32_t CHTIF4  : 1;
    __IO uint32_t CTCIF4  : 1;
} HIFCR_bits;

typedef struct {
    __IO uint32_t EN         : 1;
    __IO uint32_t DMEIE      : 1;
    __IO uint32_t TEIE       : 1;
    __IO uint32_t HTIE       : 1;
    __IO uint32_t TCIE       : 1;
    __IO uint32_t PFCTRL     : 1;
    __IO uint32_t DIR        : 2;
    __IO uint32_t CIRC       : 1;
    __IO uint32_t PINC       : 1;
    __IO uint32_t MINC       : 1;
    __IO uint32_t PSIZE      : 2;
    __IO uint32_t MSIZE      : 2;    
    __IO uint32_t PINCOS     : 1;
    __IO uint32_t PL         : 2; 
    __IO uint32_t DBM        : 1;
    __IO uint32_t CT         : 1;
    __IO uint32_t            : 1;
    __IO uint32_t PBURST     : 2;
    __IO uint32_t MBURST     : 2;
    __IO uint32_t CHSEL      : 2;
} CR_bits;

typedef struct {
    __IO uint32_t   LISR;
    __IO uint32_t   HISR;
    __IO uint32_t   LIFCR;
    __IO HIFCR_bits   HIFCR;
} DMA_TypeDef;

typedef struct {
    __IO CR_bits    CR;
    __IO uint32_t   NDTR;
    __IO uint32_t   PAR;
    __IO uint32_t   M0AR;
    __IO uint32_t   M1AR;
    __IO uint32_t   FCR;
} DMA_STREAM;

#define DMA2 ((DMA_TypeDef *) DMA2_BASE)

// UART 1 is channel 4 of following streams
#define DMA2_STREAM5 ((DMA_STREAM *) (DMA2_BASE + 0x10 + 0x18 * 5)) //Rx
#define DMA2_STREAM7 ((DMA_STREAM *) (DMA2_BASE + 0x10 + 0x18 * 7)) //Tx

const uint16_t REQUEST_SIZE = 512;
const uint16_t RESPONSE_SIZE = 2048;
const uint8_t getRequest[REQUEST_SIZE];
const uint8_t htmlResponse[RESPONSE_SIZE];

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
///////////////////////////////////////////////////////////////////////////////

void initDMA();
// USART_TypeDef * initUSART(uint8_t USART_ID, uint32_t baud_rate);
// void sendChar(USART_TypeDef * USART, uint8_t data);
// void sendString(USART_TypeDef * USART, uint8_t * charArray);
// void receiveString(USART_TypeDef * USART, uint8_t * charArray);

#endif