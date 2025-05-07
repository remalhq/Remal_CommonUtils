/**
 * @file      OTA_Example.ino
 * 
 * @author    Khalid Mansoor AlAwadhi, Remal <khalid@remal.io>
 * 
 * @date      May 7, 2025
 * 
 * @brief     Demonstrates how to enable OTA (Over-The-Air) firmware updates using the Remal platform,
 *            while logging system status and messages over USB.
 * 
 * @note      You MUST select a partition scheme that supports OTA!
 *            In Arduino IDE: Tools → Partition Scheme → Keep the default or choose anything with OTA support.
 *            Without this, OTA updates will not work and uploads will fail.
 */

#include "Remal_CommonUtils.h"
#include <WiFi.h>   // Native Wi-Fi support
 
// Wi-Fi credentials (replace with your own!)
const char* SSID     = "Your_SSID";         // Your Wi-Fi SSID (network name)
const char* Password = "Your_PASSWORD";     // Your Wi-Fi password
 
// Optional OTA parameters
const char* OTA_Hostname = "RemalBoard";    // NULL = default hostname
const char* OTA_Password = "1234";          // NULL = no password
 
int MaxWifiTimeout_ms = 20000;              // MAx time to wait to establish Wi-Fi connection



void setup()
{
  /* Initialize logger */
  GenericUART_Struct USBLogger =
  {
    .RX_Pin = 0,
    .TX_Pin = 0,
    .BaudRate = 115200
  };
 
  if (RML_COMM_LoggerInit(&USBLogger) != 0)
  {
    while (1);
  }
 
  RML_COMM_LogMsg("setup", e_INFO, "Booting OTA example...");
 
  /* Connect to Wi-Fi */
  WiFi.begin(SSID, Password);
  RML_COMM_LogMsg("Wi-Fi", e_INFO, "Connecting to Wi-Fi: %s", SSID);

  /* Try connecting */
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < MaxWifiTimeout_ms)
  {
    delay(1000);
    RML_COMM_LogMsg("Wi-Fi", e_DEBUG, "Attempting to connect...");
  }
 
  if (WiFi.status() != WL_CONNECTED)
  {
    RML_COMM_LogMsg("Wi-Fi", e_ERROR, "Failed to connect to Wi-Fi, double-check your credentials!");
    while (1);  // Stop here 
  }
 
  RML_COMM_LogMsg("Wi-Fi", e_INFO, "Connected! IP Address: %s", WiFi.localIP().toString().c_str());
 
  /* Setup OTA */
  RML_SetupArduinoOTA(OTA_Hostname, OTA_Password);
  RML_COMM_LogMsg("OTA", e_INFO, "OTA service initialized. Ready for updates! ^_^");
}
 
void loop()
{
  // Handle OTA updates (non-blocking)
  RML_HandleArduinoOTA();
  
  RML_COMM_LogMsg("loop", e_DEBUG, "System running. Free heap: %d bytes", ESP.getFreeHeap());
  RML_COMM_LogMsg("loop", e_INFO, "You can upload a new firmware now via OTA tools (e.g. Arduino IDE)");
  RML_COMM_LogMsg("loop", e_INFO, "Note that your new code will replace this example!");

  delay(2000);
}