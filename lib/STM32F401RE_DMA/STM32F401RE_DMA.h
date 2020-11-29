// STM32F401RE_DMA.h
// Header for DMA functions

#ifndef STM32F4_DMA_H
#define STM32F4_DMA_H

#include <stdint.h>
#include "STM32F401RE_USART.h"

///////////////////////////////////////////////////////////////////////////////
// Definitions
///////////////////////////////////////////////////////////////////////////////

#define __IO volatile

// Base addresses
#define DMA1_BASE (0x40026000UL) // base address of DMA1
#define DMA2_BASE (0x40026400UL) // base address of DMA2

///////////////////////////////////////////////////////////////////////////////
// Bitfield structs
///////////////////////////////////////////////////////////////////////////////

typedef struct {
    __IO uint32_t FEIF4  : 1;
    __IO uint32_t        : 1;
    __IO uint32_t DMEIF4 : 1;
    __IO uint32_t TEIF4  : 1;
    __IO uint32_t HTIF4  : 1;
    __IO uint32_t TCIF4  : 1;
    __IO uint32_t FEIF5  : 1;
    __IO uint32_t        : 1;
    __IO uint32_t DMEIF5 : 1;
    __IO uint32_t TEIF5  : 1;
    __IO uint32_t HTIF5  : 1;
    __IO uint32_t TCIF5  : 1;
    __IO uint32_t        : 4;
    __IO uint32_t FEIF6  : 1;
    __IO uint32_t        : 1;
    __IO uint32_t DMEIF6 : 1;
    __IO uint32_t TEIF6  : 1;
    __IO uint32_t HTIF6  : 1;
    __IO uint32_t TCIF6  : 1;
    __IO uint32_t FEIF7  : 1;
    __IO uint32_t        : 1;
    __IO uint32_t DMEIF7 : 1;
    __IO uint32_t TEIF7  : 1;
    __IO uint32_t HTIF7  : 1;
    __IO uint32_t TCIF7  : 1;
} HISR_bits;

typedef struct {
    __IO uint32_t CFEIF4  : 1;
    __IO uint32_t         : 1;
    __IO uint32_t CDMEIF4 : 1;
    __IO uint32_t CTEIF4  : 1;
    __IO uint32_t CHTIF4  : 1;
    __IO uint32_t CTCIF4  : 1;
    __IO uint32_t CFEIF5  : 1;
    __IO uint32_t         : 1;
    __IO uint32_t CDMEIF5 : 1;
    __IO uint32_t CTEIF5  : 1;
    __IO uint32_t CHTIF5  : 1;
    __IO uint32_t CTCIF5  : 1;
    __IO uint32_t         : 4;
    __IO uint32_t CFEIF6  : 1;
    __IO uint32_t         : 1;
    __IO uint32_t CDMEIF6 : 1;
    __IO uint32_t CTEIF6  : 1;
    __IO uint32_t CHTIF6  : 1;
    __IO uint32_t CTCIF6  : 1;
    __IO uint32_t CFEIF7  : 1;
    __IO uint32_t         : 1;
    __IO uint32_t CDMEIF7 : 1;
    __IO uint32_t CTEIF7  : 1;
    __IO uint32_t CHTIF7  : 1;
    __IO uint32_t CTCIF7  : 1;
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
    __IO uint32_t CHSEL      : 3;
} DMA_CR_bits;

typedef struct {
    __IO uint32_t   LISR;
    __IO HISR_bits   HISR;
    __IO uint32_t   LIFCR;
    __IO HIFCR_bits   HIFCR;
} DMA_TypeDef;

typedef struct {
    __IO DMA_CR_bits  CR;
    __IO uint32_t     NDTR;
    __IO uint32_t     PAR;
    __IO uint32_t     M0AR;
    __IO uint32_t     M1AR;
    __IO uint32_t     FCR;
} DMA_STREAM;

#define DMA1 ((DMA_TypeDef *) DMA1_BASE)
#define DMA2 ((DMA_TypeDef *) DMA2_BASE)

#define DMA1_STREAM6 ((DMA_STREAM *) (DMA1_BASE + 0x10 + 0x18 * 6))
#define DMA2_STREAM7 ((DMA_STREAM *) (DMA2_BASE + 0x10 + 0x18 * 7))

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
///////////////////////////////////////////////////////////////////////////////

void initDMA();
void sendData(uint8_t* str, uint16_t strLen, USART_TypeDef* ESP_USART);
void printData(uint8_t* str);

#endif