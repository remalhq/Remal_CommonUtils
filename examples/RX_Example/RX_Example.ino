/**
 * @file      RX_Example.ino
 *
 * @author    Khalid Mansoor AlAwadhi, Remal <khalid@remal.io>
 *
 * @date      Mar 11 2026
 *
 * @brief     Demonstrates the RX (input) functionality of the Remal CommonUtils library.
 *            This example uses USB CDC to echo back any received data via the logger.
 *            It shows polling with RML_COMM_RX_Available() and reading with RML_COMM_RX_Read().
 *
 * @note      Open the Serial Monitor, type a message, and press Send to see it echoed back.
*/
#include "Remal_CommonUtils.h"


void setup()
{
	/* Initialize the logger using USB CDC */
	RML_COMM_Log_Init(e_USB);

	/* Wait a moment for USB connection */
	vTaskDelay(pdMS_TO_TICKS(2000));

	RML_COMM_Log_Msg("Main", e_INFO, "RX Example started - type something in the Serial Monitor!");
	RML_COMM_Log_Msg("Main", e_INFO, "Anything you send will be echoed back via the logger");
}


void loop()
{
	/* Check if data is available */
	if (RML_COMM_RX_Available() > 0)
	{
		char Buffer[128];
		int32_t BytesRead = RML_COMM_RX_Read(Buffer, sizeof(Buffer));

		if (BytesRead > 0)
		{
			RML_COMM_Log_Msg("RX", e_INFO, "Received (%d bytes): %s", BytesRead, Buffer);
		}
	}

	vTaskDelay(pdMS_TO_TICKS(10));
}
