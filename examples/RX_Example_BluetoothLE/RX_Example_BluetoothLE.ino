/**
 * @file      RX_Example_BluetoothLE.ino
 *
 * @author    Khalid Mansoor AlAwadhi, Remal <khalid@remal.io>
 *
 * @date      Mar 11 2026
 *
 * @brief     Demonstrates RX (input) using Bluetooth Low Energy.
 *            Any received BLE data is echoed back via logger and printf.
 *
 * @note      Connect from a BLE terminal app to the device name below.
 */
#include "Remal_CommonUtils.h"


static const char* BLE_DeviceName = "Shabakah_RX_BLE";
static bool IsBLEConnected = false;


void setup()
{
	/* Initialize logger and RX over BLE */
	if (RML_COMM_Log_Init(e_BLE, BLE_DeviceName) != 0)
	{
		while(1)
		{
			vTaskDelay(pdMS_TO_TICKS(1000));
		}
	}

	/* BLE terminals typically don't support ANSI colors */
	RML_COMM_Log_EnableColor(0);

	/* Give user time to connect before first startup logs */
	uint32_t StartMs = millis();
	while ((millis() - StartMs) < 5000)
	{
		if (RML_COMM_RX_Available() >= 0)
		{
			IsBLEConnected = true;
			break;
		}

		vTaskDelay(pdMS_TO_TICKS(50));
	}

	RML_COMM_Log_Msg("Main", e_INFO, "RX BLE example started");
	RML_COMM_Log_Msg("Main", e_INFO, "Device name: %s", BLE_DeviceName);
	RML_COMM_Log_Msg("Main", e_INFO, "Send data from your BLE terminal app");
}


void loop()
{
	int32_t Available = RML_COMM_RX_Available();

	if (Available >= 0 && !IsBLEConnected)
	{
		IsBLEConnected = true;
		RML_COMM_Log_Msg("BLE", e_INFO, "BLE client connected");
	}
	else if (Available < 0 && IsBLEConnected)
	{
		IsBLEConnected = false;
	}

	if (Available > 0)
	{
		char Buffer[128];
		int32_t BytesRead = RML_COMM_RX_Read(Buffer, sizeof(Buffer));

		if (BytesRead > 0)
		{
			RML_COMM_Log_Msg("RX", e_INFO, "Received (%d bytes): %s", (int)BytesRead, Buffer);
			RML_COMM_printf("Echo: %s\r\n", Buffer);
		}
	}

	vTaskDelay(pdMS_TO_TICKS(20));
}
