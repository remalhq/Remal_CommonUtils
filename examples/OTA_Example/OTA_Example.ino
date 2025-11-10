/**
 * @file      OTA_Example.ino
 * 
 * @author    Khalid Mansoor AlAwadhi, Remal <khalid@remal.io>
 * 
 * @date      Nov 10 2025
 * 
 * @brief     Demonstrates how to enable OTA (Over-The-Air) firmware updates using the Remal CommonUtils library,
 *            while logging system status and messages over USB.
 * 
 * @note      You MUST select a partition scheme that supports OTA!
 *            In Arduino IDE: Tools → Partition Scheme → Keep the default or choose anything with OTA support.
 *            Without this, OTA updates will not work and uploads will fail.
*/
#include "Remal_CommonUtils.h"
#include <WiFi.h>
  
// Wi-Fi credentials (replace with your own!)
const char* SSID     = "Your_SSID";         // Your Wi-Fi SSID (network name)
const char* Password = "Your_Password";     // Your Wi-Fi password
  
// Optional OTA parameters
const char* OTA_Hostname = "RemalBoard";    // NULL = default hostname
const char* OTA_Password = "1234";          // NULL = no password
  
int MaxWifiTimeout_ms = 20000;              // Max time to wait to establish Wi-Fi connection
 
 
 
void setup()
{
  /* Initialize the logger */
	RML_COMM_LoggerInit(e_USB);
  
  RML_COMM_LogMsg("setup", e_INFO, "Booting OTA example...");
  
  /* Connect to Wi-Fi */
  WiFi.disconnect(true, true);  // Erase saved credentials (if any) and disconnect
  WiFi.begin(SSID, Password);
  RML_COMM_LogMsg("Wi-Fi", e_INFO, "Connecting to Wi-Fi: %s", SSID);
 
  /* Try connecting */
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < MaxWifiTimeout_ms)
  {
    delay(600);
    RML_COMM_LogMsg("Wi-Fi", e_DEBUG, "Attempting to connect...");
  }
  
  if (WiFi.status() != WL_CONNECTED)
  {
    RML_COMM_LogMsg("Wi-Fi", e_ERROR, "Failed to connect to Wi-Fi, double-check your SSID and password!");
    RML_COMM_LogMsg("Wi-Fi", e_ERROR, "The example will stop here, please fix the issue and reupload the code.");
    while (1);
  }
  
  RML_COMM_LogMsg("Wi-Fi", e_INFO, "Connected! IP Address: %s", WiFi.localIP().toString().c_str());
  
  /* Setup OTA */
  RML_COMM_SetupArduinoOTA(OTA_Hostname, OTA_Password);
  RML_COMM_LogMsg("OTA", e_INFO, "OTA service initialized. Ready for updates! ^_^");

  /* Log useful information */
  RML_COMM_LogMsg("OTA", e_INFO, "You can upload a new firmware now via OTA tools (e.g. Arduino IDE)");
  RML_COMM_LogMsg("OTA", e_INFO, "Note that your new code will replace this example!");
  
  RML_COMM_LogMsg("OTA", e_INFO, "Wait about 30 seconds, then open the Arduino IDE and click on 'Select Other Board and Port'");
  RML_COMM_LogMsg("OTA", e_INFO, "You should see '%s' listed with IP address %s.", OTA_Hostname, WiFi.localIP().toString().c_str());
  RML_COMM_LogMsg("OTA", e_INFO, "Select it, compile your updated code, and click 'Upload'");
  RML_COMM_LogMsg("OTA", e_INFO, "Once the upload completes, the board will automatically restart and run your new code!");
}
  
void loop()
{
  /* Handle OTA updates - This must be called in the loop! Also note that the below function must be 
   * called as frequently as possible to ensure OTA updates are handled properly. If calls are too slow,
   * the watchdog will kick in and reset the board */
  RML_COMM_HandleArduinoOTA();
   
  delay(1000);   // When using OTA updates, the loop must be fast to avoid watchdog resets
}