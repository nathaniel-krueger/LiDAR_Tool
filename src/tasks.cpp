#include "tasks.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ST7789.h"
#include "SPI.h"

volatile Scanner_Data currentLidar; // Current distance of what the LiDAR is seeing
volatile bool captureRequested = false;
volatile uint16_t savedDistance = 0;
volatile bool readyToPrint = false;
volatile int SavedData;               // When user presses button, this variable will hold saved data to be moved to SD card
volatile int16_t gyroX, gyroY, gyroZ; // Gyro readings for X, Y, Z axes
// LCD Setup
#define TFT_CS 9 // Chip Select
#define TFT_DC 4 // Data/Command

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, -1);

void LidarTask(void *pvParameters)
{
    uint8_t Packet[9]; // This will hold all 9 bytes from the LiDAR packet, including the checksum

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
                        readyToPrint = true;      // Set flag to indicate we have new data ready to print
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

// Heads up with this task -> colors are really screwed up and are not what they seem.
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
    tft.print("LiDAR Dashboard   V1.1"); // Title message at top of screen

    uint16_t lastDistance = 0xFFFF; // Bogus number to force the first draw

    for (;;)
    {

        if (currentLidar.distance != lastDistance) // Only redraw if the distance actually changed
        {
            tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK); //(Foreground, Background)
            tft.setCursor(10, 80);                        // WHERE THE TEXT STARTS ON THE SCREEN (X, Y) PIXEL COUNT
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
        if (abs(gyroX) < 100 && abs(gyroY) < 100 && abs(gyroZ) < 100) //If youre within this youre level
        {
            tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
            tft.setCursor(10, 200);
            tft.print("Stationary    ");
        }
        else
        {
            tft.setTextColor(ST77XX_CYAN, ST77XX_BLACK);
            tft.setCursor(10, 200);
            tft.print("Not Stationary");
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Update screen every 100ms
    }
}
void GyroTask(void *pvParameters)
{
    uint8_t xMSB;
    uint8_t xLSB;
    uint8_t yMSB;
    uint8_t yLSB;
    uint8_t zMSB;
    uint8_t zLSB;
    // Wake up Gyro
    Wire.beginTransmission(0x6B); // I2C Address
    Wire.write(0x20);             // Target the CTRL1 Register
    Wire.write(0x0F);             // 0x0F = Normal mode
    Wire.endTransmission();

    for (;;)
    {
        Wire.beginTransmission(0x6B);
        Wire.write(0x28 | 0x80);
        Wire.endTransmission(false);
        Wire.requestFrom(0x6B, 6);
        if (Wire.available() == 6)
        {
            xLSB = Wire.read();
            xMSB = Wire.read();
            gyroX = xMSB << 8 | xLSB; // Combine MSB and LSB for X-axis
            yLSB = Wire.read();
            yMSB = Wire.read();
            gyroY = yMSB << 8 | yLSB; // Combine MSB and LSB for Y-axis
            zLSB = Wire.read();
            zMSB = Wire.read();
            gyroZ = zMSB << 8 | zLSB; // Combine MSB and LSB for Z-axis
        }

        // For debugging, print gyro values to Serial
        Serial.print("Gyro X: ");
        Serial.print(gyroX);
        Serial.print(" Y: ");
        Serial.print(gyroY);
        Serial.print(" Z: ");
        Serial.println(gyroZ);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}