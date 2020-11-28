// Standard library includes.
#include <stdint.h>
#include "STM32F401RE_DMA.h"
#include "STM32F401RE_RCC.h"
#include "STM32F401RE_USART.h"
#include "STM32F401RE_TIM.h"

#define TIM TIM6
#define DMA_STREAM DMA1_Stream6
/**
 * Main program.
 */
void initDMA() {

    RCC->AHB1ENR.DMA1EN = 1;

    // Set PA2 to ALT function
    // pinMode(GPIOA, GPIO_PA2, GPIO_ALT);
    // Configure pin modes as ALT function
    // GPIOA->AFRH ;
    //AFR[0] &= ~(GPIO_AFRL_AFSEL2_Msk | GPIO_AFRL_AFSEL3_Msk);
    // Configure correct alternate functions (AF07)
    //GPIOA->AFR[0] |= (0b0111 << GPIO_AFRL_AFSEL2_Pos | 0b0111 << GPIO_AFRL_AFSEL3_Pos);

    // DMA1 configuration (channel 3 / stream 6)
    // SxCR register:
    // - Memory-to-peripheral
    // - Increment memory ptr, don't increment periph ptr.
    // - 8-bit data size for both source and destination.
    // - High priority (2/3).

    // Reset DMA stream.
    DMA2_STREAM5->CR.EN = 0;
    DMA2_STREAM7->CR.EN = 0;

    // Channel 4
    DMA2_STREAM5->CR.CHSEL = 4;
    DMA2_STREAM7->CR.CHSEL = 4;

    // Set byte size
    DMA2_STREAM5->CR.PSIZE = 0;
    DMA2_STREAM7->CR.PSIZE = 0;
    DMA2_STREAM5->CR.MSIZE = 0;
    DMA2_STREAM7->CR.MSIZE = 0;

    // peripheral to memory (Rx)
    DMA2_STREAM5->CR.DIR = 0;

    // memory to peripheral (Tx)
    DMA2_STREAM7->CR.DIR = 1;

    // use circular buffer?

    // Memory pointer increment
    DMA2_STREAM5->CR.MINC = 1;
    DMA2_STREAM7->CR.MINC = 0;

    // High priority level
    DMA2_STREAM5->CR.PL = 2;
    DMA2_STREAM7->CR.PL = 2;
    
    // Set DMA source and destination addresses.
    // Source: Address of the character array buffer in memory.
    DMA2_STREAM5->M0AR  = (uint32_t) &getRequest;
    // Dest.: USART data register
    DMA2_STREAM5->PAR   = (uint32_t) &(USART1->DR);
    // Set DMA data transfer length (# of samples).
    DMA2_STREAM5->NDTR  = (uint16_t) REQUEST_SIZE;
    
    // Set DMA source and destination addresses.
    // Source: Address of the character array buffer in memory.
    DMA2_STREAM7->M0AR  = (uint32_t) &htmlResponse;
    // Dest.: USART data register
    DMA2_STREAM7->PAR   = (uint32_t) &(USART1->DR);
    // Set DMA data transfer length (# of samples).
    DMA2_STREAM7->NDTR  = (uint16_t) RESPONSE_SIZE;

    // Clear USART TC flag
    USART->SR &= ~(USART_SR_TC);

    // Enable DMA stream.
    DMA2_STREAM5->CR.EN = 1;
    DMA2_STREAM7->CR.EN = 1;

    // TIM4 configuration.
    // Set prescaler and autoreload to issue DMA request at 1 Hz
    TIM4->PSC  =  0x0000;
    TIM4->ARR  =  SystemCoreClock;
    
    // Enable trigger output on timer update events.
    TIM4->CR2 &= ~(0b111 << 4);
    TIM4->CR2 |=  (0x2 << 4);
    TIM4->CR2 |= (1 << 3); // Set DMA request when update event occurs
    
    // Configure interrupt enable on update event
    TIM4->DIER |= 1;

    // Start the timer.
    TIM6->CR1 |=  1;
}