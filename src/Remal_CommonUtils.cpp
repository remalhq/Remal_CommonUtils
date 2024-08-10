/*
 * Remal_CommonUtils.c
 *
 *  # ALL INFO CAN BE FOUND IN THE HEADER FILE #
 */
#include "Remal_CommonUtils.h"


/*********************************************
 * Private Enums
 *********************************************/
/***************************************
 * @brief CurrentMCU enum:
 * Used to give each MCU a value as 
 * opposed to a magic number.
 ***************************************/
typedef enum
{
	e_Native = 0,
	e_ESP_ESP32 = 1,
	e_STM32_STM32xx = 2
} CurrentMCU_Enum;



/*********************************************
 * Private Variables
 *********************************************/
static uint8_t Logger_InitDone = 0;					//This flag is set to true when RML_COMM_LoggerInit() is called and is successful. Used for error handling

/*
 * The statements below handle deciding what processor 
 * architecture we are running on  
 */
#if defined(ESP32)
	#pragma message("Auto-detected to be running on ESP32, make sure you added the lines in platformio.ini to enable logging via native USB")
	static uint8_t CurrentMCU = e_ESP_ESP32;
	static uint32_t MaxBaudrate = 115200;
	#define PUTCHAR_FUNC		Serial.printf("%c",		
	#define PUTCHAR_N_FUNC		Serial.printf("%s",
	static portMUX_TYPE LogSpinlock = portMUX_INITIALIZER_UNLOCKED;			//Define a spinlock object for the shared resource - so logging tasks dont clash

#elif defined(STM32H725xx) || defined(STM32H735xx)
	#pragma message("Auto-detected to be running on STM32H7xxxx")
	#include "stm32h7xx_hal.h"
	#include <string.h>

	SemaphoreHandle_t LogMutex;

	/* Decide which UART to use based on the processor: */
	#if defined(STM32H735xx)
		extern UART_HandleTypeDef huart3;
		#define STM32_UART_HNDLR	&huart3
	#else
		extern UART_HandleTypeDef huart1;
		#define STM32_UART_HNDLR	&huart1
	#endif

	//Transmit a single character
	void UART_PutChar(char ch)
	{
		//if( xSemaphoreTake(LogMutex, 1200) == pdTRUE )
		//{
			HAL_UART_Transmit(STM32_UART_HNDLR, (uint8_t*)&ch, 1, 1000);
		//	xSemaphoreGive(LogMutex);
		//}
	}

	//Transmit a string
	void UART_PutChar_n(const char* str)
	{
		HAL_UART_Transmit(STM32_UART_HNDLR, (uint8_t*)str, strlen(str), 1000);
	}

	static uint8_t CurrentMCU = e_STM32_STM32xx;
	#define PUTCHAR_FUNC		UART_PutChar(		
	#define PUTCHAR_N_FUNC		UART_PutChar_n(
	static uint32_t MaxBaudrate = 115200;

#else
	#pragma message("Auto-detected to be running on PC or unsupported platform, defaulting to printf()")
	//The lines below are used to add code specifically for machines with native printf() support
	static uint8_t CurrentMCU = e_Native;
	#define PUTCHAR_FUNC		printf("%c",		
	#define PUTCHAR_N_FUNC		printf("%s",
	static uint32_t MaxBaudrate = 115200;			//Unused for native
#endif


/*************************************************
 * @brief Log Levels to log:
 * By default, all log messages are enabled.
 *  
 * Use RML_COMM_LogLevelSet() function to enable
 * or disable specific log level messages
 *************************************************/
static uint8_t LogLevelsEnable[5] = 
{
	1,		//0- Log "Debug" messages
	1,		//1- Log "Info" messages
	1,		//2- Log "Warning" messages
	1,		//3- Log "Error" messages
	1,		//4- Log "Fatal" messages
};

/*************************************************
 * @brief Log Level strings:
 * Used when outputting log message
 *************************************************/
char LogLevel_Str[5][10] =
{
	"DEBUG",
	"INFO",
	"WARNING",
	"ERROR",
	"FATAL"
};






int8_t RML_COMM_LoggerInit(GenericUART_Struct *UARTComm)
{
	/* Error check: 
	* is the baudrate greater than the max value or 0 */
	if(UARTComm->BaudRate > MaxBaudrate || UARTComm->BaudRate == 0)
	{
		return -1;
	}

	/* Based on the processor we are running on, call different UART init functions: */
	switch (CurrentMCU)
	{
		case e_ESP_ESP32:
			#if defined(ESP32)
			/* We use native USB port, no need to set pins */
			Serial.begin(UARTComm->BaudRate);
			Serial.setTxTimeoutMs(0);				//This is used to avoid waiting if the USB is not connected 
			#endif
			break;

		case e_STM32_STM32xx:

			/* STM32H725: */
			#if defined(STM32H725xx)
			// Nothing to do assuming the user already init UART1 and the pins are hard-wired to be
			//		> RX Pin: B15
			//		> TX Pin: A9
			LogMutex = xSemaphoreCreateMutex();
			#endif

			/* STM32H735: */
			#if defined(STM32H735xx)
			// Nothing to do assuming the user already init UART3 and the pins are hard-wired to be
			//		> RX Pin: PD9
			//		> TX Pin: PD8
			LogMutex = xSemaphoreCreateMutex();
			#endif
			break;
		
		default:
			//Native selected - nothing to do
			break;
	}

	/* Logger was init successfully */
	Logger_InitDone = 1;

	return 0;
}




void RML_COMM_LogMsg(char *Src, uint8_t LogLvl, char* Msg, ... )
{
	/* Error check: Makes sure the logger was initialized */
	if(!Logger_InitDone)
	{
		return;
	}

	char *ColorStr = "";				//Used to color the log level string


	/* Check if Log level is defined to be logged: */
	uint8_t LogLvlUnknown = 0;			//Used to check if the Log level is defined or not
	if(LogLvl == e_DEBUG)
	{
		if(LogLevelsEnable[0] == 0)
		{
			return;
		}

		/* Debug is cyan */
		ColorStr = ANSI_CYAN;
	}
	else if(LogLvl == e_INFO)
	{
		if(LogLevelsEnable[1] == 0)
		{
			return;
		}

		/* Info is green */
		ColorStr = ANSI_GREEN;
	}
	else if(LogLvl == e_WARNING)
	{
		if(LogLevelsEnable[2] == 0)
		{
			return;
		}

		/* Warning is yellow */
		ColorStr = ANSI_YELLOW;
	}
	else if(LogLvl == e_ERROR)
	{
		if(LogLevelsEnable[3] == 0)
		{
			return;
		}

		/* Error is red */
		ColorStr = ANSI_RED;
	}
	else if(LogLvl == e_FATAL)
	{
		if(LogLevelsEnable[4] == 0)
		{
			return;
		}

		/* Fatal is bold red */
		ColorStr = ANSI_BOLDRED;
	}
	else
	{
		/* Log level is unknown */
		LogLvlUnknown = 1;
	}

	#if defined(ESP32)
	taskENTER_CRITICAL(&LogSpinlock);
	#endif

	/* The log message is sent by segments depending on
	 * what needs to be sent or formatting */
	PUTCHAR_N_FUNC ColorStr);					//Color string
	PUTCHAR_N_FUNC "> [");

	/* Output log level: */
	if( LogLvlUnknown )
	{
		PUTCHAR_N_FUNC "Unknown LogLvl?");
	}
	else
	{
		PUTCHAR_N_FUNC LogLevel_Str[LogLvl]);	//LogLevel
	}
	PUTCHAR_N_FUNC "] ");
	

	/* Logs source of log (inception) */
	PUTCHAR_N_FUNC Src);
	PUTCHAR_N_FUNC ": ");

	/* Logs message */
	va_list VaList;							//Declare Variable-length argument list to store any additional args
	va_start(VaList, Msg);					//Create a list for arguments given after 'Msg'
	RML_COMM_vprintf(Msg, VaList);
	va_end(VaList);							//Clean up the list

	/* Newline */
	PUTCHAR_N_FUNC ANSI_RESET);
	PUTCHAR_N_FUNC "\r\n");

	#if defined(ESP32)
	taskEXIT_CRITICAL(&LogSpinlock);
	#endif
}



int8_t RML_COMM_LogLevelSet(uint8_t LogLvl, uint8_t Enable)
{
	/* Error check: Makes sure the logger was initialized */
	if(!Logger_InitDone)
	{
		return -1;
	}

	/* Sanitize input just in case: */
	if(Enable > 1)
	{
		Enable = 1;
	}
	
	/* Check if Log level is defined to be logged: */
	if(LogLvl == e_DEBUG)
	{
		LogLevelsEnable[0] = Enable;
	}
	else if(LogLvl == e_INFO)
	{
		LogLevelsEnable[1] = Enable;
	}
	else if(LogLvl == e_WARNING)
	{
		LogLevelsEnable[2] = Enable;
	}
	else if(LogLvl == e_ERROR)
	{
		LogLevelsEnable[3] = Enable;
	}
	else if(LogLvl == e_FATAL)
	{
		LogLevelsEnable[4] = Enable;
	}
	else
	{
		/* Log level is unknown */
		return -1;
	}

	return 0;
}




void RML_COMM_printf( char * InputStr, ... )
{
	/* Error check: Makes sure the logger was initialized */
	if(!Logger_InitDone)
	{
		return;
	}

	va_list VaList;							//Declare Variable-length argument list to store any additional args
	va_start(VaList, InputStr);				//Create a list for arguments given after 'InputStr'

	RML_COMM_vprintf(InputStr, VaList);	//Call the vprintf function to handle the rest

	va_end(VaList);							//Clean up the list
}




void RML_COMM_vprintf( char * InputStr, va_list VaList )
{
	/* Error check: Makes sure the logger was initialized */
	if(!Logger_InitDone)
	{
		return;
	}
	
	char *StringArg;			//Will be used to store any string args
	char CharArg; 				//Will be used to store any char args
	uint32_t UnsignedArg;		//Will be used to store any unsigned args
	int32_t SignedArg;			//Will be used to store any signed args
	char IntStr[20];			//Will be used to store any converted ints/floats/doubles
	double DoubleArg; 			//Will be used to store any double args


	/* Loop over the given string, check for '%' for formatting */
	while(*InputStr)
	{
		/* If we got a format specifier: */
		if(*InputStr == '%')
		{
			InputStr++;																			//Move to the next array element

			switch(*InputStr)																	//Based on the specifier, decide what to do
			{
				//String
				case 's':
					StringArg = va_arg(VaList, char *);											//Get the arg, type string
					PUTCHAR_N_FUNC StringArg);													//Print string
					InputStr++;																	//Increment to remove the specifier from printing
					break;

				//Character
				case 'c':
					CharArg = va_arg(VaList, int);												//Get the arg, type char (va_arg() needs int for char)
					PUTCHAR_FUNC CharArg);														//Print char
					InputStr++;																	//Increment to remove the specifier from printing
					break;

				//Unsigned int
				case 'u':
					UnsignedArg = va_arg(VaList, uint32_t);										//Get the arg, type unsigned
					RML_COMM_utoa(UnsignedArg, IntStr, sizeof(IntStr), 10);					//Convert unsigned int to ascii, base 10
					PUTCHAR_N_FUNC IntStr);														//Print string
					InputStr++;																	//Increment to remove the specifier from printing
					break;

				//Signed int
				case 'i':
				case 'd':
					SignedArg = va_arg(VaList, int32_t);										//Get the arg, type signed
					RML_COMM_itoa(SignedArg, IntStr, sizeof(IntStr), 10);						//Convert signed int to ascii, base 10
					PUTCHAR_N_FUNC IntStr); 													//Print string
					InputStr++;																	//Increment to remove the specifier from printing
					break;

				//User wants to print a '%'
				case '%':
					PUTCHAR_FUNC '%');															//Print char
					InputStr++;																	//Increment to remove the specifier from printing
					break;

				//Hex value
				case 'X':
				case 'x':
					UnsignedArg = va_arg(VaList, uint32_t);										//Get the arg, type unsigned
					RML_COMM_utoa(UnsignedArg, IntStr, sizeof(IntStr), 16);					//Convert unsigned int to ascii, base 16
					PUTCHAR_N_FUNC IntStr);														//Print string
					InputStr++;
					break;

				//User wants to set the number of decimal places for the float/double 
				case '.':
					InputStr++;																	//Get the next value whoch should be between 1-6
					//Check what value InputStr contains:
					switch(*InputStr)
					{
						//1 decimal place
						case '1':
							DoubleArg = va_arg(VaList, double);									//Get the arg, type double
							RML_COMM_ftoa(DoubleArg, IntStr, sizeof(IntStr), 1);				//Convert float/double to ascii, 1 decimal place
							PUTCHAR_N_FUNC IntStr);												//Print string
							InputStr = InputStr + 2;											//Increment to remove the 'f' specifier from printing
							break;

						//2 decimal places
						case '2':
							DoubleArg = va_arg(VaList, double);									//Get the arg, type double
							RML_COMM_ftoa(DoubleArg, IntStr, sizeof(IntStr), 2);				//Convert float/double to ascii, 2 decimal places
							PUTCHAR_N_FUNC IntStr);												//Print string
							InputStr = InputStr + 2;											//Increment to remove the 'f' specifier from printing
							break;

						//3 decimal places
						case '3':
							DoubleArg = va_arg(VaList, double);									//Get the arg, type double
							RML_COMM_ftoa(DoubleArg, IntStr, sizeof(IntStr), 3);				//Convert float/double to ascii, 3 decimal places
							PUTCHAR_N_FUNC IntStr);												//Print string
							InputStr = InputStr + 2;											//Increment to remove the 'f' specifier from printing
							break;

						//4 decimal places
						case '4':
							DoubleArg = va_arg(VaList, double);									//Get the arg, type double
							RML_COMM_ftoa(DoubleArg, IntStr, sizeof(IntStr), 4);				//Convert float/double to ascii, 3 decimal places
							PUTCHAR_N_FUNC IntStr);												//Print string
							InputStr = InputStr + 2;											//Increment to remove the 'f' specifier from printing
							break;

						//5 decimal places
						case '5':
							DoubleArg = va_arg(VaList, double);									//Get the arg, type double
							RML_COMM_ftoa(DoubleArg, IntStr, sizeof(IntStr), 5);				//Convert float/double to ascii, 3 decimal places
							PUTCHAR_N_FUNC IntStr);												//Print string
							InputStr = InputStr + 2;											//Increment to remove the 'f' specifier from printing
							break;

						//6 decimal places
						case '6':
							DoubleArg = va_arg(VaList, double);									//Get the arg, type double
							RML_COMM_ftoa(DoubleArg, IntStr, sizeof(IntStr), 6);				//Convert float/double to ascii, 3 decimal places
							PUTCHAR_N_FUNC IntStr);												//Print string
							InputStr = InputStr + 2;											//Increment to remove the 'f' specifier from printing
							break;

						//Unknown specifier - just print it
						default:
							PUTCHAR_FUNC *InputStr);											//Print char
							InputStr++;
							break;
					}
					break;

				//Double/float value
				case 'f':
					DoubleArg = va_arg(VaList, double);											//Get the arg, type double
					RML_COMM_ftoa(DoubleArg, IntStr, sizeof(IntStr), 2);						//Convert float/double to ascii, 2 decimal places by default
					PUTCHAR_N_FUNC IntStr);														//Print string
					InputStr++;
					break;

				//End of string reached
				case '\0':
					break;

				//Unknown specifier - just print it in hopes of the user realizing that
				default:
					PUTCHAR_FUNC '%');															//Print char
					PUTCHAR_FUNC *InputStr);													//Print char
					InputStr++;
			}
		}

		/* Not a format specifier, print the char: */
		else
		{
			PUTCHAR_FUNC *InputStr);															//Print char
			InputStr++;																			//Move to next char in the string
		}
	}
}




int32_t RML_COMM_utoa(uint32_t Value, char* ResultBuff, uint32_t ResultBuff_Size, uint8_t Base)
{
	// check that the base if valid
	if (Base < 2 || Base > 36) 
	{
		// if the base is invalid, return an empty string
		*ResultBuff = '\0';
		return -1;
	}
	
	// initialize pointers to the start and end of the result string
	char* ptr = ResultBuff, *ptr1 = ResultBuff;
	
	// store the input value for later use
	uint32_t tmp_value;
	
	// do-while loop to repeatedly divide the value by the base and store the remainder as a character in the result string
	do
	{
		tmp_value = Value;
		Value /= Base;
		
		// lookup the character for the current remainder in the lookup table and store it in the result string
		*ptr++ = "ZYXWVUTSRQPONMLKJIHGFEDCBA9876543210123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ" [35 + (tmp_value - Value * Base)];
	} while ( Value );
	
	// check if the result buffer is large enough to hold the resulting string
	if (ptr - ResultBuff >= ResultBuff_Size) 
	{
		// if the result buffer is too small, return an error
		*ResultBuff = '\0';
		return -1;
	}
	
	// terminate the string
	*ptr-- = '\0';
	
	// reverse the string
	while(ptr1 < ptr)
	{
		char tmp_char = *ptr;
		*ptr--= *ptr1;
		*ptr1++ = tmp_char;
	}
	
	// return the length of the string
	return ptr - ResultBuff;
}




int32_t RML_COMM_itoa(int32_t Value, char* ResultBuff, uint32_t ResultBuff_Size, uint8_t Base)
{
	// check that the base if valid
	if (Base < 2 || Base > 36) 
	{
		// if the base is invalid, return an empty string
		*ResultBuff = '\0';
		return -1;
	}
	
	// initialize pointers to the start and end of the result string
	char* ptr = ResultBuff, *ptr1 = ResultBuff, tmp_char;
	
	// store the input value for later use
	int32_t tmp_value, tmp_value2;

	tmp_value2 = Value;
	
	// do-while loop to repeatedly divide the value by the base and store the remainder as a character in the result string
	do 
	{
		tmp_value = Value;
		Value /= Base;
		
		// lookup the character for the current remainder in the lookup table and store it in the result string
		*ptr++ = "ZYXWVUTSRQPONMLKJIHGFEDCBA9876543210123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ" [35 + (tmp_value - Value * Base)];
	} while ( Value );

	// handle negative values
	if (tmp_value2 < 0 && Base == 10)
	{
		// store the negative sign in the result string
		*ptr++ = '-';
	}
	
	// check if the result buffer is large enough to hold the resulting string
	if (ptr - ResultBuff >= ResultBuff_Size) 
	{
		// if the result buffer is too small, return an error
		*ResultBuff = '\0';
		return -1;
	}
	
	// terminate the string
	*ptr-- = '\0';
	
	// reverse the string
	while (ptr1 < ptr) 
	{
		tmp_char = *ptr;
		*ptr--= *ptr1;
		*ptr1++ = tmp_char;
	}
	
	// return the length of the string
	return ptr - ResultBuff;
}




void RML_COMM_ReverseString(char* Str, uint32_t Length)
{
	uint32_t i, j;
	char temp;
	for (i = 0, j = Length - 1; i < j; i++, j--)
	{
		temp = Str[i];
		Str[i] = Str[j];
		Str[j] = temp;
	}
}




int32_t RML_COMM_ftoa(double Value, char* ResultBuff, uint32_t BuffSize, uint8_t Afterpoint)
{
	int32_t WholePart = (int32_t)Value;
	double FractionalPart = Value - WholePart;
	uint32_t i = 0;
	uint8_t NegativeFlag = 0;
	
	// Handle negative numbers
	if( WholePart < 0 )
	{
		NegativeFlag = 1;
		WholePart = -WholePart;
		FractionalPart = -FractionalPart;
	}

	// Convert whole part to string
	do
	{
		if (i < BuffSize)
		{
			ResultBuff[i++] = WholePart % 10 + '0';
		}
		else
		{
			ResultBuff[0] = '\0';
			return -1;
		}
		WholePart /= 10;
	} while (WholePart > 0);

	if(NegativeFlag && i < BuffSize)
	{
		ResultBuff[i++] = '-';
	}

	// Reverse the string
	RML_COMM_ReverseString(ResultBuff, i);
	
	
	// if Afterpoint is non-zero, add a decimal point and the specified number of decimal places
	if( Afterpoint > 0 )
	{
		if (i < BuffSize)
		{
			ResultBuff[i++] = '.';
		}
		else
		{
			// Buffer is full
			ResultBuff[i] = '\0';
			return -1;
		}
		
		for( int32_t z = 0; z < Afterpoint; z++ )
		{
			FractionalPart *= 10;
			int32_t digit = (int32_t)FractionalPart;
			if (i < BuffSize)
			{
				ResultBuff[i++] = digit + '0';
			}
			else
			{
				ResultBuff[0] = '\0';
				return -1;
			}
			FractionalPart -= digit;
		}
	}
	
	// Add null terminator
	if (i + 1 > BuffSize)
	{
		ResultBuff[0] = '\0'; // Return empty string if buffer size is too small
	}
	else
	{
		ResultBuff[i] = '\0';
	}

	return i;
}



void _RML_COMM_Assert(const char* FileName, uint32_t LineNumber)
{
	// char FileNameStr[20] = {0};
	// snprintf(FileNameStr, 20, "%s", FileName);
	
	/* 
	 * Assertion failed - Pause debugger and look at values
	 */
	RML_COMM_LogMsg("RML_ASSERT", e_FATAL, "ASSERTION FAILED:\r\n\t--> File: %s\r\n\t--> Line: %u", FileName, LineNumber);
	while(1);
}


