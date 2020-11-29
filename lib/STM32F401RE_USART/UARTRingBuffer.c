/**
    UART Ring Buffer: Defines functions to interact with ring buffer
    @file UARTRingBuffer.c
    @author Josh Brake
    @version 1.0 10/7/2020
*/

#include "UARTRingBuffer.h"
#include <string.h>

ring_buffer rx_buffer = {{0}, 0, 0};

void init_ring_buffer(void){
    _rx_buffer = &rx_buffer;
}

void store_char(int8_t c, ring_buffer * buffer){
    uint32_t i = (uint32_t)(buffer->head + 1) % UART_BUFFER_SIZE;

    if(i != buffer->tail) {
        buffer->buffer[buffer->head] = c;
        buffer-> head = i;
    }
}

uint8_t read_char_buffer(void){
    if(_rx_buffer->head == _rx_buffer->tail) return -1;
    else {
        int8_t c = _rx_buffer->buffer[_rx_buffer->tail];
        _rx_buffer->tail = (uint32_t)(_rx_buffer->tail + 1) % UART_BUFFER_SIZE;
        return c;
    }
}

void write_char_buffer(uint8_t c, ring_buffer * buffer) {
    store_char(c, buffer);
}

uint8_t is_data_available(void){
    return (uint8_t)((_rx_buffer->head - _rx_buffer->tail) );
}

void flush_buffer(void){
    memset(_rx_buffer->buffer, '\0', UART_BUFFER_SIZE);
    _rx_buffer->head = 0;
    _rx_buffer->tail = 0;
}

void usart_ISR(USART_TypeDef * USART){
    if (USART->SR.RXNE && USART->CR1.RXNEIE) {
        uint8_t c = USART->DR.DR;
        store_char(c, _rx_buffer);
    }
}