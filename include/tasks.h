#ifndef TASKS_H
#define TASKS_H

#include <Arduino.h>
#include <FreeRTOS_SAMD51.h>

struct Scanner_Data
{
    uint16_t distance = 0;
    uint16_t strength = 0;
    float temp = 0.0;
};

void LEDTask(void *pvParameters);
void LidarTask(void *pvParameters);

#endif