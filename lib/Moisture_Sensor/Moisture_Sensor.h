
#ifndef MOISTURE_SENSOR_H
#define MOISTURE_SENSOR_H

#include "stdint.h"
#define MOISTURE_BUFFER_SIZE 512

//Moisture defines
// #define dry 3425
// #define wet 1900
#define dry 3550
#define wet 2138

#define WATER_PUMP GPIO_PA5

// Moisture value
uint8_t moisture;

// Configurables
uint8_t moistureThreshold;
uint16_t PROBE_INTERVAL;
uint16_t WATER_TIME_SECONDS;

// Booleans
uint8_t pumpOn;
uint8_t lowMoisture;

/** Convert ADC measure to percentage
 */
uint8_t moisturePercentage(uint16_t moistureADC);

/** Probe moisture sensor
 */
void probe();

/** Flush the buffer
    (i.e., set all its contents to 0 and reset the tail).
 */
void setMoistureThreshold(uint8_t moisture);
void setWaterTime(uint8_t waterTimeSeconds);
void setProbeInterval(uint8_t probeInterval);

#endif