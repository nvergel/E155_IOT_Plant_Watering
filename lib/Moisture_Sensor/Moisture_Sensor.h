
#ifndef MOISTURE_SENSOR_H
#define MOISTURE_SENSOR_H

#include "stdint.h"
#define MOISTURE_BUFFER_SIZE 512

//Moisture defines
#define dry 3425
#define wet 1900
#define diff dry - wet

#define WATER_PUMP GPIO_PA5

/** Structure for ring_buffer.
    Contains a uint8_t buffer of length UART_BUFFER_SIZE to hold a string as
    well as two integers to hold the value of the head and tail of the buffer
    to indicate what characters have or have not been read out.
 */
typedef struct {
    uint8_t buffer[MOISTURE_BUFFER_SIZE];
    volatile uint16_t tail;
} moisture_buffer;

moisture_buffer * _moisture_buffer;
uint8_t moistureThreshold;
uint16_t PROBE_INTERVAL;
uint16_t WATER_TIME_SECONDS;

/** Initialize the moisture buffer and GPIO
 */
void init_moisture_sensor();

/** Convert ADC measure to percentage
 */
uint8_t moisturePercentage(uint16_t moistureADC);

/** Probe moisture sensor
 */
void probe();

/** Flush the buffer
    (i.e., set all its contents to 0 and reset the tail).
 */
void flush_moisture_buffer(void);

void setMoistureThreshold(uint8_t moisture);
void setWaterTime(uint8_t waterTimeSeconds);
void setProbeInterval(uint8_t probeInterval);

#endif