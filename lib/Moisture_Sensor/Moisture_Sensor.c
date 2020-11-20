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
    //waterPlant(moisture);
}

void waterPlant(uint8_t moisture) {
    if (moisture < moistureThreshold) {
        // Flush buffer?
        TIM3->DIER &= ~1;
        
        // Water plant by turning pump on
        digitalWrite(GPIOA, WATER_PUMP, 1);

        // delay micros is actualy delay secs for tim3
        delay_millis(TIM3, WATER_TIME_SECONDS);

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