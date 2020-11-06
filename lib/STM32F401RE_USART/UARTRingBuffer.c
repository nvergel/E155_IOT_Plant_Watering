/**
    UART Ring Buffer: Defines functions to interact with ring buffer
    @file UARTRingBuffer.c
    @author Josh Brake
    @version 1.0 10/7/2020
*/

#include "UARTRingBuffer.h"
#include <string.h>

ring_buffer rx_buffer = {{0}, 0, 0};

ring_buffer tx1_buffer = {{0}, 0, 0};

ring_buffer tx2_buffer = {{0}, 0, 0};

void init_ring_buffer(void){
    _rx_buffer = &rx_buffer;
    _tx1_buffer = &tx1_buffer;
    _tx2_buffer = &tx2_buffer;
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
    memset(_tx1_buffer->buffer, '\0', UART_BUFFER_SIZE);
    _tx1_buffer->head = 0;
    _tx1_buffer->tail = 0;
    memset(_tx2_buffer->buffer, '\0', UART_BUFFER_SIZE);
    _tx2_buffer->head = 0;
    _tx2_buffer->tail = 0;
}

void sendCharBuffer(USART_TypeDef * USART, ring_buffer * buffer) {
    if (buffer->tail != buffer->head) {
        int8_t c = buffer->buffer[buffer->tail];
        buffer->tail = (uint32_t)(buffer->tail + 1) % UART_BUFFER_SIZE;
        USART->DR.DR = c;
    } else {
        USART->CR1.TXEIE = 0;
    }
}

uint8_t transmitBufferEmpty() {
    return _tx1_buffer->tail == _tx1_buffer->head;
}

void usart_ISR(USART_TypeDef * USART){
    if (USART->SR.RXNE && USART->CR1.RXNEIE) {
        uint8_t c = USART->DR.DR;
        store_char(c, _rx_buffer);
    }
    if (USART->SR.TXE && USART->CR1.TXEIE) {
        sendCharBuffer(USART, _tx1_buffer);
    }
}

void usart_ISR2(USART_TypeDef * USART){
    if (USART->SR.TXE && USART->CR1.TXEIE) {
        sendCharBuffer(USART, _tx2_buffer);
    }
}

uint8_t look_for_substring (uint8_t *str, uint8_t *buffertolookinto){
    uint32_t stringlength = strlen(str);
	volatile int bufferlength = strlen(buffertolookinto);
	uint32_t so_far = 0;
	uint32_t indx = 0;
repeat:
	while (str[so_far] != buffertolookinto[indx]) {
        if(indx >= bufferlength) return 0;
        indx++;
    }

	if (str[so_far] == buffertolookinto[indx])
	{
		while (str[so_far] == buffertolookinto[indx])
		{
			so_far++;
			indx++;
		}
	}

	else
	{
		so_far = 0;
		if (indx >= bufferlength) return 0;
		goto repeat;
	}

	if (so_far == stringlength) return 1;
	else return 0;
}
