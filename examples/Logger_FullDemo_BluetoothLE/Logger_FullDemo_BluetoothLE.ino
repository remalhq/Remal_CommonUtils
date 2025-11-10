/**
 * @file      Logger_FullDemo_BluetoothLE.ino
 * 
 * @author    Khalid Mansoor AlAwadhi, Remal <khalid@remal.io>
 * 
 * @date      Nov 10 2025
 * 
 * @brief     An in-depth example demonstrating the usage of the Remal logger 
 *            using Bluetooth Low Energy (BLE) with various log levels and format specifiers.
 * 
 * @note      Bluetooth logging has some issues that we plan on addressing a future update, 
 *            use this example with caution.
*/
#include "Remal_CommonUtils.h"


int MessageDelay_ms = 2000;         // Delay between log messages in milliseconds


/* LED Configuration */
const int LED_1_PIN = 1;                    //The pin connected to LED 1 on Shabakah
const int LED_2_PIN = 3;                    //The pin connected to LED 2 on Shabakah
const int NumLEDs = 1;                      //Number of LEDs on each pin
int LED_Brightness = 50;                    //Controls LED brightness (Range is from 0 to 255)
Adafruit_NeoPixel Shabakah_LED_1(NumLEDs, LED_1_PIN, NEO_GRB + NEO_KHZ800);       //Object to control LED 1
Adafruit_NeoPixel Shabakah_LED_2(NumLEDs, LED_2_PIN, NEO_GRB + NEO_KHZ800);       //Object to control LED 2

void setup() 
{
    /* Init Shabakah LEDs */
    RML_COMM_LED_Init(Shabakah_LED_1, LED_Brightness);
    RML_COMM_LED_Init(Shabakah_LED_2, LED_Brightness);

    /* Initialize the logger using Bluetooth Low Energy */
    if( RML_COMM_LoggerInit(e_BLE, "Shabakah_Logger") != 0 )
    {
        // Logger initialization failed, blink LED RED to indicate error
        while(1)
        {
            RML_COMM_LED_SetLEDColor(Shabakah_LED_1, 255, 0, 0);   // RED
            RML_COMM_LED_SetLEDColor(Shabakah_LED_2, 255, 0, 0);   // RED
            delay(500);
            RML_COMM_LED_SetLEDColor(Shabakah_LED_1, 0, 0, 0);     // OFF
            RML_COMM_LED_SetLEDColor(Shabakah_LED_2, 0, 0, 0);     // OFF
            delay(500);
        }
    }

    /* For BLE, colored logs are disabled by default to ensure compatibility with most BLE terminal apps */
    RML_COMM_EnableColorLogs(0);

    /* Enable/disable specific log levels as needed */
    RML_COMM_LogLevelSet(e_DEBUG, 1);
    RML_COMM_LogLevelSet(e_INFO, 1);
    RML_COMM_LogLevelSet(e_WARNING, 1);
    RML_COMM_LogLevelSet(e_ERROR, 1);
    RML_COMM_LogLevelSet(e_FATAL, 1);
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

    char str[] = "Hello World!";
    RML_COMM_LogMsg("loop", e_DEBUG, "string(%%s): %s", str); // Log string using %s
    delay(MessageDelay_ms);

    char c = 'A';
    RML_COMM_LogMsg("loop", e_DEBUG, "char(%%c): %c", c); // Log character using %c
    delay(MessageDelay_ms);

    unsigned int u = 123456;
    RML_COMM_LogMsg("loop", e_DEBUG, "unsigned int(%%u): %u", u); // Log unsigned integer using %u
    delay(MessageDelay_ms);

    int i = -123456;
    RML_COMM_LogMsg("loop", e_DEBUG, "int(%%d or %%i): %d", i); // Log signed integer using %d or %i
    delay(MessageDelay_ms);

    /*
     * Demonstrate escaping % character in log messages
     */
    RML_COMM_LogMsg("loop", e_DEBUG, "To print a '%%' character, you need to escape it by adding another '%%'");
    delay(MessageDelay_ms);

    // Default float precision demonstration (2 decimal places)
    float f = 5.272016;
    RML_COMM_LogMsg("loop", e_DEBUG, "Default float precision (%%f): %f", f);
    delay(MessageDelay_ms);

    // Demonstrate specifying custom decimal precision with float
    RML_COMM_LogMsg("loop", e_DEBUG, "float with 4 decimals (%%.4f): %.4f", f);
    delay(MessageDelay_ms);

    RML_COMM_LogMsg("loop", e_DEBUG, "float with 6 decimals (%%.6f): %.6f", f);
    delay(MessageDelay_ms);

    // Use double for higher precision
    RML_COMM_LogMsg("loop", e_DEBUG, "For precision beyond 6 decimals, use a double:");
    delay(MessageDelay_ms);

    double d = 10.123456789123456;

    // Demonstrating double precision clearly
    RML_COMM_LogMsg("loop", e_DEBUG, "double with 10 decimals (%%.10f): %.10f", d);
    delay(MessageDelay_ms);

    RML_COMM_LogMsg("loop", e_DEBUG, "double with 12 decimals (recommended, %%.12f): %.12f", d);
    delay(MessageDelay_ms);

    RML_COMM_LogMsg("loop", e_DEBUG, "double with 15 decimals (max useful precision, %%.15f): %.15f", d);
    delay(MessageDelay_ms);

    // Explain limitations clearly
    RML_COMM_LogMsg("loop", e_DEBUG, "Note: Due to floating-point limitations, precision beyond ~15 decimals may be inaccurate");
    delay(MessageDelay_ms);

    RML_COMM_LogMsg("loop", e_DEBUG, "If more than 15 decimal places are requested, the function defaults to 15 decimals");
    delay(MessageDelay_ms);

    RML_COMM_LogMsg("loop", e_DEBUG, "Recommendation: Stick to max 6 decimals for float, 10-12 decimals for double for reliable results");
    delay(MessageDelay_ms);

    /*
     * Demonstrate printing numbers in hexadecimal
     */
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
    RML_COMM_LogMsg("loop", e_DEBUG, "RML_COMM_LogLevelSet(e_DEBUG, 0);");    
    RML_COMM_LogLevelSet(e_DEBUG, 0);
    delay(MessageDelay_ms);

    RML_COMM_LogMsg("loop", e_DEBUG, "All debug messages are now disabled you will not see this message");

    /* 
     * Re-enable debug messages
     */
    RML_COMM_LogMsg("loop", e_INFO, "To re-enable debug messages, call the following function:");
    delay(MessageDelay_ms);

    RML_COMM_LogMsg("loop", e_INFO, "RML_COMM_LogLevelSet(e_DEBUG, 1);");
    RML_COMM_LogLevelSet(e_DEBUG, 1);
    delay(MessageDelay_ms);

    /* 
     * printf() style log messages 
     */
    RML_COMM_printf("> You can also use RML_COMM_printf() to print messages directly to the console\r\n");

    /* 
     * Final messages before the demo repeats
     */
    RML_COMM_LogMsg("loop", e_DEBUG, "This was a brief overview of the logger. For more information, please refer to the documentation and source code");
    RML_COMM_LogMsg("loop", e_DEBUG, "Good luck on your projects and build something awesome!");
    RML_COMM_LogMsg("loop", e_DEBUG, "The demo will now repeat\n\n");
    delay(MessageDelay_ms);
}