/**
    UART Ring Buffer: Initializes and provides functions to interact with ring buffer
    @file UARTRingBuffer.h
    @author Josh Brake
    @version 1.0 10/7/2020
*/

#ifndef UARTRINGBUFFER_H_
#define UARTRINGBUFFER_H_

#include "stdint.h"
#include "STM32F401RE_USART.h"
#define UART_BUFFER_SIZE 1024

/** Structure for ring_buffer.
    Contains a uint8_t buffer of length UART_BUFFER_SIZE to hold a string as
    well as two integers to hold the value of the head and tail of the buffer
    to indicate what characters have or have not been read out.
 */
typedef struct {
    uint8_t buffer[UART_BUFFER_SIZE];
    volatile uint32_t head;
    volatile uint32_t tail;
} ring_buffer;

ring_buffer * _rx_buffer;

/** TODO
    Initialize the ring buffer.
    Currently this function only initializes a receive buffer. You should add
    the functionality for a transmit buffer and initialize it in this function.
 */
void init_ring_buffer(void);

/** Read a character from the buffer
    @return character if it there is a character to read; otherwise return -1.
 */
uint8_t read_char_buffer(void);

/** Check if there is data that has not yet been read out of the buffer.
    i.e., check if head is equal to tail.
    @return 1 if head != tail or 0 if head == tail
 */
uint8_t is_data_available(void);

/** Flush the buffer
    (i.e., set all its contents to 0 and reset the head and tail).
 */
void flush_buffer(void);

/** Define our interrupt service routine here. Make sure to put this function
    call into the correct IRQHandler function to map it to the vector table.
 */
void usart_ISR(USART_TypeDef * USART);

#endif