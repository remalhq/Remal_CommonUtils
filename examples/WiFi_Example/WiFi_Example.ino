/**
 * @file      WiFi_Example.ino
 *
 * @author    Khalid Mansoor AlAwadhi, Remal <khalid@remal.io>
 *
 * @date      Feb 4 2026
 *
 * @brief     Demonstrates how to use the WiFi wrapper functions from the Remal CommonUtils library.
 *            This example shows connecting to WiFi with timeout, checking connection status,
 *            and automatic reconnection when the connection drops.
 *
 * @note      Replace "Your_SSID" and "Your_Password" with your actual WiFi credentials.
*/
#include "Remal_CommonUtils.h"


// Wi-Fi credentials (replace with your own!)
const char* SSID     = "Your_SSID";         // Your Wi-Fi SSID (network name)
const char* Password = "Your_Password";     // Your Wi-Fi password

int WiFi_Timeout_ms = 15000;                // Max time to wait for WiFi connection (15 seconds)


void setup()
{
	/* Initialize the logger using USB */
	RML_COMM_Logger_Init(e_USB);

	delay(3000);  // Give time for serial monitor to connect

	RML_COMM_Logger_Msg("Setup", e_INFO, "WiFi Example Starting...");

	/*
	 * Connect to WiFi using the library wrapper
	 * This function:
	 *   - Stores credentials for later reconnection
	 *   - Logs connection status automatically
	 *   - Returns 0 on success, -1 on timeout
	 */
	if (RML_COMM_WiFi_Connect(SSID, Password, WiFi_Timeout_ms) != 0)
	{
		RML_COMM_Logger_Msg("Setup", e_ERROR, "Failed to connect to WiFi!");
		RML_COMM_Logger_Msg("Setup", e_ERROR, "Please check your SSID and password, then reset the board.");

		// Halt execution
		while (1)
		{
			delay(1000);
		}
	}

	RML_COMM_Logger_Msg("Setup", e_INFO, "WiFi connected successfully!");
	RML_COMM_Logger_Msg("Setup", e_INFO, "The loop will now monitor the connection and reconnect if needed.");
}


void loop()
{
	/*
	 * Check if WiFi is still connected
	 * If disconnected, attempt to reconnect using stored credentials
	 */
	if (!RML_COMM_WiFi_IsConnected())
	{
		RML_COMM_Logger_Msg("Loop", e_WARNING, "WiFi connection lost! Attempting to reconnect...");

		/*
		 * Reconnect using stored credentials from the initial RML_COMM_WiFi_Connect() call
		 * This uses the same timeout that was specified during initial connection
		 */
		if (RML_COMM_WiFi_Reconnect() != 0)
		{
			RML_COMM_Logger_Msg("Loop", e_ERROR, "Reconnection failed! Will retry in 5 seconds...");
			delay(5000);
			return;  // Skip rest of loop and try again
		}

		RML_COMM_Logger_Msg("Loop", e_INFO, "Reconnected successfully!");
	}

	// Your application code goes here
	// For this demo, we just log a status message every 10 seconds
	static uint32_t lastLogTime = 0;
	if (millis() - lastLogTime >= 10000)
	{
		RML_COMM_Logger_Msg("Loop", e_DEBUG, "WiFi still connected. IP: %s", WiFi.localIP().toString().c_str());
		lastLogTime = millis();
	}

	delay(100);
}
