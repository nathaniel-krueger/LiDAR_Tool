#include "Arduino.h"
#include "FreeRTOS_SAMD51.h"
#include "tasks.h"


TaskHandle_t Handle_LidarTask;
TaskHandle_t Handle_LEDTask;
TaskHandle_t Handle_PrintTask;    


//Interrupts
void Button8ISR() //ISR D5
{
    captureRequested = true; 
}
void Button7ISR() //ISR D7
{
    captureRequested = true; 
}
void Button6ISR() //ISR D6
{
    captureRequested = true; 
}


void setup()
{
    pinMode(LED_BUILTIN, OUTPUT); //setup LED pin for debugging
    pinMode(8, INPUT); //Button
    pinMode(10, OUTPUT); //LCD Backlight Pin
    digitalWrite(10, HIGH); //Turn backlight on the LCD
    attachInterrupt(digitalPinToInterrupt(8), Button8ISR, RISING);
    attachInterrupt(digitalPinToInterrupt(7), Button7ISR, RISING);
    attachInterrupt(digitalPinToInterrupt(6), Button6ISR, RISING);

    Serial.begin(115200); 
    Serial1.begin(115200); // LiDAR Baud Rate

    // Fast blinks to prove the CPU is starting
    for(int i=0; i<5; i++) {
        digitalWrite(LED_BUILTIN, HIGH); delay(50);
        digitalWrite(LED_BUILTIN, LOW);  delay(50);
    }

   
    // // If you don't open the serial monitor, the tasks will NEVER start.
    // while (!Serial) {
    //     digitalWrite(LED_BUILTIN, HIGH); delay(100);
    //     digitalWrite(LED_BUILTIN, LOW);  delay(100);
    // }

    
    xTaskCreate(LidarTask, "Lidar_Task", 512, NULL, 1, &Handle_LidarTask);
    xTaskCreate(LEDTask,   "LED_Task",   512, NULL, 1, &Handle_LEDTask);
    xTaskCreate(PrintTask,   "Print_Task",   512, NULL, 1, &Handle_PrintTask);

    Serial.println("Scheduler Starting..."); 
    vTaskStartScheduler(); 
}

void loop()
{
    // LEAVE EMPTY!!!!!!!!!!!!
}

