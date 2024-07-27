/**
 * @file 		Remal_CommonUtils.h
 * @author 		Khalid M AlAwadhi (khalid@remal.io), Remal
 * @date 		27 July 2024 (Initial release - 14 May 2020)
 * 
 * @brief   	This library contains custom logger functionality that is portable across 
 *          	portable across multiple MCUs. If a certain MCU is not supported, the logger will route
 *  			the output to the native system printf() function.
 *
 * @note		Currently supported processor are:
 * 					> Native (PC)
 *          		> Espressif Systems ESP32 
 * 					> STM32 STM32F411xE
 * 					> [WORKING] STM32 STM32H7 and STM32F4
**/
#ifndef _REMAL_COMMONUTILS_H_
#define _REMAL_COMMONUTILS_H_


//<!-- Standard includes -->
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>

//<!-- ESP32 includes -->
#if defined(ESP32)
#include <Arduino.h>
#endif

//<!-- STM32 includes -->
#if defined(STM32H7) || defined(STM32F411xE)
#include "cmsis_os.h"
#include "semphr.h"
#endif

/*################################################################################################################################
  #													<!-- Defines/Structs/Enums -->
  ################################################################################################################################*/
/*************************************************************************************
 * @brief Custom UART structure to initialize a UART instance.
 *
 * This generic structure can be used to initialize a chosen UART instance on multiple 
 * MCUs. Check the library notes to see what processors are currently supported. On
 * a non-embedded system (like a PC), this struct is ignored and native printf() calls
 * are used.
 * 
 * Offers a lot of parameters to be modified. For more customizability, explore the 
 * initialization function itself as not all parameters are exposed in this custom 
 * structure. However, most of them are so you might not need to.
 *************************************************************************************/
typedef struct
{
   	/** GPIO Pin of the RX */
	int8_t RX_Pin;

	/** GPIO Pin of the TX */
	int8_t TX_Pin;

	/** UART Baud Rate */
	uint32_t BaudRate;
} GenericUART_Struct;

/***************************************
 * @brief Log Level enum:
 * Used distinguish different log levels
 ***************************************/
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
 * @note	ESP32:
 * 				On ESP32 we use native USB serial, you cannot choose the UART pins or baud rate. Any logs will simply be output
 * 				to the native USB serial port.
 * 			
 * @note	STM32:
 * 				If you're using this library on a STM32F411xE or STM32H7, it assumes you already have a UART instance initialized
 * 				throught the CubeMX code generator. Make sure to define and fill in the UARTComm struct with the correct
 * 				settings. Also, if you're using this library on a STM32F411xE it will assume you're using USART1 which is hardwired
 * 				to use pins PA9 and PA10, so the pins in the struct will be ignored.
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
void RML_COMM_LogMsg(char *Src, uint8_t LogLvl, const char* Msg, ... );




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
 * 				- %.Xf => Float/Double, where X is the number of decimal places (up to 6 decimal places)
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
void RML_COMM_printf( const char* InputStr, ... );



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
 * 				- %.Xf => Float/Double, where X is the number of decimal places (up to 6 decimal places)
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
void RML_COMM_vprintf( const char* InputStr, va_list VaList );




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
 * 			number of decimal places
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




#endif /* _REMAL_COMMONUTILS_H_ */
