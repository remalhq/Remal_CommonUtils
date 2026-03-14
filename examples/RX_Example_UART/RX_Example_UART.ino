/**
 * @file      RX_Example_UART.ino
 *
 * @author    Khalid Mansoor AlAwadhi, Remal <khalid@remal.io>
 *
 * @date      Mar 11 2026
 *
 * @brief     Demonstrates RX (input) using UART.
 *            Any received UART data is echoed back via the logger.
 *
 * @note      Wire external UART as:
 *            - External TX -> Shabakah RX pin (RX_Pin below)
 *            - External RX -> Shabakah TX pin (TX_Pin below)
 *            - Common GND
 */
#include "Remal_CommonUtils.h"


/* UART Configuration */
const int TX_Pin = 6;                    // Shabakah UART TX pin
const int RX_Pin = 7;                    // Shabakah UART RX pin (set to -1 to disable RX)
const int Baudrate = 115200;
const uart_port_t UART_Num = UART_NUM_0;


void setup()
{
	/* Initialize logger and RX over UART */
	if (RML_COMM_Log_Init(e_UART, TX_Pin, RX_Pin, Baudrate, UART_Num) != 0)
	{
		while(1)
		{
			delay(1000);
		}
	}

	RML_COMM_Log_Msg("Main", e_INFO, "RX UART example started");
	RML_COMM_Log_Msg("Main", e_INFO, "Send UART data at %d baud", Baudrate);
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

	delay(10);
}
