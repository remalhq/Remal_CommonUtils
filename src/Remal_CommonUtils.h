/**
 * @file 		Remal_CommonUtils.h
 * @author 		Khalid Mansoor AlAwadhi, Remal <khalid@remal.io>
 * @date 		9 August 2024 (Initial release - 14 May 2020)
 * 
 * @brief   	This library provides various tools and utilities used by Remal developers on 
 * 				Remal hardware and other platforms. It includes a portable logger functionality 
 * 				compatible with multiple MCUs. If an MCU is not supported, the logger will default 
 * 				to using the system's native printf() function for output.
 *
 * @note		Currently Supported Processors:
 * 					- Native (PC)
 * 					- Espressif Systems ESP32 **(Remal Shabakah v3.x, v4)**
 * 					- STM32 STM32H735xx
 * 					- STM32 STM32H725xx
 * 				Refer to the README.md file for more information about this library.
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
#if defined(ESP32)
#include <Arduino.h>
/* 
	When using PlatformIO, the following must be added to the platformio.ini file:

build_flags = 
	-DARDUINO_USB_MODE=1
	-DARDUINO_USB_CDC_ON_BOOT=1

	This allows the ESP32 to use the USB port as a serial port, which this library uses for logging.
*/
#endif

//<!-- STM32 includes -->
#if defined(STM32H725xx) || defined(STM32H735xx)
#include "cmsis_os.h"
#include "semphr.h"
#endif

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

//Logging:
#define ENABLE_COLOR_SUPPORT			0				//Enable or disable color support for the logger
//ANSI text colors:
#if ENABLE_COLOR_SUPPORT
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
#else
#define ANSI_RESET     		""
#define ANSI_BLACK     		""
#define ANSI_RED       		""
#define ANSI_GREEN     		""
#define ANSI_YELLOW    		""
#define ANSI_BLUE      		""
#define ANSI_MAGENTA   		""
#define ANSI_CYAN      		""
#define ANSI_WHITE     		""
#define ANSI_BOLDBLACK     	""
#define ANSI_BOLDRED       	""
#define ANSI_BOLDGREEN     	""
#define ANSI_BOLDYELLOW    	""
#define ANSI_BOLDBLUE      	""
#define ANSI_BOLDMAGENTA   	""
#define ANSI_BOLDCYAN      	""
#define ANSI_BOLDWHITE     	""
#endif


/*********************************************
 * Structs
 *********************************************/
/**
 * @brief Custom UART structure to initialize a UART instance.
 *
 * This generic structure can be used to initialize a chosen UART instance on multiple 
 * MCUs. Check the library notes to see what processors are currently supported. On
 * a non-embedded system (like a PC), this struct is ignored and native printf() calls
 * are used.
 * 
 * Offers some parameters to be modified. For more customizability, explore the 
 * initialization function itself as not all parameters are exposed in this custom structure.
 */
typedef struct
{
   	/** GPIO Pin of the RX */
	int8_t RX_Pin;

	/** GPIO Pin of the TX */
	int8_t TX_Pin;

	/** UART Baud Rate */
	uint32_t BaudRate;
} GenericUART_Struct;


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





/*################################################################################################################################
  #													<!-- Logging functions -->
  ################################################################################################################################*/
/************************************************************************************************************************
 * @brief	Initializes the Logger. On a non-embedded system or a system with native printf() support like a PC this 
 * 			function does nothing.
 * 
 * 			=> UART Settings:
 * 				- 8 data bits
 * 				- No parity
 * 				- 1 stop bit
 *
 * @note	If you're using Shabakah v3.x or higher (ESP32), this function will default to using the native USB
 * 			port for logging. In the future we plan on having custom pin selection.
 * 
 * 			Also note when using PlatformIO, the following must be added to the platformio.ini file:
 * 				build_flags = 
 * 					-DARDUINO_USB_MODE=1
 * 					-DARDUINO_USB_CDC_ON_BOOT=1
 * 			This allows the ESP32 to use the USB port as a serial port, which this library uses for logging.
 *   
 * @note	If you're using this library on a STM32xx, it assumes you already have a UART instance initialized
 * 			through the CubeMX code generator. Make sure to define and fill in the UARTComm struct with the correct
 * 			settings. Additionally, check the function itself and see what UART instance it is using and pins so it
 * 			matches your IOC file.
 * 
 * 
 * @param[in] UARTComm
 * 			Structure that contains all the wanted UART settings to initialize. On systems that are not
 * 			embedded or do not support UART, this param is ignored
 *
 * @return
 * 			0 on success, -1 on failure
 ************************************************************************************************************************/
int8_t RML_COMM_LoggerInit(GenericUART_Struct *UARTComm);



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
void RML_COMM_LogMsg(char *Src, uint8_t LogLvl, char* Msg, ... );



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



/*################################################################################################################################
  #													<!-- printf functions -->
  ################################################################################################################################*/
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
void RML_COMM_printf( char * InputStr, ... );



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
void RML_COMM_vprintf( char * InputStr, va_list VaList );



/*################################################################################################################################
  #													<!-- String convertor functions -->
  ################################################################################################################################*/
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
 * 			The length of the resulting string. If the base is invalid or the result buffer is too small, -1 is returned
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
 * 			The length of the resulting string. If the base is invalid or the result buffer is too small, -1 is returned
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



#endif /* _REMAL_COMMONUTILS_H_ */
