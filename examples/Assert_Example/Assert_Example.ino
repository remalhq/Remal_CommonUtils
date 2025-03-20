/**
 * @file      Assert_Example.ino
 * 
 * @author    Khalid Mansoor AlAwadhi, Remal <khalid@remal.io>
 * 
 * @date      21 March 2025
 * 
 * @brief     Simple demonstration of using RML_ASSERT with the Remal logger.
 *            This example shows how assertions can be used to validate critical 
 *            assumptions during runtime. If an assertion fails, the program will 
 *            halt and log a fatal error message that includes the file and line
 *            number where the assertion failed, making it easier to identify 
 *            and debug issues during development.
 * 
 *            Assertions are particularly useful for:
 *              - Validating function inputs or outputs.
 *              - Ensuring invariants or conditions that must always be true.
 *              - Catching unexpected states or errors early in development.
 * 
 *            NOTE: #define RML_ASSERT_ENABLE must be added to the top of the file 
 *            for asserts to work, more specifically before including Remal_CommonUtils.h.
 *            If RML_ASSERT_ENABLE is not defined, all assertions will be compiled out, 
 *            allowing the program to continue execution even if conditions are not met.
 *            This also reduces the size of the compiled binary, saving valuable space 
 *            in memory-constrained environments like microcontrollers. Maybe do that 
 *            only when you are sure your code is bug-free and ready for release.
*/
#define RML_ASSERT_ENABLE			// <---- Must be defined before including Remal_CommonUtils.h for asserts to work
#include "Remal_CommonUtils.h"



void setup() 
{
	/* Create a logger object */
	GenericUART_Struct USBLogger =
	{
		.RX_Pin = 0,            // Native USB port
		.TX_Pin = 0,
		.BaudRate = 115200
	};

	/* Initialize the logger, pass the logger object */
	if( RML_COMM_LoggerInit(&USBLogger) != 0 )
	{
		// Halt if the logger initialization fails
		while(1);
	}

	delay(5000);		// Give time for the user to open the serial monitor

	int TestValue = 10;
	RML_COMM_LogMsg("Test", e_INFO, "Test value is: %d", TestValue);

	// Assert that TestValue should never exceed 5 (this will intentionally fail)
	RML_ASSERT(TestValue <= 5);

	// This message will never be logged due to the assert above
	RML_COMM_LogMsg("Test", e_INFO, "If you see this message, the assert is disabled. Enable by defining RML_ASSERT_ENABLE before including Remal_CommonUtils.h");
}


void loop() 
{
	// Nothing to do here
}