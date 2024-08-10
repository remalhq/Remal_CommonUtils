/**
 * @file      Logger_SimpleDemo.ino
 * 
 * @author    Khalid Mansoor AlAwadhi, Remal <khalid@remal.io>
 * 
 * @date      August 9, 2024
 * 
 * @brief     A simple example demonstrating the usage of the Remal logger.
*/
#include "Remal_CommonUtils.h"


int MessageDelay_ms = 2000;         // Delay between log messages in milliseconds


void setup() 
{
	/* Create a logger object */
	GenericUART_Struct USBLogger =
	{
		.RX_Pin = 0,            // On Shabakah, we use the native USB port for logging
		.TX_Pin = 0,            // Which means pins 18 and 19 are automatically used
		.BaudRate = 115200
	};

	/* Initialize the logger, pass the logger object */
	if( RML_COMM_LoggerInit(&USBLogger) != 0 )
	{
		// If the logger fails to initialize, then we will be stuck in this loop
		while(1);
	}
}


void loop() 
{
	delay(MessageDelay_ms);
	RML_COMM_LogMsg("loop", e_DEBUG, "Hello World! This is a debug message");
	delay(MessageDelay_ms);

	RML_COMM_LogMsg("loop", e_INFO, "and this is an info message");
	delay(MessageDelay_ms);

	RML_COMM_LogMsg("loop", e_WARNING, "and this is a warning message");
	delay(MessageDelay_ms);

	RML_COMM_LogMsg("loop", e_ERROR, "and this is an error message");
	delay(MessageDelay_ms);

	RML_COMM_LogMsg("loop", e_FATAL, "and this is a fatal message");
	delay(MessageDelay_ms);

	// Use the logger to print variables
	int Number = 42;
	RML_COMM_LogMsg("loop", e_DEBUG, "You can also print variables, like this: %d", Number);
	delay(MessageDelay_ms);

	RML_COMM_LogMsg("loop", e_DEBUG, "It's very similar to printf(), however, it has different features");
	delay(MessageDelay_ms);

	RML_COMM_LogMsg("loop", e_DEBUG, "Check the Logger_FullDemo example to see all the features of the logger");
	delay(MessageDelay_ms);

	RML_COMM_LogMsg("loop", e_DEBUG, "The logger is very useful for debugging and monitoring your code");
	delay(MessageDelay_ms);

	RML_COMM_LogMsg("loop", e_DEBUG, "It's also very useful for logging data and events");
	delay(MessageDelay_ms);

	RML_COMM_LogMsg("loop", e_DEBUG, "We designed the logger to be easy and powerful to use");
	delay(MessageDelay_ms);

	RML_COMM_LogMsg("loop", e_DEBUG, "Good luck with your projects! The demo will now repeat\n\n");
	delay(MessageDelay_ms);
}