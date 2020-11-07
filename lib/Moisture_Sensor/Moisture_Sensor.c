#include "Moisture_Sensor.h"
#include "STM32F401RE.h"
#include <string.h>

moisture_buffer buffer = {{0}, 0};

void init_moisture_sensor(){
    _moisture_buffer = &buffer;
    pinMode(GPIOA, WATER_PUMP, GPIO_INPUT);
}

uint8_t moisturePercentage(uint16_t moistureADC){
    return (moistureADC - wet)/diff;
}

void probe(){
    uint16_t moistureADC = ADCmeasure();
    uint8_t moisture = moisturePercentage(moistureADC);
    _moisture_buffer->buffer[_moisture_buffer->tail] = moisture;
    ++_moisture_buffer->tail;
    waterPlant(moisture);
}

void waterPlant(uint8_t moisture) {
    if (moisture < moistureThreshold) {
        // Flush buffer?
        TIM3->DIER &= ~1;
        
        // Water plant by turning pump on
        digitalWrite(GPIOA, WATER_PUMP, 1);

        // delay micros is actualy delay secs for tim3
        delay_micros(TIM3, WATER_TIME_SECONDS);

        // End plant watering
        digitalWrite(GPIOA, WATER_PUMP, 0);

        // Reset AAR timer
        setTimer(TIM3, PROBE_INTERVAL);

        // Flush buffer?
        TIM3->DIER |= 1;
    }
}

void setMoistureThreshold(uint8_t moisture) {
    moistureThreshold = moisture;
}

void setWaterTime(uint8_t waterTimeSeconds) {
    WATER_TIME_SECONDS = waterTimeSeconds;
}

void setProbeInterval(uint8_t probeInterval) {
    PROBE_INTERVAL = probeInterval;
    setTimer(TIM3, PROBE_INTERVAL);
}

void flush_moisture_buffer(void){
    memset(_moisture_buffer->buffer, '\0', MOISTURE_BUFFER_SIZE);
    _moisture_buffer->tail = 0;
}