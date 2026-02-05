/**
 * @file      Assert_Example.ino
 *
 * @author    Khalid Mansoor AlAwadhi, Remal <khalid@remal.io>
 *
 * @date      Feb 4 2026
 *
 * @brief     Simple demonstration of using RML_ASSERT with the Remal logger.
 *            This example shows how assertions can be used to validate critical
 *            assumptions during runtime. If an assertion fails, the program will
 *            halt and log a fatal error message that includes the file, line
 *            number, and the expression that failed, making it easier to identify
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
 *
 *            NEW in v2.0: You can now register a callback function that will be called
 *            when an assert fails, before the program halts. This allows you to perform
 *            cleanup, save crash data, or blink an LED to indicate the error.
*/
#define RML_ASSERT_ENABLE				// <---- Must be defined before including Remal_CommonUtils.h for asserts to work
#include "Remal_CommonUtils.h"


/**
 * @brief Optional callback function that gets called when an assert fails.
 *        This is called BEFORE the program enters the infinite loop.
 *        Use this to save crash data, blink an LED, or perform cleanup.
 */
void MyAssertCallback(const char* FileName, uint32_t LineNumber, const char* Expression)
{
	// Example: You could save crash info to EEPROM, blink an LED, etc.
	// For this demo, we just log an extra message
	RML_COMM_Logger_Msg("Callback", e_WARNING, "Assert callback triggered! You could save crash data here.");
}


void setup()
{
	/* Initialize the logger (By default, it uses USB CDC with all log levels enabled) */
	RML_COMM_Logger_Init();

	delay(5000);		// Give time for the user to open the serial monitor

	/* Register the assert callback (optional - remove this line if you don't need it) */
	RML_COMM_Assert_SetCallback(MyAssertCallback);

	int TestValue = 10;
	RML_COMM_Logger_Msg("Test", e_INFO, "Test value is: %d", TestValue);

	// Assert that TestValue should never exceed 5 (this will intentionally fail)
	RML_ASSERT(TestValue <= 5);

	// This message will never be logged due to the assert above
	RML_COMM_Logger_Msg("Test", e_INFO, "If you see this message, the assert is disabled. Enable by defining RML_ASSERT_ENABLE before including Remal_CommonUtils.h");
}


void loop()
{
	// Nothing to do here
}
