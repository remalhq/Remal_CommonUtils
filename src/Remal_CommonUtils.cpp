/*
 * Remal_CommonUtils.cpp
 *
 *  # ALL INFO CAN BE FOUND IN THE HEADER FILE #
 */
#include "Remal_CommonUtils.h"


/*********************************************
 * Private Variables
 *********************************************/
static uint8_t Logger_InitDone = 0;					// This flag is set to true when RML_COMM_Log_Init() is called and is successful. Used for error handling
static LogProtocol_Enum CurrentLogProtocol = e_USB;	// Tracks selected logging protocol
static SemaphoreHandle_t LogMutex = nullptr;		// Mutex to protect logging from multiple tasks
static uint8_t ColorLogsEnabled = false;			// Flag to indicate if colored logs are enabled
static BLESerial BT_Device;							// BLE Serial object for BLE logging
static const char DIGITS[] = "ZYXWVUTSRQPONMLKJIHGFEDCBA9876543210123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"; 	// Used for itoa/utoa functions


/*********************************************
 * Assert Callback
 *********************************************/
static AssertCallback_t UserAssertCallback = nullptr;	// User-defined callback for assert failures


/*********************************************
 * WiFi Credentials & Settings Storage
 *********************************************/
static char StoredSSID[33] = {0};				// Max SSID length is 32 + null
static char StoredPassword[65] = {0};			// Max password length is 64 + null
static uint8_t StoredMaxAttempts = 1;			// Default single attempt
static uint32_t StoredRetryDelayMs = 5000;		// Default 5 seconds between retries
static bool StoredRebootOnFailure = false;		// Default: return error, don't reboot

// Internal constant - per-attempt timeout (not user-configurable)
static const uint32_t WIFI_TIMEOUT_MS = 15000;	// 15 seconds per attempt


/*********************************************
 * OTA Background Task
 *********************************************/
static TaskHandle_t OTA_TaskHandle = nullptr;	// Handle for OTA background task

// Forward declaration for OTA background task
static void OTA_BackgroundTask(void* pvParameters);


/*********************************************
 * Function Pointers for logging backend
 *********************************************/
static void (*Main_putc)(char) = nullptr;			// Function pointer for putc function
static void (*Main_puts)(const char*) = nullptr;	// Function pointer for puts function


// --- USB-CDC backend (Arduino Serial) ---
static inline void USB_putc(char c)
{
	Serial.write((uint8_t)c);
}

static inline void USB_puts(const char* s)
{
	if (s)
	{
		Serial.write((const uint8_t*)s, strlen(s));
	}
}

// --- UART backend (ESP-IDF driver) ---
static uart_port_t SelectedUARTPort = UART_NUM_0;			// Used to store the selected UART port for logging, defaults to UART_NUM_0
static inline void UART_putc(char c)
{
	uart_write_bytes(SelectedUARTPort, &c, 1);
}

static inline void UART_puts(const char* s)
{
	if (s)
	{
		uart_write_bytes(SelectedUARTPort, s, strlen(s));
	}
}

// --- BLE backend (Remal_BLE_Serial) ---
static inline void BLE_putc(char c)
{
    if (BT_Device.IsConnected())
    {
        char buf[2] = { c, '\0' };   		// make a valid null-terminated string
        BT_Device.Send_Data(buf);
    }
}

static inline void BLE_puts(const char* s)
{
    if (s && BT_Device.IsConnected())
    {
        BT_Device.Send_Data(s);
    }
}



/*************************************************
 * @brief Log Levels to log:
 * By default, all log messages are enabled.
 *
 * Use RML_COMM_Log_SetLevel() function to enable
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
const char LogLevel_Str[5][10] =
{
	"DEBUG",
	"INFO",
	"WARNING",
	"ERROR",
	"FATAL"
};





int8_t RML_COMM_Log_Init()
{
	/* Default to USB logging */
	return RML_COMM_Log_Init(e_USB);
}



int8_t RML_COMM_Log_Init(uint8_t LoggingProtocol)
{
	/* Error check:
	* Verify the LoggingProtocol is valid for the function */
	if( LoggingProtocol != e_USB )
	{
		return -1;
	}

	/* Error check:
	 * Check if the logger was already init: */
	if( Logger_InitDone )
	{
		return 0;		//Logger was already init, return success
	}

	/*
	 * USB Logging was selected:
	 */
	Serial.begin();							// Native USB does not use baud rate
	Serial.setTxTimeoutMs(0);				// This is used to avoid waiting if the USB is not connected

	/*
	 * Set function pointers for logging backend:
	 */
	Main_putc = &USB_putc;
	Main_puts = &USB_puts;

	/* Create mutex */
	if (LogMutex == NULL)
	{
		LogMutex = xSemaphoreCreateMutex();
	}

	/* Logger was init successfully */
	Logger_InitDone = 1;
	CurrentLogProtocol = e_USB;

	return 0;
}



int8_t RML_COMM_Log_Init(uint8_t LoggingProtocol, uint8_t TX_Pin, uint32_t Baudrate, uart_port_t UART_Num)
{
	/* Error check:
	 * Verify the LoggingProtocol is valid for the function */
	if( LoggingProtocol != e_UART )
	{
		return -1;
	}

	/* Error check:
	 * Check if the logger was already init: */
	if( Logger_InitDone )
	{
		return 0;		//Logger was already init, return success
	}

	/* Error check:
	 * Baudrate set to 0 */
	if( Baudrate == 0 )
	{
		return -1;
	}

	/* Error check:
	 * UART instance is valid and supported by the MCU */
	if( UART_Num >= SOC_UART_NUM )
	{
		return -1;
	}

	/*
	 * UART Logging was selected:
	 */
  	// Configure UART parameters
  	const uart_config_t UARTConfig =
	{
		.baud_rate = Baudrate,
		.data_bits = UART_DATA_8_BITS,
		.parity    = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		.source_clk = UART_SCLK_APB,
	};

	// Apply configuration
	if( uart_param_config(UART_Num, &UARTConfig) != ESP_OK )
	{
		return -1;
	}

	// Set UART pins
	if( uart_set_pin(UART_Num, TX_Pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) != ESP_OK )
	{
		return -1;
	}

	// Install UART driver with RX and TX buffers even though RX is not used for logging but required by the driver
	if( uart_driver_install(UART_Num, /*rx*/256, /*tx*/2048, /*queue*/0, NULL, 0) != ESP_OK )
	{
		return -1;
	}

	SelectedUARTPort = UART_Num;			// Store the selected UART port for logging

	/*
	 * Set function pointers for logging backend:
	 */
	Main_putc = &UART_putc;
	Main_puts = &UART_puts;

	/* Create mutex */
	if (LogMutex == NULL)
	{
		LogMutex = xSemaphoreCreateMutex();
	}

	/* Logger was init successfully */
	Logger_InitDone = 1;
	CurrentLogProtocol = e_UART;

	return 0;
}



int8_t RML_COMM_Log_Init(uint8_t LoggingProtocol, const char* BT_Name)
{
	/* Error check:
	 * Verify the LoggingProtocol is valid for the function */
	if( LoggingProtocol != e_BLE )
	{
		return -1;
	}

	/* Error check:
	 * Check if the logger was already init: */
	if( Logger_InitDone )
	{
		return 0;		//Logger was already init, return success
	}

	/* Error check:
	 * BT_Name is null or length 0 */
	if( BT_Name == nullptr || strlen(BT_Name) == 0 )
	{
		return -1;
	}

	/*
	 * BT Logging was selected:
	 */
	BT_Device.Init(BT_Name);

	/*
	 * Set function pointers for logging backend:
	 */
	Main_putc = &BLE_putc;
	Main_puts = &BLE_puts;

	/* Disable colored logs for BT */
	RML_COMM_Log_EnableColor(0);

	/* Create mutex */
	if (LogMutex == NULL)
	{
		LogMutex = xSemaphoreCreateMutex();
	}

	/* Logger was init successfully */
	Logger_InitDone = 1;
	CurrentLogProtocol = e_BLE;

	return 0;
}



void RML_COMM_Log_Msg(const char *Src, uint8_t LogLvl, const char* Msg, ... )
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
		if(ColorLogsEnabled)
		{
			ColorStr = ANSI_BOLDCYAN;
		}
	}
	else if(LogLvl == e_INFO)
	{
		if(LogLevelsEnable[1] == 0)
		{
			return;
		}

		/* Info is green */
		if(ColorLogsEnabled)
		{
			ColorStr = ANSI_BOLDGREEN;
		}
	}
	else if(LogLvl == e_WARNING)
	{
		if(LogLevelsEnable[2] == 0)
		{
			return;
		}

		/* Warning is yellow */
		if(ColorLogsEnabled)
		{
			ColorStr = ANSI_BOLDYELLOW;
		}
	}
	else if(LogLvl == e_ERROR)
	{
		if(LogLevelsEnable[3] == 0)
		{
			return;
		}

		/* Error is red */
		if(ColorLogsEnabled)
		{

			ColorStr = ANSI_BOLDRED;
		}
	}
	else if(LogLvl == e_FATAL)
	{
		if(LogLevelsEnable[4] == 0)
		{
			return;
		}

		/* Fatal is red on white bg */
		if(ColorLogsEnabled)
		{
			ColorStr = ANSI_REDONWHITEBG;
		}
	}
	else
	{
		/* Log level is unknown */
		LogLvlUnknown = 1;
	}

	if (LogMutex)
	{
		xSemaphoreTake(LogMutex, portMAX_DELAY);
	}

	/* If using BLE, begin buffering */
	if (CurrentLogProtocol == e_BLE)
	{
		BT_Device.Begin_Buffer();
	}

	/* The log message is sent by segments depending on
	 * what needs to be sent or formatting */
	Main_puts(ColorStr);					//Color string
	Main_puts("> [");

	/* Output log level: */
	if( LogLvlUnknown )
	{
		Main_puts("Unknown LogLvl?");
	}
	else
	{
		Main_puts(LogLevel_Str[LogLvl]);	//LogLevel
	}
	Main_puts("] ");


	/* Logs source of log (inception) */
	Main_puts(Src);
	Main_puts(": ");

	/* Logs message */
	va_list VaList;							//Declare Variable-length argument list to store any additional args
	va_start(VaList, Msg);					//Create a list for arguments given after 'Msg'
	RML_COMM_vprintf(Msg, VaList);
	va_end(VaList);							//Clean up the list

	/* Newline */
	if (ColorLogsEnabled)
	{
		Main_puts(ANSI_RESET);
	}
	Main_puts("\r\n");

	/* If using BLE, flush the buffer */
	if (CurrentLogProtocol == e_BLE)
	{
		BT_Device.Flush_Buffer();
	}

	if (LogMutex)
	{
		xSemaphoreGive(LogMutex);
	}
}



int8_t RML_COMM_Log_SetLevel(uint8_t LogLvl, uint8_t Enable)
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



void RML_COMM_Log_EnableColor(uint8_t Enable)
{
	/* Error check: Makes sure the logger was initialized */
	if(!Logger_InitDone)
	{
		return;
	}

	if(Enable)
	{
		ColorLogsEnabled = 1;
	}
	else
	{
		ColorLogsEnabled = 0;
	}
}



int8_t RML_COMM_Debug_LogStackUsage(UBaseType_t TaskStackSize)
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
	RML_COMM_Log_Msg(pcTaskGetTaskName(NULL),
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

	if (LogMutex)
	{
		xSemaphoreTake(LogMutex, portMAX_DELAY);
	}

	/* If using BLE, begin buffering */
	if (CurrentLogProtocol == e_BLE)
	{
		BT_Device.Begin_Buffer();
	}

	va_list VaList;							// Declare Variable-length argument list to store any additional args
	va_start(VaList, InputStr);				// Create a list for arguments given after 'InputStr'

	RML_COMM_vprintf(InputStr, VaList);		// Call the vprintf function to handle the rest

	va_end(VaList);							// Clean up the list

	/* If using BLE, flush the buffer */
	if (CurrentLogProtocol == e_BLE)
	{
		BT_Device.Flush_Buffer();
	}

	if (LogMutex)
	{
		xSemaphoreGive(LogMutex);
	}
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
					Main_puts(StringArg);														//Print string
					InputStr++;																	//Increment to remove the specifier from printing
					break;

				//Character
				case 'c':
					CharArg = va_arg(VaList, int);												//Get the arg, type char (va_arg() needs int for char)
					Main_putc(CharArg);															//Print char
					InputStr++;																	//Increment to remove the specifier from printing
					break;

				//Unsigned int
				case 'u':
					UnsignedArg = va_arg(VaList, uint32_t);										//Get the arg, type unsigned
					RML_COMM_String_utoa(UnsignedArg, IntStr, sizeof(IntStr), 10);				//Convert unsigned int to ascii, base 10
					Main_puts(IntStr);															//Print string
					InputStr++;																	//Increment to remove the specifier from printing
					break;

				//Signed int
				case 'i':
				case 'd':
					SignedArg = va_arg(VaList, int32_t);										//Get the arg, type signed
					RML_COMM_String_itoa(SignedArg, IntStr, sizeof(IntStr), 10);				//Convert signed int to ascii, base 10
					Main_puts(IntStr); 															//Print string
					InputStr++;																	//Increment to remove the specifier from printing
					break;

				//User wants to print a '%'
				case '%':
					Main_putc('%');																//Print char
					InputStr++;																	//Increment to remove the specifier from printing
					break;

				//Hex value
				case 'X':
				case 'x':
					UnsignedArg = va_arg(VaList, uint32_t);										//Get the arg, type unsigned
					RML_COMM_String_utoa(UnsignedArg, IntStr, sizeof(IntStr), 16);				//Convert unsigned int to ascii, base 16
					Main_puts(IntStr);															//Print string
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
						RML_COMM_String_ftoa(DoubleArg, IntStr, sizeof(IntStr), Decimals);
						Main_puts(IntStr);
						InputStr++; // move past 'f'
					}
					else
					{
						// Unknown specifier; print literally
						Main_putc('%');
						Main_putc('.');
						Main_putc(*InputStr);
						InputStr++;
					}
					break;

				//Double/float value with no specified decimal places
				case 'f':
					DoubleArg = va_arg(VaList, double);											//Get the arg, type double
					RML_COMM_String_ftoa(DoubleArg, IntStr, sizeof(IntStr), 2);					//Convert float/double to ascii, 2 decimal places by default
					Main_puts(IntStr);															//Print string
					InputStr++;
					break;

				//End of string reached
				case '\0':
					break;

				//Unknown specifier - just print it in hopes of the user realizing that
				default:
					Main_putc('%');																//Print char
					Main_putc(*InputStr);														//Print char
					InputStr++;
			}
		}

		/* Not a format specifier, print the char: */
		else
		{
			Main_putc(*InputStr);																//Print char
			InputStr++;																			//Move to next char in the string
		}
	}
}



int32_t RML_COMM_String_utoa(uint32_t v, char* ResultBuff, uint32_t ResultBuff_Size, uint8_t Base)
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



int32_t RML_COMM_String_itoa(int32_t Value, char* ResultBuff, uint32_t ResultBuff_Size, uint8_t Base)
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





void RML_COMM_String_Reverse(char* Str, uint32_t Length)
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




int32_t RML_COMM_String_ftoa(double Value, char* ResultBuff, uint32_t BuffSize, uint8_t Afterpoint)
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




void RML_COMM_Assert_SetCallback(AssertCallback_t Callback)
{
	UserAssertCallback = Callback;
}



void _RML_COMM_Assert_Handler(const char* FileName, uint32_t LineNumber, const char* Expression)
{
	/*
	 * Assertion failed, log info
	 */
	RML_COMM_Log_Msg("RML_COMM_ASSERT", e_FATAL, "ASSERTION FAILED:\r\n\t--> File: %s\r\n\t--> Line: %u\r\n\t--> Expr: %s", FileName, LineNumber, Expression);

	/* Call user callback if registered */
	if (UserAssertCallback != nullptr)
	{
		UserAssertCallback(FileName, LineNumber, Expression);
	}

	while(1)
	{
		vTaskDelay(portMAX_DELAY);
	}
}




int8_t RML_COMM_WiFi_Connect(const char* SSID, const char* Password,
							  uint8_t MaxAttempts, uint32_t RetryDelayMs, bool RebootOnFailure)
{
	static const char FuncName[] = "RML_COMM_WiFi_Connect()";

	/* Error check: SSID is not null or empty */
	if (SSID == nullptr || strlen(SSID) == 0)
	{
		RML_COMM_Log_Msg(FuncName, e_ERROR, "SSID is null or empty");
		return -1;
	}

	/* Ensure at least 1 attempt */
	if (MaxAttempts == 0)
	{
		MaxAttempts = 1;
	}
	
	/* Store credentials for Reconnect() */
	strncpy(StoredSSID, SSID, sizeof(StoredSSID) - 1);
	StoredSSID[sizeof(StoredSSID) - 1] = '\0';

	if (Password != nullptr)
	{
		strncpy(StoredPassword, Password, sizeof(StoredPassword) - 1);
		StoredPassword[sizeof(StoredPassword) - 1] = '\0';
	}
	else
	{
		StoredPassword[0] = '\0';
	}

	/* Store retry settings for Reconnect() */
	StoredMaxAttempts = MaxAttempts;
	StoredRetryDelayMs = RetryDelayMs;
	StoredRebootOnFailure = RebootOnFailure;

	/* Retry loop */
	for (uint8_t attempt = 1; attempt <= MaxAttempts; attempt++)
	{
		if (MaxAttempts > 1)
		{
			RML_COMM_Log_Msg(FuncName, e_INFO, "Attempt %u/%u: Connecting to %s...", attempt, MaxAttempts, SSID);
		}
		else
		{
			RML_COMM_Log_Msg(FuncName, e_INFO, "Connecting to %s...", SSID);
		}

		/* Ensure clean state before connecting */
		WiFi.disconnect(true);				/* true = turn off WiFi radio completely */
		vTaskDelay(pdMS_TO_TICKS(500));		/* Give WiFi driver time to fully reset */
		WiFi.mode(WIFI_STA);				/* Set station mode */

		WiFi.begin(SSID, Password);

		/* Wait for connection, checking for success or definitive failure */
		uint32_t startTime = millis();
		while (millis() - startTime < WIFI_TIMEOUT_MS)
		{
			wl_status_t status = WiFi.status();

			/* Success */
			if (status == WL_CONNECTED)
			{
				RML_COMM_Log_Msg(FuncName, e_INFO, "Connected! IP: %s", WiFi.localIP().toString().c_str());
				return 0;
			}

			/* Definitive failures - no point waiting longer */
			if (status == WL_NO_SSID_AVAIL)
			{
				RML_COMM_Log_Msg(FuncName, e_ERROR, "SSID '%s' not found", SSID);
				break;
			}
			if (status == WL_CONNECT_FAILED)
			{
				RML_COMM_Log_Msg(FuncName, e_ERROR, "Connection failed (wrong password?)");
				break;
			}

			vTaskDelay(pdMS_TO_TICKS(100));
		}

		/* If we get here, attempt failed (timeout or definitive failure) */
		if (WiFi.status() != WL_NO_SSID_AVAIL && WiFi.status() != WL_CONNECT_FAILED)
		{
			RML_COMM_Log_Msg(FuncName, e_ERROR, "Connection attempt timed out");
		}

		/* Disconnect before retry to ensure clean state */
		WiFi.disconnect(true);

		/* If not last attempt, wait before retry */
		if (attempt < MaxAttempts)
		{
			RML_COMM_Log_Msg(FuncName, e_WARNING, "Retrying in %u ms...", RetryDelayMs);
			vTaskDelay(pdMS_TO_TICKS(RetryDelayMs));
		}
	}

	/* All attempts exhausted */
	if (RebootOnFailure)
	{
		RML_COMM_Log_Msg(FuncName, e_FATAL, "WiFi connection failed after %u attempts. Board will restart in 3 seconds...", MaxAttempts);
		vTaskDelay(pdMS_TO_TICKS(3000));
		ESP.restart();
		/* Never returns */
	}

	return -1;
}



int8_t RML_COMM_WiFi_Reconnect()
{
	static const char FuncName[] = "RML_COMM_WiFi_Reconnect()";

	if (StoredSSID[0] == '\0')
	{
		RML_COMM_Log_Msg(FuncName, e_ERROR, "No stored credentials - call RML_COMM_WiFi_Connect first");
		return -1;
	}

	/* Retry loop using stored parameters */
	for (uint8_t attempt = 1; attempt <= StoredMaxAttempts; attempt++)
	{
		if (StoredMaxAttempts > 1)
		{
			RML_COMM_Log_Msg(FuncName, e_INFO, "Attempt %u/%u: Reconnecting to %s...", attempt, StoredMaxAttempts, StoredSSID);
		}
		else
		{
			RML_COMM_Log_Msg(FuncName, e_INFO, "Reconnecting to %s...", StoredSSID);
		}

		/* Ensure clean state before connecting */
		WiFi.disconnect(true);				/* true = turn off WiFi radio completely */
		vTaskDelay(pdMS_TO_TICKS(500));		/* Give WiFi driver time to fully reset */
		WiFi.mode(WIFI_STA);				/* Set station mode */

		WiFi.begin(StoredSSID, StoredPassword);

		/* Wait for connection, checking for success or definitive failure */
		uint32_t startTime = millis();
		while (millis() - startTime < WIFI_TIMEOUT_MS)
		{
			wl_status_t status = WiFi.status();

			/* Success */
			if (status == WL_CONNECTED)
			{
				RML_COMM_Log_Msg(FuncName, e_INFO, "Reconnected! IP: %s", WiFi.localIP().toString().c_str());
				return 0;
			}

			/* Definitive failures - no point waiting longer */
			if (status == WL_NO_SSID_AVAIL)
			{
				RML_COMM_Log_Msg(FuncName, e_ERROR, "SSID '%s' not found", StoredSSID);
				break;
			}
			if (status == WL_CONNECT_FAILED)
			{
				RML_COMM_Log_Msg(FuncName, e_ERROR, "Connection failed (wrong password?)");
				break;
			}

			vTaskDelay(pdMS_TO_TICKS(100));
		}

		/* If we get here, attempt failed (timeout or definitive failure) */
		if (WiFi.status() != WL_NO_SSID_AVAIL && WiFi.status() != WL_CONNECT_FAILED)
		{
			RML_COMM_Log_Msg(FuncName, e_ERROR, "Reconnection attempt timed out");
		}

		/* Disconnect before retry to ensure clean state */
		WiFi.disconnect(true);

		/* If not last attempt, wait before retry */
		if (attempt < StoredMaxAttempts)
		{
			RML_COMM_Log_Msg(FuncName, e_WARNING, "Retrying in %u ms...", StoredRetryDelayMs);
			vTaskDelay(pdMS_TO_TICKS(StoredRetryDelayMs));
		}
	}

	/* All attempts exhausted */
	if (StoredRebootOnFailure)
	{
		RML_COMM_Log_Msg(FuncName, e_FATAL, "WiFi reconnection failed after %u attempts. Board will restart in 3 seconds...", StoredMaxAttempts);
		vTaskDelay(pdMS_TO_TICKS(3000));
		ESP.restart();
		/* Never returns */
	}

	return -1;
}



void RML_COMM_WiFi_Disconnect()
{
	static const char FuncName[] = "RML_COMM_WiFi_Disconnect()";
	WiFi.disconnect();
	RML_COMM_Log_Msg(FuncName, e_INFO, "Disconnected from WiFi");
}



bool RML_COMM_WiFi_IsConnected()
{
	return (WiFi.status() == WL_CONNECTED);
}




void RML_COMM_OTA_Setup(const char* Hostname, const char* Password, bool RunInBackground)
{
	static const char FuncName[] = "RML_COMM_OTA_Setup()";

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
		const char* type;

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

		RML_COMM_Log_Msg(FuncName, e_INFO, "Start updating %s", type);
	});

	ArduinoOTA.onEnd([]()
	{
		RML_COMM_Log_Msg(FuncName, e_INFO, "End");
	});

	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
	{
		static uint8_t lastLoggedThreshold = 255;  /* 255 = uninitialized, will log 0% on first call */

		/* Calculate current percentage */
		const uint32_t pct = total ? (uint32_t)(((uint64_t)progress * 100u) / (uint64_t)total) : 0u;

		/* Round down to nearest 10% threshold */
		const uint8_t currentThreshold = (pct / 10) * 10;

		/* Only log when crossing a new 10% threshold */
		if (currentThreshold != lastLoggedThreshold)
		{
			RML_COMM_Log_Msg("RML_COMM_OTA_Setup()", e_INFO, "Progress: %u%%", currentThreshold);
			lastLoggedThreshold = currentThreshold;
		}

		/* Reset tracker when upload completes (so next OTA starts fresh) */
		if (pct >= 100)
		{
			lastLoggedThreshold = 255;
		}
	});

	ArduinoOTA.onError([](ota_error_t error)
	{
		const char* errorMsg;

		switch (error)
		{
			case OTA_AUTH_ERROR:
				errorMsg = "Auth Failed";
				break;

			case OTA_BEGIN_ERROR:
				errorMsg = "Begin Failed";
				break;

			case OTA_CONNECT_ERROR:
				errorMsg = "Connect Failed";
				break;

			case OTA_RECEIVE_ERROR:
				errorMsg = "Receive Failed";
				break;

			case OTA_END_ERROR:
				errorMsg = "End Failed";
				break;

			default:
				errorMsg = "Unknown Error";
				break;
		}

		RML_COMM_Log_Msg(FuncName, e_ERROR, "OTA Error[%u]: %s", error, errorMsg);
	});

	ArduinoOTA.begin();

	/* Spawn background task if requested */
	if (RunInBackground)
	{
		/* Only create task if it doesn't already exist */
		if (OTA_TaskHandle == nullptr)
		{
			xTaskCreate(
				OTA_BackgroundTask,
				"OTA",
				4096,		// Stack size in bytes
				NULL,
				1,			// Low priority - won't interfere with user tasks
				&OTA_TaskHandle
			);

			RML_COMM_Log_Msg(FuncName, e_INFO, "OTA background task started");
		}
	}
}



/**
 * @brief Background task that handles OTA updates automatically.
 *        Runs at low priority (1) and checks for updates every 500ms.
 */
static void OTA_BackgroundTask(void* pvParameters)
{
	(void)pvParameters;		// Unused

	while (1)
	{
		ArduinoOTA.handle();
		vTaskDelay(pdMS_TO_TICKS(500));		// Check every 500ms
	}
}



void RML_COMM_OTA_Handle()
{
	/* Handle OTA updates - only needed if RunInBackground = false */
	ArduinoOTA.handle();
}



void RML_COMM_LED_Init(Adafruit_NeoPixel &LED_Obj, uint8_t Brightness)
{
	LED_Obj.begin();
	LED_Obj.setBrightness(Brightness);
	LED_Obj.clear();
	LED_Obj.show();
}



int8_t RML_COMM_LED_SetColor(Adafruit_NeoPixel &LED_Obj, uint8_t Red, uint8_t Green, uint8_t Blue)
{
	// Get the number of LEDs
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



void RML_COMM_LED_Off(Adafruit_NeoPixel &LED_Obj)
{
	LED_Obj.clear();
	LED_Obj.show();
	vTaskDelay(pdMS_TO_TICKS(1));
}



void RML_COMM_LED_SetBrightness(Adafruit_NeoPixel &LED_Obj, uint8_t Brightness)
{
	LED_Obj.setBrightness(Brightness);
	LED_Obj.show();
	vTaskDelay(pdMS_TO_TICKS(1));
}



int8_t RML_COMM_LED_SetPixels(Adafruit_NeoPixel &LED_Obj, uint16_t StartIndex, uint8_t Red, uint8_t Green, uint8_t Blue, uint16_t Count)
{
	uint16_t NumLEDs = LED_Obj.numPixels();

	// Bounds check
	if (StartIndex >= NumLEDs || Count == 0)
	{
		return -1;
	}

	// Clamp count to available pixels
	if (StartIndex + Count > NumLEDs)
	{
		Count = NumLEDs - StartIndex;
	}

	const uint32_t Color = LED_Obj.Color(Red, Green, Blue);
	LED_Obj.fill(Color, StartIndex, Count);
	LED_Obj.show();
	vTaskDelay(pdMS_TO_TICKS(1));

	return 0;
}
