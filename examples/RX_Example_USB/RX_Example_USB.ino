/**
 * @file      RX_Example_USB.ino
 *
 * @author    Khalid Mansoor AlAwadhi, Remal <khalid@remal.io>
 *
 * @date      Mar 11 2026
 *
 * @brief     Demonstrates RX (input) using USB CDC.
 *            Any received USB data is echoed back via the logger.
 *
 * @note      Open the Serial Monitor, type a message, and press Send.
 */
#include "Remal_CommonUtils.h"


void setup()
{
	/* Initialize logger and RX over native USB CDC */
	RML_COMM_Log_Init(e_USB);

	/* Give the host time to enumerate USB CDC */
	vTaskDelay(pdMS_TO_TICKS(2000));

	RML_COMM_Log_Msg("Main", e_INFO, "RX USB example started");
	RML_COMM_Log_Msg("Main", e_INFO, "Type in Serial Monitor to test RX");
}


void loop()
{
	if (RML_COMM_RX_Available() > 0)
	{
		char Buffer[128];
		int32_t BytesRead = RML_COMM_RX_Read(Buffer, sizeof(Buffer));

		if (BytesRead > 0)
		{
			RML_COMM_Log_Msg("RX", e_INFO, "Received (%d bytes): %s", (int)BytesRead, Buffer);
		}
	}

	vTaskDelay(pdMS_TO_TICKS(10));
}
