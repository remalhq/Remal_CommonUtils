/**
 * @file 		Remal_CommonUtils.h
 * @author 		Khalid Mansoor AlAwadhi, Remal <khalid@remal.io>
 * @date 		Oct 29 2025 (Initial release - 14 May 2020)
 * @version		1.4
 * 
 * @brief   	This library provides various tools and utilities to be used 
 * 				on Remal hardware.
 *
 * @note		Refer to the README.md file for more information about this library.
**/
#ifndef _REMAL_COMMONUTILS_H_
#define _REMAL_COMMONUTILS_H_


//<!-- Standard includes -->
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

//<!-- ESP32 includes -->
#include <Arduino.h>
#include "driver/uart.h"

//<!-- Remal includes -->
#include "Remal_BLE_Serial.h"

//<!-- Arduino includes -->
#include <ArduinoOTA.h>
#include <Adafruit_NeoPixel.h>


/*********************************************
 * Defines
 *********************************************/
/**
 * @brief This define is used to halt the program when an assert fails, it gets the
 * filename and line number and calls RML_COMM_Assert() to loop forever.
 * Symbol '-D' RML_ASSERT_ENABLE must be added or assert calls to work, else they 
 * will be compiled out
 */
#ifdef RML_ASSERT_ENABLE
#pragma message("RML_ASSERT() calls are enabled")
#define BASENAME(_file) ((strrchr(_file, '/') ? : (strrchr(_file, '\\') ? : _file)) + 1)		//This is used to only get the filename from the full path
#define RML_ASSERT(expr)										\
		{														\
			if (!(expr))										\
			{													\
				const char *file = BASENAME(__FILE__);			\
				_RML_COMM_Assert(file, __LINE__);				\
			}													\
		}
#else
#pragma message("RML_ASSERT() calls are disabled")
#define RML_ASSERT(expr)		((void)0)
#endif

/**
 * @brief Defines to add colored logs on support 
 * terminals that support ANSI color codes.
 */
#define ANSI_RESET     		"\x1B[0m"
#define ANSI_BLACK     		"\x1B[30m"
#define ANSI_RED       		"\x1B[31m"
#define ANSI_GREEN     		"\x1B[32m"
#define ANSI_YELLOW    		"\x1B[33m"
#define ANSI_BLUE      		"\x1B[34m"
#define ANSI_MAGENTA   		"\x1B[35m"
#define ANSI_CYAN      		"\x1B[36m"
#define ANSI_WHITE     		"\x1B[37m"
#define ANSI_BOLDBLACK     	"\x1B[90m"
#define ANSI_BOLDRED       	"\x1B[91m"
#define ANSI_BOLDGREEN     	"\x1B[92m"
#define ANSI_BOLDYELLOW    	"\x1B[93m"
#define ANSI_BOLDBLUE      	"\x1B[94m"
#define ANSI_BOLDMAGENTA   	"\x1B[95m"
#define ANSI_BOLDCYAN      	"\x1B[96m"
#define ANSI_BOLDWHITE     	"\x1B[97m"
#define ANSI_WHITEONREDBG  	"\x1B[41;97m"
#define ANSI_REDONWHITEBG  	"\x1B[47;91m"


/*********************************************
 * Enums
 *********************************************/
/**
 * @brief Log Level enum:
 * Used distinguish different log levels
 */
typedef enum
{
	e_DEBUG = 0,
	e_INFO = 1,
	e_WARNING = 2,
	e_ERROR = 3,
	e_FATAL = 4
} LogLevel_Enum;

/**
 * @brief Log Protocol enum:
 * Used to select which protocol to use for logging
 */
typedef enum
{
	e_USB = 0,
	e_UART = 1,
	e_BLE = 2,
} LogProtocol_Enum;






/**********************************************************************************************************************************
 * 												<!-- Logging Functions -->
 **********************************************************************************************************************************/
 /************************************************************************************************************************
 * @brief	Initializes the logger, defaults to using native USB (USB CDC). 
 * 
 * 
 * @note	When using PlatformIO, the following must be added to the platformio.ini file:
 * 				build_flags = 
 * 					-DARDUINO_USB_MODE=1
 * 					-DARDUINO_USB_CDC_ON_BOOT=1
 * 			This allows the ESP32 to use the USB port as a serial port, which this library uses for logging
 * 
 * @param[in] LoggingProtocol
 * 			Use the LogProtocol_Enum to select which protocol to use for logging:
 * 				- e_USB
 * 				- e_UART
 * 				- e_BLE
 *
 * @return
 * 			0 on success, -1 on failure
 ************************************************************************************************************************/
int8_t RML_COMM_LoggerInit(uint8_t LoggingProtocol = e_USB);


 /************************************************************************************************************************
 * @brief	Overload - Initializes the logger using UART. UART is init with the following settings: 
 * 				- 8 data bits
 * 				- No parity
 * 				- 1 stop bit
 * 				- No flow control
 * 
 * 
 * @param[in] LoggingProtocol
 * 			Use the LogProtocol_Enum to select which protocol to use for logging:
 * 				- e_USB
 * 				- e_UART
 * 				- e_BLE
 * 
 * @param[in] TX_Pin
 * 			The TX pin to use for UART logging
 * 
 * @param[in] Baudrate
 * 			The baudrate to use for UART logging
 * 
 * @param[in] UART_Num
 * 			The UART instance to use. Use ESP32 defines, On Shabakah (ES32C3) options are:
 * 				- UART_NUM_0 [Default]
 * 				- UART_NUM_1
 *
 * @return
 * 			0 on success, -1 on failure
 ************************************************************************************************************************/
int8_t RML_COMM_LoggerInit(uint8_t LoggingProtocol, uint8_t TX_Pin, uint32_t Baudrate, uart_port_t UART_Num = UART_NUM_0);



 /************************************************************************************************************************
 * @brief	Overload - Initializes the logger using BLE (Bluetooth Low Energy).
 * 
 * 
 * @param[in] LoggingProtocol
 * 			Use the LogProtocol_Enum to select which protocol to use for logging:
 * 				- e_USB
 * 				- e_UART
 * 				- e_BLE
 * 
 * @param[in] BT_Name
 * 			The name of the BLE device that will be advertised for logging
 *
 * @return
 * 			0 on success, -1 on failure
 ************************************************************************************************************************/
int8_t RML_COMM_LoggerInit(uint8_t LoggingProtocol, char* BT_Name);



/************************************************************************************************************************
 * @brief	Logs a message (UART or native). Supports specifiers and additional arguments if needed, <b> check out 
 * 			RML_COMM_printf() for more information. </b>
 * 
 * 			Example usage:
 * 				- No additional args: RML_COMM_LogMsg("Main", e_INFO, "This is a test log message");
 * 				- With args: RML_COMM_LogMsg("Main", e_INFO, "Loop number - %u. Text to log %s", UnsignedNum, TempStr); <-- Similar to printf()!
 *
 *
 * @param[in] Src
 * 			Source of the log (ex: function name)
 *
 * @param[in] LogLvl
 * 			Log level of the message, use the enums defined in this file:
 * 			- e_DEBUG
 * 			- e_INFO
 * 			- e_WARNING
 * 			- e_ERROR
 * 			- e_FATAL
 *
 * @param[in] Msg
 * 			Message to output in the log
 *
 * @param[in] ...
 * 			Any additional arguments
 *
 * @return
 *          None
 ************************************************************************************************************************/
void RML_COMM_LogMsg(const char *Src, uint8_t LogLvl, const char* Msg, ... );



/************************************************************************************************************************
 * @brief	Enables or disables a certain log level. By default, all log levels are enabled.
 *
 *
 * @param[in] LogLvl
 * 			Log level to enable or disable, use the enums defined in this file:
 * 				- e_DEBUG
 * 				- e_INFO
 * 				- e_WARNING
 * 				- e_ERROR
 * 				- e_FATAL
 * 
 * @param[in] Enable
 * 			Enable or disable the log level. 1 to enable, 0 to disable
 *
 * @return
 * 			0 on success, -1 if invalid log level
 ************************************************************************************************************************/
int8_t RML_COMM_LogLevelSet(uint8_t LogLvl, uint8_t Enable);



/************************************************************************************************************************
 * @brief	Enables or disables colored logs. By default, colored logs are disabled.
 *
 * 
 * @param[in] Enable
 * 			Enable or disable colored logs. 1 to enable, 0 to disable
 *
 * @return
 * 			None
 ************************************************************************************************************************/
void RML_COMM_EnableColorLogs(uint8_t Enable);



/**********************************************************************************************************************************
 * 											<!-- FreeRTOS Helper Functions -->
 **********************************************************************************************************************************/
/************************************************************************************************************************
 * @brief	Logs the stack usage of the current task. Great for debugging purposes.
 *
 *
 * @param[in] TaskStackSize
 * 			The given stack size when the task was created e.g. xTaskCreate() or xTaskCreatePinnedToCore()
 * 
 * @return
 * 			0 on success, -1 if invalid stack size
 ************************************************************************************************************************/
int8_t RML_COMM_LogStackUsage(UBaseType_t TaskStackSize);



/**********************************************************************************************************************************
 * 												<!-- Printf Functions -->
 **********************************************************************************************************************************/
/************************************************************************************************************************
 * @brief	Extremely lightweight implementation of printf() for embedded systems. Prints output to UART. In case this 
 * 			was run on a system that isn't supported, routes output to stdout via printf(). This function calls 
 * 			RML_COMM_vprintf() to do the actual printing
 * 
 * 			Currently supports the following specifiers:
 * 				- %s => String
 * 				- %c => Character
 * 				- %u => Unsigned integer
 * 				- %d or %i => Signed integer
 * 				- %% => To print a '%'
 * 				- %X or %x => Hex value
 * 				- %f => Float/Double, default precision is 2 decimal places
 * 				- %.Xf => Float/Double, where X is the number of decimal places (up to 15 decimal places)
 * 
 * 			Why this was created? Mainly for 2 reasons:
 * 				1- printf() has a lot of code overhead and not recommend on embedded systems (that is assuming it
 * 				   even is supported and works)
 * 				2- I usually have no idea how printf() is handled on different systems and I cant really manipulate
 * 		 		   where the output goes on embedded systems easily, so this makes it extremely easy for me to route
 * 				   my output wherever (in my case, UART for logging stuff)
 *
 *
 * @param[in] InputStr
 * 			String with desired format specifiers
 *
 * @param[in] ...
 * 			Any additional arguments
 *
 * @return
 * 			None
 ************************************************************************************************************************/
void RML_COMM_printf( const char * InputStr, ... );



/************************************************************************************************************************
 * @brief	Extremely lightweight implementation of vprintf() for embedded systems. Prints output to UART. In case this 
 * 			was run on a system that isn't supported, routes output to stdout via printf(). Use RML_COMM_printf() unless 
 * 			you know what you are doing.
 * 
 * 			Currently supports the following specifiers:
 * 				- %s => String
 * 				- %c => Character
 * 				- %u => Unsigned integer
 * 				- %d or %i => Signed integer
 * 				- %% => To print a '%'
 * 				- %X or %x => Hex value
 * 				- %f => Float/Double, default precision is 2 decimal places
 * 				- %.Xf => Float/Double, where X is the number of decimal places (up to 15 decimal places)
 *
 * @note	Base code was gotten from: https://www.youtube.com/watch?v=Y9kUWsyyChk. Thanks to him for the explanation 
 * 			and simplified logic!
 * 
 *
 * @param[in] InputStr
 * 			String with desired format specifiers
 * 
 * @param[in] VaList
 * 			List of arguments
 * 
 * @return
 * 			None
 ************************************************************************************************************************/
void RML_COMM_vprintf( const char * InputStr, va_list VaList );



/**********************************************************************************************************************************
 * 												<!-- Printf Helper Functions -->
 **********************************************************************************************************************************/
/************************************************************************************************************************
 * @brief 	Converts an unsigned integer to a string. Returns the length of the resulting string
 * 
 * 			You can convert values with different bases, for example:
 * 				- Convert to base 10 (stores string in decimal) => RML_COMM_utoa(Value, ResultBuff, sizeof(ResultBuff), 10);
 * 				- Convert to base 16 (stores string in hex) => RML_COMM_utoa(Value, ResultBuff, sizeof(ResultBuff), 16);
 * 				- Convert to base 2 (stores string in binary) => RML_COMM_utoa(Value, ResultBuff, sizeof(ResultBuff), 2);
 * 
 * 
 * @param[in] Value
 * 			The unsigned integer to be converted
 *
 * @param[out] ResultBuff
 * 			The buffer where the resulting string will be stored
 * 
 * @param[in] ResultBuff_Size
 * 			The size of the ResultBuff buffer, you can call sizeof(ResultBuff) to get this value
 * 
 * @param[in] Base
 * 			The base to use for the conversion. The base must be between 2 and 36
 * 
 * @return
 * 			The length of the resulting string, -1 on error
 ************************************************************************************************************************/
int32_t RML_COMM_utoa(uint32_t Value, char* ResultBuff, uint32_t ResultBuff_Size, uint8_t Base);



/************************************************************************************************************************
 * @brief 	Converts a signed integer to a string. Returns the length of the resulting string
 * 
 * 			You can convert values with different bases, for example:
 * 				- Convert to base 10 (stores string in decimal) => RML_COMM_itoa(Value, ResultBuff, sizeof(ResultBuff), 10);
 * 				- Convert to base 16 (stores string in hex) => RML_COMM_itoa(Value, ResultBuff, sizeof(ResultBuff), 16);
 * 				- Convert to base 2 (stores string in binary) => RML_COMM_itoa(Value, ResultBuff, sizeof(ResultBuff), 2);
 * 
 * 
 * @param[in] Value
 * 			The signed integer to be converted
 *
 * @param[out] ResultBuff
 * 			The buffer where the resulting string will be stored
 * 
 * @param[in] ResultBuff_Size
 * 			The size of the ResultBuff buffer, you can call sizeof(ResultBuff) to get this value
 * 
 * @param[in] Base
 * 			The base to use for the conversion. The base must be between 2 and 36
 * 
 * @return
 * 			The length of the resulting string, -1 on error
 ************************************************************************************************************************/
int32_t RML_COMM_itoa(int32_t Value, char* ResultBuff, uint32_t ResultBuff_Size, uint8_t Base);



/************************************************************************************************************************
 * @brief 	This function reverses a given string
 * 
 * 
 * @param[out] Str
 * 			The string to be reversed
 *
 * @param[in] Length
 * 			Length of the string to be reversed
 * 
 * @return
 * 			None
 ************************************************************************************************************************/
void RML_COMM_ReverseString(char* Str, uint32_t Length);



/************************************************************************************************************************
 * @brief 	This function converts a double-precision floating-point number to a string representation with a specified 
 * 			number of decimal places, up to 15 decimal places. Returns the length of the resulting string
 * 
 * 
 * @param[in] Value
 * 			Double-precision floating-point number to be converted to a string
 *
 * @param[out] ResultBuff
 * 			The buffer where the resulting string will be stored
 * 
 * @param[in] ResultBuff_Size
 * 			The size of the ResultBuff buffer, you can call sizeof(ResultBuff) to get this value
 * 
 * @param[in] Afterpoint
 * 			Specifies the number of decimal places to include in the output string. If set to 0 will default to 2 decimal places
 * 
 * @return
 * 			The length of the resulting string. If the result buffer is too small, -1 is returned
 ************************************************************************************************************************/
int32_t RML_COMM_ftoa(double Value, char* ResultBuff, uint32_t ResultBuff_Size, uint8_t Afterpoint);


/**********************************************************************************************************************************
 * 												<!-- Assert Functions -->
 **********************************************************************************************************************************/
/************************************************************************************************************************
 * @brief	This function is called when #define RML_ASSERT(expr) fails. It allows you to see in which file and line 
 * 			number the assert failed. <b> Do not call directly, use the #define! </b>
 * 
 * @note	Symbol '-D' RML_ASSERT_ENABLE must be added for assert calls to work, else they will be compiled out. 
 * 			This is done because each call to assert uses the __FILE__ macro which stores the entire filepath during 
 * 			compile time. This takes up memory and depending on the hardware we might not have enough space. Having 
 * 			a it be a symbol applied during building makes it easy to quickly remove all assert calls when not needed 
 * 			to save space after debugging or for the Release build.
 * 
 *
 * @param[in] FileName
 * 			C preprocessor macro __FILE__ should be used here
 * 
 * @param[in] LineNumber
 * 			C preprocessor macro __LINE__ should be used here
 * 
 * @return
 * 			None
 ************************************************************************************************************************/
void _RML_COMM_Assert(const char* FileName, uint32_t LineNumber);



/**********************************************************************************************************************************
 * 												<!-- OTA Wrapper Functions -->
 **********************************************************************************************************************************/
/************************************************************************************************************************
 * @brief	This function sets up the Arduino OTA (Over-The-Air) update functionality. It allows you to update the firmware of
 * 			the ESP32 based Remal boards over Wi-Fi.
 * 
 * @note 	Make sure to call this function after the Wi-Fi connection is established! You also must keep calling 
 * 			RML_COMM_HandleArduinoOTA() in the main loop or a task to handle the OTA update process.
 *
 * 
 * @param[in] Hostname
 * 			Hostname for the OTA update. This will be used to identify the device on the network.
 * 			Pass NULL to use the default hostname (esp32-[MAC]).
 * 
 * @param[in] Password
 * 			Password for the OTA update. This will be used to authenticate the OTA update process.
 * 			Pass NULL to not use a password.
 * 
 * @return
 * 			None
 ************************************************************************************************************************/
void RML_COMM_SetupArduinoOTA(const char* Hostname, const char* Password);



/************************************************************************************************************************
 * @brief	This function handles the Arduino OTA (Over-The-Air) update process. It should be called in the main loop or a 
 * 			task to handle the OTA update process.
 * 
 * @note 	Make sure to call this function after the Wi-Fi connection is established and after calling 
 * 			RML_COMM_SetupArduinoOTA()!
 *
 * @return
 * 			None
 ************************************************************************************************************************/
void RML_COMM_HandleArduinoOTA();



/**********************************************************************************************************************************
 * 												<!-- LED Wrapper Functions -->
 **********************************************************************************************************************************/
/************************************************************************************************************************
 * @brief	A wrapper for Adafruit Neopixel library. Inits a given addressable LED
 * 
 * 
 * @param[in] LED_Obj
 * 			Adafruit_NeoPixel LED object, must be init already in your code. 
 * 				Ex: Adafruit_NeoPixel Shbk_LED1(SHBK_NUM_LEDS, SHBK_LED_1_PIN, NEO_GRB + NEO_KHZ800); 
 * 
 * @param[in] Brightness
 * 			The brightness to set the LED at. Range 0 - 255 XXX CHECK WHAT 0 DOES IF SET IN INIT 
 * 
 * @return
 * 			None
 ************************************************************************************************************************/
void RML_COMM_LED_Init(Adafruit_NeoPixel &LED_Obj, uint8_t Brightness);



/************************************************************************************************************************
 * @brief	A wrapper for Adafruit Neopixel library. Sets the color of a given LED
 * 
 * 
 * @param[in] LED_Obj
 * 			Adafruit_NeoPixel LED object, must be init already in your code. 
 * 				Ex: Adafruit_NeoPixel Shbk_LED1(SHBK_NUM_LEDS, SHBK_LED_1_PIN, NEO_GRB + NEO_KHZ800); 
 * 
 * @param[in] Red
 * 			Red value of the color to set the LED to. Range 0 - 255	
 * 
 * @param[in] Green
 * 			Green value of the color to set the LED to. Range 0 - 255
 * 
 * @param[in] Blue
 * 			Blue value of the color to set the LED to. Range 0 - 255
 * 
 * @return
 * 			0 on success, -1 if number of LEDs is invalid
 ************************************************************************************************************************/
int8_t RML_COMM_LED_SetLEDColor(Adafruit_NeoPixel &LED_Obj, uint8_t Red, uint8_t Green, uint8_t Blue);




#endif /* _REMAL_COMMONUTILS_H_ */
