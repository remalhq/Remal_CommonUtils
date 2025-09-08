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
static const char DIGITS[] = "ZYXWVUTSRQPONMLKJIHGFEDCBA9876543210123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"; 	//Used for itoa/utoa functions

/*
 * The statements below handle deciding what processor 
 * architecture we are running on  
 */
#if defined(ESP32)
	#pragma message("Auto-detected to be running on ESP32, make sure you added the lines in platformio.ini to enable logging via native USB")
	static uint8_t CurrentMCU = e_ESP_ESP32;
	static uint32_t MaxBaudrate = 115200;
	#define PUTCHAR_FUNC        Serial.printf("%c",
	#define PUTCHAR_N_FUNC      Serial.printf("%s",
	static SemaphoreHandle_t LogMutex = NULL;					// Mutex to protect logging from multiple tasks
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

			/* Create mutex */
			if (LogMutex == NULL)
			{
				LogMutex = xSemaphoreCreateMutex();
			}
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




void RML_COMM_LogMsg(const char *Src, uint8_t LogLvl, const char* Msg, ... )
{
	/* Error check: Makes sure the logger was initialized */
	if(!Logger_InitDone)
	{
		return;
	}

	const char *ColorStr = "";				// Used to color the log level string


	/* Check if Log level is defined to be logged: */
	uint8_t LogLvlUnknown = 0;				// Used to check if the Log level is defined or not
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
	if (LogMutex)
	{
		xSemaphoreTake(LogMutex, portMAX_DELAY);
	}
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
	if (LogMutex)
	{
		xSemaphoreGive(LogMutex);
	}
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



int8_t RML_COMM_LogStackUsage(UBaseType_t TaskStackSize)
{
	/* Error check: Stack size is valid */
	if(TaskStackSize <= 0)
	{
		return -1;
	}
	
	UBaseType_t StackFreeWords = uxTaskGetStackHighWaterMark(NULL);

	// Calculate remaining free stack in bytes
	UBaseType_t StackFreeBytes = StackFreeWords * sizeof(StackType_t);

	// Calculate total stack in bytes (since TaskStackSize is in words)
	UBaseType_t TotalStackBytes = TaskStackSize * sizeof(StackType_t);

	// Use float division to avoid truncation
	float StackFreePercent = ((float)StackFreeBytes / TotalStackBytes) * 100.0f;
	float StackUsedPercent = 100.0f - StackFreePercent;

	// Log usage
	RML_COMM_LogMsg(pcTaskGetTaskName(NULL),
					e_DEBUG, 
					"Stack usage: %.2f%% used, %.2f%% free (%u bytes free of %u bytes total)", 
					StackUsedPercent, 
					StackFreePercent, 
					StackFreeBytes, 
					TotalStackBytes
					);

	return 0;
}



void RML_COMM_printf( const char * InputStr, ... )
{
	/* Error check: Makes sure the logger was initialized */
	if(!Logger_InitDone)
	{
		return;
	}
	
	#if defined(ESP32)
	if (LogMutex)
	{
		xSemaphoreTake(LogMutex, portMAX_DELAY);
	}
	#endif

	va_list VaList;							// Declare Variable-length argument list to store any additional args
	va_start(VaList, InputStr);				// Create a list for arguments given after 'InputStr'

	RML_COMM_vprintf(InputStr, VaList);		// Call the vprintf function to handle the rest

	va_end(VaList);							// Clean up the list
	
	#if defined(ESP32)
	if (LogMutex)
	{
		xSemaphoreGive(LogMutex);
	}
	#endif
}



void RML_COMM_vprintf( const char * InputStr, va_list VaList )
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
	char IntStr[40];			//Will be used to store any converted ints/floats/doubles
	double DoubleArg; 			//Will be used to store any double args
	uint8_t Decimals;			//Will be used to store the number of decimal places for the float/double


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
					RML_COMM_utoa(UnsignedArg, IntStr, sizeof(IntStr), 16);						//Convert unsigned int to ascii, base 16
					PUTCHAR_N_FUNC IntStr);														//Print string
					InputStr++;
					break;

				//User wants to set the number of decimal places for the float/double 
				case '.':
					InputStr++;																	//Get the next value which should be between 1-6
					
					Decimals = 0;

					// Handle two-digit precision (e.g., .10 to .15)
					if (isdigit(*InputStr))
					{
						Decimals = (*InputStr++) - '0';											//Get the number of decimal places

						if (isdigit(*InputStr)) 												//Check if second digit
						{
							Decimals = (Decimals * 10) + ((*InputStr++) - '0');					//Get the second digit
						}
					}
					
					// Limit decimals to a maximum of 15
					if (Decimals > 15)
					{
						Decimals = 15;
					}
					if (Decimals == 0)
					{
						Decimals = 2; 															//Default if somehow zero
					}

					if (*InputStr == 'f')
					{
						DoubleArg = va_arg(VaList, double);
						RML_COMM_ftoa(DoubleArg, IntStr, sizeof(IntStr), Decimals);
						PUTCHAR_N_FUNC IntStr);
						InputStr++; // move past 'f'
					}
					else
					{
						// Unknown specifier; print literally
						PUTCHAR_FUNC '%');
						PUTCHAR_FUNC '.');
						PUTCHAR_FUNC *InputStr);
						InputStr++;
					}
					break;

				//Double/float value with no specified decimal places
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



int32_t RML_COMM_utoa(uint32_t v, char* ResultBuff, uint32_t ResultBuff_Size, uint8_t Base)
{
	/* Error check: ResultBuff is NULL */
	if( ResultBuff == NULL )
	{
		return -1;
	}

	/* Error check: ResultBuff_Size is 0 */
	if( ResultBuff_Size == 0 )
	{
		return -1;
	}

	/* Error check: Base is valid */
	if( Base < 2 || Base > 36 )
	{
		return -1;
	}

    uint32_t len = 0;

    /* Build in reverse with strict bounds checking (always keep room for '\0') */
    do
	{
		if (len + 1 >= ResultBuff_Size)
		{ 
			ResultBuff[0] = '\0';
			return -1;
		}
        uint32_t q = v / Base;
        uint32_t r = v - q * Base;               /* 0..Base-1 */
        ResultBuff[len++] = DIGITS[35 + (int32_t)r];
        v = q;
    } while (v);

    /* Reverse in-place */
    for (uint32_t i = 0, j = len - 1; i < j; ++i, --j)
	{
        char t = ResultBuff[i]; 
		ResultBuff[i] = ResultBuff[j]; 
		ResultBuff[j] = t;
    }

    ResultBuff[len] = '\0';
    return (int32_t)len;
}



int32_t RML_COMM_itoa(int32_t Value, char* ResultBuff, uint32_t ResultBuff_Size, uint8_t Base)
{
	/* Error check: ResultBuff is NULL */
	if( ResultBuff == NULL )
	{
		return -1;
	}

	/* Error check: ResultBuff_Size is 0 */
	if( ResultBuff_Size == 0 )
	{
		return -1;
	}

	/* Error check: Base is valid */
	if( Base < 2 || Base > 36 )
	{
		return -1;
	}

    uint32_t len = 0;
    int32_t v = Value;

    /* Build in reverse with strict bounds checking (always keep room for '\0') */
    do 
	{
        if (len + 1 >= ResultBuff_Size)
		{ 
			ResultBuff[0] = '\0'; 
			return -1;
		}
        int32_t q = v / (int32_t)Base;           /* truncates toward zero */
        int32_t r = v - q * (int32_t)Base;       /* -(Base-1)..(Base-1) */
        ResultBuff[len++] = DIGITS[35 + r];      /* handles negative r via lookup */
        v = q;
    } while (v);

    /* Prepend '-' only for base 10, matching your original behavior */
    if (Value < 0 && Base == 10)
	{
        if (len + 1 >= ResultBuff_Size)
		{ 
			ResultBuff[0] = '\0'; 
			return -1;
		}
        ResultBuff[len++] = '-';
    }

    /* Reverse in-place */
    for (uint32_t i = 0, j = len - 1; i < j; ++i, --j)
	{
        char t = ResultBuff[i]; 
		ResultBuff[i] = ResultBuff[j]; 
		ResultBuff[j] = t;
    }

    ResultBuff[len] = '\0';
    return (int32_t)len;
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
	if (Value < 0)
	{
		NegativeFlag = 1;
		WholePart = -WholePart;
		FractionalPart = -FractionalPart;
	}

	// Convert fractional part to integer for rounding
	double rounding = 0.5;
	for (uint8_t j = 0; j < Afterpoint; ++j)
		rounding /= 10;

	FractionalPart += rounding;

	if (FractionalPart >= 1.0)
	{
		WholePart += 1;
		FractionalPart -= 1.0;
	}

	// Convert whole part to string
	char TempBuff[32] = {0};
	uint32_t idx = 0;
	do
	{
		TempBuff[idx++] = (WholePart % 10) + '0';
		WholePart /= 10;
	} while (WholePart && idx < sizeof(TempBuff) - 1);

	if (NegativeFlag && idx < sizeof(TempBuff) - 1)
		TempBuff[idx++] = '-';

	// Reverse TempBuff into ResultBuff
	while (idx--)
	{
		if (i < BuffSize - 1)
			ResultBuff[i++] = TempBuff[idx];
		else
		{
			ResultBuff[0] = '\0';
			return -1;
		}
	}

	// Add decimal point and fractional digits
	if (Afterpoint > 0)
	{
		if (i < BuffSize - 1)
			ResultBuff[i++] = '.';
		else
		{
			ResultBuff[0] = '\0';
			return -1;
		}

		for (uint8_t j = 0; j < Afterpoint; ++j)
		{
			FractionalPart *= 10;
			int digit = (int)(FractionalPart);
			if (i < BuffSize - 1)
				ResultBuff[i++] = digit + '0';
			else
			{
				ResultBuff[0] = '\0';
				return -1;
			}
			FractionalPart -= digit;
		}
	}

	ResultBuff[i] = '\0';

	return i; // length of string
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




void RML_COMM_SetupArduinoOTA(const char* Hostname, const char* Password)
{
	static const char FuncName[] = "RML_COMM_SetupArduinoOTA";

	/* Check if hostname is given */
	if (Hostname != NULL)
	{
		ArduinoOTA.setHostname(Hostname);
	}

	/* Check if password is given */
	if (Password != NULL)
	{
		ArduinoOTA.setPassword(Password);
	}

	/* Setup OTA handlers */
	ArduinoOTA.onStart([]()
	{
		String type;

		/* Check if the update is for the sketch or SPIFFS */
		if (ArduinoOTA.getCommand() == U_FLASH) 
		{
			type = "sketch";
		} 
		else 
		{ // U_SPIFFS
			type = "filesystem";

			/* NOTE: If updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end() */
		}
		
		RML_COMM_LogMsg(FuncName, e_INFO, "Start updating %s", type.c_str());
	});

	ArduinoOTA.onEnd([]()
	{
		RML_COMM_LogMsg(FuncName, e_INFO, "End");
	});

	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) 
	{
		// Clamp: if progress > total (shouldn't happen, but be safe) use total
		const uint32_t capped = (total && progress > total) ? total : progress;
		// Guard + 64-bit math: If total==0 → pct=0; else pct = floor((capped*100) / total) with 64-bit multiply
		const uint32_t pct = total ? (uint32_t)(((uint64_t)capped * 100u) / (uint64_t)total) : 0u;
		RML_COMM_LogMsg(FuncName, e_INFO, "Progress: %u%%", pct);
	});
	
	ArduinoOTA.onError([](ota_error_t error) 
	{
		RML_COMM_LogMsg(FuncName, e_ERROR, "Error[%u]: ", error);

		if (error == OTA_AUTH_ERROR)
		{
			RML_COMM_LogMsg(FuncName, e_ERROR, "Auth Failed");
		} 
		else if (error == OTA_BEGIN_ERROR) 
		{
			RML_COMM_LogMsg(FuncName, e_ERROR, "Begin Failed");
		} 
		else if (error == OTA_CONNECT_ERROR) 
		{
			RML_COMM_LogMsg(FuncName, e_ERROR, "Connect Failed");
		} 
		else if (error == OTA_RECEIVE_ERROR) 
		{
			RML_COMM_LogMsg(FuncName, e_ERROR, "Receive Failed");
		} 
		else if (error == OTA_END_ERROR) 
		{
			RML_COMM_LogMsg(FuncName, e_ERROR, "End Failed");
		}
	});

	ArduinoOTA.begin();
}



void RML_COMM_HandleArduinoOTA()
{
	/* Handle OTA updates */
	ArduinoOTA.handle();
}



void RML_COMM_LED_Init(Adafruit_NeoPixel &LED_Obj, uint8_t Brightness)
{
	LED_Obj.begin();
	LED_Obj.setBrightness(Brightness);
	LED_Obj.clear();
	LED_Obj.show();
}



int8_t RML_COMM_LED_SetLEDColor(Adafruit_NeoPixel &LED_Obj, uint8_t Red, uint8_t Green, uint8_t Blue)
{
	/// Get the number of LEDs
	uint16_t NumLEDs = LED_Obj.numPixels();

	// Error check: Make sure the number of LEDs is valid
	if (NumLEDs == 0)
	{
		return -1;
	}

	// Set the color for the LED
	const uint32_t Color = LED_Obj.Color(Red, Green, Blue);
    LED_Obj.fill(Color, 0, NumLEDs);

	// Show the color on the LEDs
	LED_Obj.show();

	// Wait for a short time to allow the LEDs to update
	vTaskDelay(pdMS_TO_TICKS(1));

	return 0;
}