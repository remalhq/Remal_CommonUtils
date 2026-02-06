/**
 * @file      LED_Example.ino
 *
 * @author    Khalid Mansoor AlAwadhi, Remal <khalid@remal.io>
 *
 * @date      Feb 6 2026
 *
 * @brief     Demonstrates how to use the LED wrapper functions from the Remal CommonUtils library.
 *            This example shows initializing an Adafruit NeoPixel LED and cycling through colors.
 *
 * @note      This library requires the Adafruit NeoPixel library to be installed.
 *            Install it via: Arduino IDE -> Library Manager -> Search "Adafruit NeoPixel"
 *
 *            Configure LED_PIN and LED_COUNT to match your hardware setup.
*/
#include "Remal_CommonUtils.h"
#include <Adafruit_NeoPixel.h>


// LED Configuration - adjust these for your hardware
#define LED_PIN     1      	// GPIO pin connected to the NeoPixel data line (Shabakah v4 uses GPIO 1 for LED 1)
#define LED_COUNT   1       // Number of NeoPixels in your strip


// Create NeoPixel object
Adafruit_NeoPixel LED(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);


void setup()
{
	/* Initialize the logger using USB */
	RML_COMM_Log_Init(e_USB);

	delay(3000);  // Give time for serial monitor to connect

	RML_COMM_Log_Msg("Setup", e_INFO, "LED Example Starting...");

	/*
	 * Initialize the LED with brightness (0-255)
	 * Lower values = dimmer but less power consumption
	 * 50 is a good starting point
	 */
	RML_COMM_LED_Init(LED, 50);

	RML_COMM_Log_Msg("Setup", e_INFO, "LED initialized");
}


void loop()
{
	/*
	 * Demo 1: SetColor - sets all LEDs in the strip to the same color
	 */
	RML_COMM_Log_Msg("Loop", e_INFO, "--- SetColor Demo ---");

	RML_COMM_Log_Msg("Loop", e_DEBUG, "Setting color: Red");
	RML_COMM_LED_SetColor(LED, 255, 0, 0);
	delay(1000);

	RML_COMM_Log_Msg("Loop", e_DEBUG, "Setting color: Green");
	RML_COMM_LED_SetColor(LED, 0, 255, 0);
	delay(1000);

	RML_COMM_Log_Msg("Loop", e_DEBUG, "Setting color: Blue");
	RML_COMM_LED_SetColor(LED, 0, 0, 255);
	delay(1000);

	/*
	 * Demo 2: SetBrightness - change brightness at runtime
	 */
	RML_COMM_Log_Msg("Loop", e_INFO, "--- SetBrightness Demo ---");
	RML_COMM_LED_SetColor(LED, 255, 255, 255);  // White

	RML_COMM_Log_Msg("Loop", e_DEBUG, "Brightness: 100%% (255)");
	RML_COMM_LED_SetBrightness(LED, 255);
	delay(1000);

	RML_COMM_Log_Msg("Loop", e_DEBUG, "Brightness: 50%% (128)");
	RML_COMM_LED_SetBrightness(LED, 128);
	delay(1000);

	RML_COMM_Log_Msg("Loop", e_DEBUG, "Brightness: 10%% (25)");
	RML_COMM_LED_SetBrightness(LED, 25);
	delay(1000);

	// Restore brightness
	RML_COMM_LED_SetBrightness(LED, 50);

	/*
	 * Demo 3: LED_Off - turn off all LEDs (cleaner than SetColor(0,0,0))
	 */
	RML_COMM_Log_Msg("Loop", e_INFO, "--- LED_Off Demo ---");
	RML_COMM_Log_Msg("Loop", e_DEBUG, "Turning LED off");
	RML_COMM_LED_Off(LED);
	delay(2000);
}
