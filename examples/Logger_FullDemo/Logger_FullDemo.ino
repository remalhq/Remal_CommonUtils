/**
 * @file      Logger_FullDemo.ino
 * 
 * @author    Khalid Mansoor AlAwadhi, Remal <khalid@remal.io>
 * 
 * @date      August 9, 2024
 * 
 * @brief     An in-depth example demonstrating the usage of the Remal logger with various log levels and format specifiers.
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
	/*
	 * Log various messages with different log levels
	 */
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

	/*
	 * Explain supported format specifiers for the logger
	 */
	RML_COMM_LogMsg("loop", e_DEBUG, "The above are all the log levels. Now, let's test the logger's ability to print variables, it's similar to printf()");
	delay(MessageDelay_ms);
		
	RML_COMM_LogMsg("loop", e_DEBUG, "However, note that it doesn't support all the format specifiers like printf()");
	delay(MessageDelay_ms);

	RML_COMM_LogMsg("loop", e_DEBUG, "Here are all the currently supported format specifiers:");
	delay(MessageDelay_ms);

	String str = "Hello World!";
	RML_COMM_LogMsg("loop", e_DEBUG, "String(%%s): %s", str.c_str()); // Log string using %s
	delay(MessageDelay_ms);

	char c = 'A';
	RML_COMM_LogMsg("loop", e_DEBUG, "Character(%%c): %c", c); // Log character using %c
	delay(MessageDelay_ms);

	unsigned int u = 123456;
	RML_COMM_LogMsg("loop", e_DEBUG, "Unsigned Integer(%%u): %u", u); // Log unsigned integer using %u
	delay(MessageDelay_ms);

	int i = -123456;
	RML_COMM_LogMsg("loop", e_DEBUG, "Signed Integer(%%d or %%i): %d", i); // Log signed integer using %d or %i
	delay(MessageDelay_ms);

	/*
	 * Demonstrate escaping % character in log messages
	 */
	RML_COMM_LogMsg("loop", e_DEBUG, "To print a '%%' character, you need to escape it by adding another '%%'");
	delay(MessageDelay_ms);

	float f = 101.123456;
	RML_COMM_LogMsg("loop", e_DEBUG, "Float(%%f): %f", f); // Log float using %f
	delay(MessageDelay_ms);

	RML_COMM_LogMsg("loop", e_DEBUG, "By default, the logger prints floats with 2 decimal places");
	delay(MessageDelay_ms);

	RML_COMM_LogMsg("loop", e_DEBUG, "You can change the number of decimal places by adding a number after the '%%' character");
	delay(MessageDelay_ms);

	/*
	 * Demonstrate float formatting with different decimal places
	 */
	RML_COMM_LogMsg("loop", e_DEBUG, "Float with 4 decimal places(%%.4f): %.4f", f);
	delay(MessageDelay_ms);

	RML_COMM_LogMsg("loop", e_DEBUG, "Float with 5 decimal places(%%.5f): %.5f", f);
	delay(MessageDelay_ms);

	RML_COMM_LogMsg("loop", e_DEBUG, "Float with 6 decimal places (max supported)(%%.6f): %.6f", f);
	delay(MessageDelay_ms);

	int number = 47802;
	RML_COMM_LogMsg("loop", e_DEBUG, "You can also print numbers in hexadecimal");
	delay(MessageDelay_ms);

	RML_COMM_LogMsg("loop", e_DEBUG, "Number in decimal(%%d): %d", number); // Log number in decimal
	delay(MessageDelay_ms);

	RML_COMM_LogMsg("loop", e_DEBUG, "Number in hexadecimal(%%x): 0x%x", number); // Log number in hexadecimal
	delay(MessageDelay_ms);

	/* 
	 * Demonstrate changing the log source identifier
	 */
	RML_COMM_LogMsg("loop", e_DEBUG, "You can change the log source by changing the first argument of the log message");
	delay(MessageDelay_ms);

	RML_COMM_LogMsg("main", e_DEBUG, "loop has now been changed to main"); // Change log source to "main"
	delay(MessageDelay_ms);

	RML_COMM_LogMsg("RandomFunction", e_DEBUG, "This is a debug message from a random function"); // Log from a random function
	delay(MessageDelay_ms);

	RML_COMM_LogMsg("Space", e_DEBUG, "Change the log source to anything you want, it doesn't have to be a function name. The point of it is to help you identify where the log message is coming from");
	delay(MessageDelay_ms);

	/* 
	 * Discuss enabling or disabling log messages based on log level
	 */
	RML_COMM_LogMsg("loop", e_DEBUG, "You can also enable or disable log messages based on their log level");
	delay(MessageDelay_ms);

	RML_COMM_LogMsg("loop", e_DEBUG, "For example, you can disable all debug messages and only enable info, warning, error, and fatal messages");
	delay(MessageDelay_ms);

	RML_COMM_LogMsg("loop", e_DEBUG, "This is useful when you want to reduce the amount of log messages being printed to the console");
	delay(MessageDelay_ms);

	RML_COMM_LogMsg("loop", e_DEBUG, "You can do this by calling the following function:");
	delay(MessageDelay_ms);

	/*
	 * Demonstrate disabling debug messages
	 */
	RML_COMM_LogMsg("loop", e_DEBUG, "RML_COMM_LogLevelSet(e_DEBUG, false);");    
	RML_COMM_LogLevelSet(e_DEBUG, false);
	delay(MessageDelay_ms);

	RML_COMM_LogMsg("loop", e_DEBUG, "All debug messages are now disabled you will not see this message");

	/* 
	 * Re-enable debug messages
	 */
	RML_COMM_LogMsg("loop", e_INFO, "To re-enable debug messages, call the following function:");
	delay(MessageDelay_ms);

	RML_COMM_LogMsg("loop", e_INFO, "RML_COMM_LogLevelSet(e_DEBUG, true);");
	RML_COMM_LogLevelSet(e_DEBUG, true);
	delay(MessageDelay_ms);

	/* 
	 * printf() style log messages 
	 */
	RML_COMM_printf("> You can also use RML_COMM_printf() to print messages directly to the console\n");

	/* 
	 * Final messages before the demo repeats
	 */
	RML_COMM_LogMsg("loop", e_DEBUG, "This was a brief overview of the logger. For more information, please refer to the documentation and source code");
	RML_COMM_LogMsg("loop", e_DEBUG, "Good luck on your projects and build something awesome!");
	RML_COMM_LogMsg("loop", e_DEBUG, "The demo will now repeat\n\n");
	delay(MessageDelay_ms);
}