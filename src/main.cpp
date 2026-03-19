#include <Arduino.h>
#include <FreeRTOS_SAMD51.h>
#include "tasks.h"


TaskHandle_t Handle_LidarTask;
TaskHandle_t Handle_LEDTask;

// Function declarations

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200); 
    Serial1.begin(115200); //This is LiDAR Baud Rate

    xTaskCreate(
        LidarTask, "Lidar_Task", 2048, NULL, 1, &Handle_LidarTask);

    xTaskCreate(
        LEDTask, "LED_Task", 2048, NULL, 1, &Handle_LEDTask);
    vTaskStartScheduler(); //Starting RTOS scheduler
}

void loop()
{
    // LEAVE EMPTY!!!!!!!!!!!!
}


