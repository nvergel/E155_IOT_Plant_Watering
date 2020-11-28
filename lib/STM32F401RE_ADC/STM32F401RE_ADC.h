// STM32F401RE_ADC.h
// Header for ADC functions

#ifndef STM32F4_ADC_H
#define STM32F4_ADC_H

#include <stdint.h> // Include stdint header

///////////////////////////////////////////////////////////////////////////////
// Definitions
///////////////////////////////////////////////////////////////////////////////

#define ADC1_BASE (0x40012000UL)
#define __IO volatile

///////////////////////////////////////////////////////////////////////////////
// Bitfield structs
///////////////////////////////////////////////////////////////////////////////


typedef struct {
  __IO uint32_t AWD       : 1;
  __IO uint32_t EOC       : 1;
  __IO uint32_t JEOC      : 1;
  __IO uint32_t JSTRT     : 1;
  __IO uint32_t STRT      : 1;
  __IO uint32_t OVR       : 1;
} ADC_SR_bits;

typedef struct {
  __IO uint32_t ADON        : 1;
  __IO uint32_t CONT        : 1;
  __IO uint32_t             : 6;
  __IO uint32_t DMA         : 1;
  __IO uint32_t DDS         : 1;
  __IO uint32_t EOCS        : 1;
  __IO uint32_t ALIGN       : 1;
  __IO uint32_t             : 4;
  __IO uint32_t JEXTSEL     : 4;
  __IO uint32_t JEXTEN      : 2;
  __IO uint32_t JSWSTART    : 1;
  __IO uint32_t             : 1;
  __IO uint32_t EXTSEL      : 4;
  __IO uint32_t EXTEN       : 2;
  __IO uint32_t SWSTART     : 1;
} ADC_CR2_bits;

typedef struct {
  __IO uint32_t DR  : 16;
} ADC_DR_bits;

typedef struct {
  __IO uint32_t        : 16;
  __IO uint32_t ADCPRE : 2;
} ADC_CCR_bits;


typedef struct {
  __IO ADC_SR_bits SR;      /*!< ADC status register,                  Address offset: 0x00 */
  __IO uint32_t CR1;        /*!< ADC control register 1 ,              Address offset: 0x04 */
  __IO ADC_CR2_bits CR2;    /*!< ADC control register 2,               Address offset: 0x08 */
  __IO uint32_t SMPR1;      /*!< ADC sample time 1,                    Address offset: 0x0C */
  __IO uint32_t SMPR2;      /*!< ADC sample time 2,                    Address offset: 0x10 */
  __IO uint32_t JOFR1;      /*!< ADC offset,                           Address offset: 0x14 */
  __IO uint32_t JOFR2;      /*!< ADC offset,                           Address offset: 0x18 */
  __IO uint32_t JOFR3;      /*!< ADC offset,                           Address offset: 0x1C */
  __IO uint32_t JOFR4;      /*!< ADC offset,                           Address offset: 0x20 */
  __IO uint32_t HTR;        /*!< ADC watchdog high,                    Address offset: 0x24 */
  __IO uint32_t LTR;        /*!< ADC watchdog low,                     Address offset: 0x28 */
  __IO uint32_t SQR1;       /*!< ADC regular sequence 1,               Address offset: 0x2C */
  __IO uint32_t SQR2;       /*!< ADC regular sequence 2,               Address offset: 0x30 */
  __IO uint32_t SQR3;       /*!< ADC regular sequence 3,               Address offset: 0x34 */
  __IO uint32_t JSQR;       /*!< ADC injected sequence,                Address offset: 0x38 */
  __IO ADC_DR_bits JDR1;    /*!< ADC injected data register,           Address offset: 0x3C */
  __IO ADC_DR_bits JDR2;    /*!< ADC injected data register,           Address offset: 0x40 */
  __IO ADC_DR_bits JDR3;    /*!< ADC injected data register,           Address offset: 0x44 */
  __IO ADC_DR_bits JDR4;    /*!< ADC injected data register,           Address offset: 0x48 */
  __IO ADC_DR_bits DR;      /*!< ADC regular data register,            Address offset: 0x4C */
} ADC_TypeDef;

// Pointers to GPIO-sized chunks of memory for each peripheral
#define ADC1 ((ADC_TypeDef *) ADC1_BASE)
#define ADC1_CCR ((ADC_CCR_bits *) (ADC1_BASE + 0x300 + 0x04)) 

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
///////////////////////////////////////////////////////////////////////////////

void initADC();

uint16_t ADCmeasure();

#endif