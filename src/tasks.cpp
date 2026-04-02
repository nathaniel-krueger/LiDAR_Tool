#include "tasks.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ST7789.h"
#include "SPI.h"

volatile Scanner_Data currentLidar; //Current distance of what the LiDAR is seeing
volatile bool captureRequested = false;
volatile uint16_t savedDistance = 0;
volatile bool readyToPrint = false;
volatile int SavedData; // When user presses button, this variable will hold saved data to be moved to SD card
// LCD Setup
#define TFT_CS 9 // Chip Select
#define TFT_DC 4 // Data/Command

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, -1);

void LidarTask(void *pvParameters)
{
    uint8_t Packet[9]; //This will hold all 9 bytes from the LiDAR packet, including the checksum

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

                    if (captureRequested == true) // See if interrupt D5 requested a capture
                    {
                        // Take a snapshot of the clean data
                        savedDistance = currentLidar.distance;
                        captureRequested = false; // Clear the interrupt flag
                        readyToPrint = true; // Set flag to indicate we have new data ready to print
                    }
                    // Print to terminal    //This was for debugging
                    // Serial.print("Distance: ");
                    // Serial.print(currentLidar.distance);
                    // Serial.println(" cm");
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10)); // 10ms delay for testing
    }
}

void LEDTask(void *pvParameters) // This is to make sure the CPU doesnt get locked up
{
    for (;;)
    {
        digitalWrite(LED_BUILTIN, HIGH); // Red LED
        vTaskDelay(pdMS_TO_TICKS(500));
        digitalWrite(LED_BUILTIN, LOW);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}


//Heads up with this task -> colors are really screwed up and are not what they seem.
void PrintTask(void *pvParameters)
{
    tft.init(240, 320); // Initialize the ST7789 chip
    Serial.println(">>> PrintTask: Screen Init SUCCESS!");
    tft.setRotation(1);           // 1 for Landscape, 2 for Portrait
    tft.fillScreen(ST77XX_BLACK); // Clear the screen

    // Initial static text
    tft.setTextSize(3);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(10, 20);
    tft.print("LiDAR Dashboard   V1.0"); // Title message at top of screen

    uint16_t lastDistance = 0xFFFF; // Bogus number to force the first draw

    for (;;)
    {
        
        if (currentLidar.distance != lastDistance) // Only redraw if the distance actually changed
        {
            tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK); //(Foreground, Background)
            tft.setCursor(10, 80); //WHERE THE TEXT STARTS ON THE SCREEN (X, Y) PIXEL COUNT
            tft.print("Live:  ");

            // Format string: "%-4d" means pad with spaces up to 4 characters
            // This ensures "9 cm  " perfectly overwrites "100 cm"
            char buffer[16];
            sprintf(buffer, "%-4d cm", currentLidar.distance);
            tft.print(buffer);

            lastDistance = currentLidar.distance;
        }

        
        if (savedDistance > 0) // Check interrupt flag from the button
        {
            tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
            tft.setCursor(10, 140);
            tft.print("Saved: ");

            char buffer[16];
            sprintf(buffer, "%-4d cm", savedDistance);
            tft.print(buffer);
        }

        
        vTaskDelay(pdMS_TO_TICKS(100)); //Update screen every 100ms
    }
}