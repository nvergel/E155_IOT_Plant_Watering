#include "Moisture_Sensor.h"
#include "STM32F401RE.h"
#include <string.h>

uint8_t moisturePercentage(uint16_t moistureADC){
    return (dry - moistureADC)*100/(dry - wet);
}

void probe(){
    uint16_t moistureADC = ADCmeasure();
    while (moistureADC == 2048) {
        moistureADC = ADCmeasure();
    }
    moisture = moisturePercentage(moistureADC);
    if (moisture < moistureThreshold) {
        lowMoisture = 1;
    }
}

void waterPlant() {
    lowMoisture = 0;

    
    // Water plant by turning pump on
    digitalWrite(GPIOA, WATER_PUMP, 1);

    TIM3->DIER &= ~1;
    setTimer(TIM3, WATER_TIME_SECONDS);
    TIM3->DIER |= 1;

    pumpOn = 1;
}

void stopWaterPlant() {
    // End plant watering
    digitalWrite(GPIOA, WATER_PUMP, 0);

    TIM3->DIER &= ~1;
    setTimer(TIM3, PROBE_INTERVAL);
    TIM3->DIER |= 1;

    pumpOn = 0;
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