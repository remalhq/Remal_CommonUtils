/**
 * @file      OTA_Example.ino
 *
 * @author    Khalid Mansoor AlAwadhi, Remal <khalid@remal.io>
 *
 * @date      Feb 4 2026
 *
 * @brief     Demonstrates how to enable OTA (Over-The-Air) firmware updates using the Remal CommonUtils library,
 *            while logging system status and messages over USB.
 *
 *            By default, OTA runs in a background FreeRTOS task, so you don't need to call OTA_Handle() in your loop.
 *            If you prefer manual control, pass 'false' as the third parameter to RML_COMM_OTA_Setup().
 *
 * @note      You MUST select a partition scheme that supports OTA!
 *            In Arduino IDE: Tools -> Partition Scheme -> Keep the default or choose anything with OTA support.
 *            Without this, OTA updates will not work and uploads will fail.
*/
#include "Remal_CommonUtils.h"

// Wi-Fi credentials (replace with your own!)
const char* SSID     = "Your_SSID";         // Your Wi-Fi SSID (network name)
const char* Password = "Your_Password";     // Your Wi-Fi password

// Optional OTA parameters
const char* OTA_Hostname = "RemalBoard";    // NULL = default hostname
const char* OTA_Password = "1234";          // NULL = no password



void setup()
{
	/* Initialize the logger */
	RML_COMM_Log_Init(e_USB);

	RML_COMM_Log_Msg("Setup", e_INFO, "Booting OTA example...");

	/*
	 * Connect to Wi-Fi with retry support:
	 * - 3 connection attempts
	 * - 5 second delay between attempts
	 * - Auto-reboot if all attempts fail (OTA needs WiFi!)
	 */
	RML_COMM_WiFi_Connect(SSID, Password, 3, 5000, true);
	// If we reach here, WiFi is connected (otherwise board would have rebooted)

	/*
	 * Setup OTA - by default runs in background task (no need to call OTA_Handle in loop)
	 * If you want manual control, use: RML_COMM_OTA_Setup(OTA_Hostname, OTA_Password, false);
	 */
	RML_COMM_OTA_Setup(OTA_Hostname, OTA_Password);

	/* Log useful information */
	RML_COMM_Log_Msg("OTA", e_INFO, "You can upload a new firmware now via OTA tools (e.g. Arduino IDE)");
	RML_COMM_Log_Msg("OTA", e_INFO, "Note that your new code will replace this example!");

	RML_COMM_Log_Msg("OTA", e_INFO, "Wait about 30 seconds, then open the Arduino IDE and click on 'Select Other Board and Port'");
	RML_COMM_Log_Msg("OTA", e_INFO, "You should see '%s' listed with IP address %s.", OTA_Hostname, WiFi.localIP().toString().c_str());
	RML_COMM_Log_Msg("OTA", e_INFO, "Select it, compile your updated code, and click 'Upload'");
	RML_COMM_Log_Msg("OTA", e_INFO, "Once the upload completes, the board will automatically restart and run your new code!");
}

void loop()
{
	/*
	 * Check if WiFi is still connected, reconnect if needed.
	 * Reconnect() uses the same retry settings from Connect():
	 * - 3 attempts, 5s delay, auto-reboot on failure
	 */
	if (!RML_COMM_WiFi_IsConnected())
	{
		RML_COMM_Log_Msg("WiFi", e_WARNING, "WiFi disconnected, reconnecting...");
		RML_COMM_WiFi_Reconnect();  // Will reboot if all attempts fail
	}

	/*
	 * OTA is handled automatically in the background task.
	 * If you used RML_COMM_OTA_Setup(..., false), you would call RML_COMM_OTA_Handle() here.
	 */

	delay(1000);
}
