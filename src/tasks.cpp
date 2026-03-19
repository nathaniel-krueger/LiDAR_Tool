#include "tasks.h"

volatile Scanner_Data currentLidar;

void LidarTask(void *pvParameters)
{
    uint8_t Packet[9];

    for (;;)
    {
        // Full Packet is 9 bytes: [0x59, 0x59, Dist_L, Dist_H, Strength_L, Strength_H, Temp_L, Temp_H, Checksum]
        while (Serial1.available() >= 9)
        {

            // 0x59 0x59 are the first two bytes of the packet
            if (Serial1.read() == 0x59 && Serial1.peek() == 0x59)
            {
                Packet[0] = 0x59;
                Packet[1] = Serial1.read(); // Get the second 0x59 I peeked at

                // Read the remaining 7 bytes of the packet
                Serial1.readBytes(&Packet[2], 7);

                // Verify Checksum (Sum of bytes 0 through 7)
                uint8_t checksum = 0;
                for (int i = 0; i < 8; i++)
                {
                    checksum += Packet[i];
                }

                // Compare calculated checksum to byte 8
                if (Packet[8] == checksum)
                {
                    currentLidar.distance = Packet[2] | (Packet[3] << 8);
                    currentLidar.strength = Packet[4] | (Packet[5] << 8);

                    // Print to terminal
                    Serial.print("Distance: ");
                    Serial.print(currentLidar.distance);
                    Serial.println(" cm");
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10)); // 10ms delay for testing
    }
}

void LEDTask(void *pvParameters) //This is to make sure the CPU doesnt get locked up
{
    for (;;)
    {
        digitalWrite(LED_BUILTIN, HIGH);// Red LED (L)
        vTaskDelay(pdMS_TO_TICKS(500));
        digitalWrite(LED_BUILTIN, LOW);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}