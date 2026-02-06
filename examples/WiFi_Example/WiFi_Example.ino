/**
 * @file      WiFi_Example.ino
 *
 * @author    Khalid Mansoor AlAwadhi, Remal <khalid@remal.io>
 *
 * @date      Feb 4 2026
 *
 * @brief     Demonstrates how to use the WiFi wrapper functions from the Remal CommonUtils library.
 *            This example shows connecting to WiFi with automatic retry and optional auto-restart
 *            on failure.
 *
 * @note      Replace "Your_SSID" and "Your_Password" with your actual WiFi credentials.
*/
#include "Remal_CommonUtils.h"


// Wi-Fi credentials (replace with your own!)
const char* SSID     = "Your_SSID";         // Your Wi-Fi SSID (network name)
const char* Password = "Your_Password";     // Your Wi-Fi password


void setup()
{
	/* Initialize the logger using USB */
	RML_COMM_Log_Init(e_USB);

	delay(3000);  // Give time for serial monitor to connect

	RML_COMM_Log_Msg("Setup", e_INFO, "WiFi Example Starting...");

	/*
	 * Connect to WiFi with retry support:
	 *
	 * RML_COMM_WiFi_Connect(SSID, Password, MaxAttempts, RetryDelayMs, RebootOnFailure)
	 *
	 * Parameters:
	 *   - SSID: Network name
	 *   - Password: Network password (NULL for open networks)
	 *   - MaxAttempts: Number of connection attempts (default: 1)
	 *   - RetryDelayMs: Delay between retries in ms (default: 5000)
	 *   - RebootOnFailure: Auto-restart if all attempts fail (default: false)
	 *
	 * Examples:
	 *   RML_COMM_WiFi_Connect("SSID", "pass");                // Single attempt, no reboot
	 *   RML_COMM_WiFi_Connect("SSID", "pass", 3);             // 3 attempts, 5s delay, no reboot
	 *   RML_COMM_WiFi_Connect("SSID", "pass", 5, 5000, true); // 5 attempts, reboot on failure
	 */
	if (RML_COMM_WiFi_Connect(SSID, Password, 3, 5000, false) != 0)
	{
		RML_COMM_Log_Msg("Setup", e_ERROR, "Failed to connect to WiFi after 3 attempts!");
		RML_COMM_Log_Msg("Setup", e_ERROR, "Please check your SSID and password, then reset the board.");

		// Halt execution
		while (1)
		{
			delay(1000);
		}
	}

	RML_COMM_Log_Msg("Setup", e_INFO, "WiFi connected successfully!");
	RML_COMM_Log_Msg("Setup", e_INFO, "The loop will now monitor the connection and reconnect if needed.");
}


void loop()
{
	/*
	 * Check if WiFi is still connected
	 * If disconnected, attempt to reconnect using stored credentials and retry settings
	 */
	if (!RML_COMM_WiFi_IsConnected())
	{
		RML_COMM_Log_Msg("Loop", e_WARNING, "WiFi connection lost!");

		/*
		 * Reconnect uses the same retry settings from the initial Connect() call:
		 *   - 3 attempts
		 *   - 5 second delay between attempts
		 *   - No auto-reboot (returns -1 on failure)
		 */
		if (RML_COMM_WiFi_Reconnect() != 0)
		{
			RML_COMM_Log_Msg("Loop", e_ERROR, "All reconnection attempts failed! Will try again in 10 seconds...");
			delay(10000);
			return;  // Skip rest of loop and try again
		}
	}

	// Your application code goes here
	// For this demo, we just log a status message every 10 seconds
	static uint32_t lastLogTime = 0;
	if (millis() - lastLogTime >= 10000)
	{
		RML_COMM_Log_Msg("Loop", e_DEBUG, "WiFi still connected. IP: %s", WiFi.localIP().toString().c_str());
		lastLogTime = millis();
	}

	delay(100);
}
