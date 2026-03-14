# Remal CommonUtils Library

## Overview
The **Remal Common Utilities** library provides a set of essential tools and utilities used by Remal developers across various hardware platforms. The library provides a set of utilities that are commonly used in projects such as logging, assert, LED control and Arduino OTA support.

**Bleeding edge development is pushed to the main branch and may contain bugs. For the latest stable version, please use the latest release.**


## Features
- **Logging (TX)**: Supports logging on Remal Shabakah boards via various protocols with multiple log levels and optional colored output:
    - Protocols:
        - `USB CDC`: Default logging protocol, requires USB CDC to be enabled.
        - `UART`: Logs messages over UART
        - `Bluetooth Low Energy (BLE)`: Logs messages over BLE using the `Remal_BLE_Serial` library.
    - Log Levels:
        - 🟦 `DEBUG`: Cyan
        - 🟩 `INFO`: Green
        - 🟨 `WARNING`: Yellow
        - 🟥 `ERROR`: Red
        - ⬜🟥 `FATAL`: Red on White Background
- **Input (RX)**: Receive data from the active protocol (USB, UART, or BLE). Includes polling, line-based reading with timeout, buffer flushing, and an optional callback for data arrival notifications.
- **Assert Handling**: Customizable assert function to handle errors with detailed file, line number, and expression reporting. Supports user-defined callbacks for custom error handling.
- **WiFi Wrapper Functions**: Simplified WiFi connection management with automatic credential storage, timeout support, and reconnection capabilities.
- **Lightweight `printf()` Implementation**: Optimized for embedded systems, reducing overhead while maintaining functionality.
- **String Conversion Utilities**: Functions to convert integers and floating-point numbers to strings, with support for various bases and precision.
- **Arduino OTA Support**: Wrapper functions for Arduino OTA updates, making it easier to implement OTA functionality in your projects.


## Supported Processors
- **Espressif Systems ESP32** (Remal Shabakah v3.x, v4)


## Dependencies
This library requires the following dependencies to be installed:
- [Remal BLE Serial](https://github.com/remalhq/Remal_BLE_Serial) - Required for BLE logging functionality
- [Adafruit NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel) - Required for LED wrapper functions


## Getting Started
### Installation
To install the library, download the latest release from the [Releases](https://github.com/remalhq/Remal_CommonUtils/releases) page and import it into your Arduino IDE or PlatformIO project.

#### Arduino IDE
1. Download the latest release from the [Releases](https://github.com/remalhq/Remal_CommonUtils/releases) page.
2. Open the Arduino IDE, navigate to `Sketch` > `Include Library` > `Add .ZIP Library...`.
3. Select the downloaded ZIP file and click `Open`.

**Note:** Ensure that the USB CDC is enabled for the logger to function correctly. Go to `Tools` > `USB CDC On Boot` and enable it.

#### PlatformIO (Enabling USB CDC)
To enable the USB CDC feature in PlatformIO, add the following build flags to your `platformio.ini` file:
```ini
build_flags = 
    -DARDUINO_USB_MODE=1
    -DARDUINO_USB_CDC_ON_BOOT=1
```

## Example Usage (Logger - USB CDC)
```cpp
#include <Remal_CommonUtils.h>

void setup()
{
    RML_COMM_Log_Init(e_USB);        // Initialize logger with default settings (all log levels enabled)
    RML_COMM_Log_EnableColor(1);     // Enable colored log output (Make sure your terminal supports ANSI colors, Arduino Serial Monitor does not support colors)
}

void loop()
{
    RML_COMM_Log_Msg("Loop", e_INFO, "Logging example message.");
}
```

## Example Usage (WiFi Connection with Auto-Retry)
```cpp
#include <Remal_CommonUtils.h>

const char* SSID = "Your_SSID";
const char* Password = "Your_Password";

void setup()
{
    RML_COMM_Log_Init(e_USB);

    // Connect to WiFi with retry support:
    // - 3 connection attempts
    // - 5 second delay between attempts
    // - Auto-reboot if all attempts fail
    RML_COMM_WiFi_Connect(SSID, Password, 3, 5000, true);
    // If we reach here, WiFi is connected
}

void loop()
{
    // Check connection and reconnect if needed
    // Reconnect uses the same retry settings from Connect()
    if (!RML_COMM_WiFi_IsConnected())
    {
        RML_COMM_Log_Msg("Loop", e_WARNING, "WiFi lost, reconnecting...");
        RML_COMM_WiFi_Reconnect();  // Will auto-reboot if all attempts fail
    }

    delay(1000);
}
```

## Example Usage (RX Input - USB)
```cpp
#include <Remal_CommonUtils.h>

void setup()
{
    RML_COMM_Log_Init(e_USB);
    RML_COMM_Log_Msg("Main", e_INFO, "Type something in the Serial Monitor!");
}

void loop()
{
    if (RML_COMM_RX_Available() > 0)
    {
        char Buffer[128];
        int32_t BytesRead = RML_COMM_RX_Read(Buffer, sizeof(Buffer));
        if (BytesRead > 0)
        {
            RML_COMM_Log_Msg("RX", e_INFO, "Received: %s", Buffer);
        }
    }
    delay(10);
}
```

## Example Usage (RX Input - ReadUntil with Callback)
```cpp
#include <Remal_CommonUtils.h>

volatile bool DataReady = false;

void OnDataReceived()
{
    DataReady = true;
}

void setup()
{
    RML_COMM_Log_Init(e_USB);
    RML_COMM_RX_SetCallback(OnDataReceived);
}

void loop()
{
    if (DataReady)
    {
        DataReady = false;
        char Line[128];
        int32_t Len = RML_COMM_RX_ReadUntil(Line, sizeof(Line), '\n', 500);
        if (Len > 0)
        {
            RML_COMM_Log_Msg("RX", e_INFO, "Line: %s", Line);
        }
    }
    delay(10);
}
```

## Changelog
### v3.0 (Breaking Changes) - RX Support Added!:
- **Breaking:** UART `RML_COMM_Log_Init()` signature changed — now requires an `RX_Pin` parameter after `TX_Pin`. Pass `-1` to disable RX (TX-only mode)
- Updated `Logger_FullDemo_UART.ino` for new init signature
- Added RX (input) functions for receiving data across all three protocols (USB, UART, BLE):
    - `RML_COMM_RX_Available()` - Check if data is available to read
    - `RML_COMM_RX_Read()` - Read available data into a buffer
    - `RML_COMM_RX_ReadUntil()` - Read until a terminator character with timeout
    - `RML_COMM_RX_Flush()` - Clear pending RX data
    - `RML_COMM_RX_SetCallback()` - Register a callback for data arrival (runs in a background FreeRTOS task)
- Added `RXCallback_t` typedef for RX callback function pointers
- RX uses a separate mutex (`RX_Mutex`) from TX logging (`LogMutex`) so reads don't block log output
- BLE `ReadUntil` uses an internal residual buffer for proper line parsing across discrete BLE messages
- All timeout handling uses FreeRTOS-friendly internal timing for ESP32 compatibility
- Replaced legacy single RX example with protocol-specific examples
- Added protocol-specific RX examples:
    - `RX_Example_USB.ino`
    - `RX_Example_UART.ino`
    - `RX_Example_BluetoothLE.ino`


**Migration (v2.0 → v3.0):**

| v2.0 Call | v3.0 Call |
|-----------|-----------|
| `RML_COMM_Log_Init(e_UART, TX_Pin, Baudrate)` | `RML_COMM_Log_Init(e_UART, TX_Pin, -1, Baudrate)` |
| `RML_COMM_Log_Init(e_UART, TX_Pin, Baudrate, UART_Num)` | `RML_COMM_Log_Init(e_UART, TX_Pin, -1, Baudrate, UART_Num)` |

*Note: USB and BLE init calls are unchanged.*

### v2.0 (Breaking Changes) - BIG Changes!:
- **Breaking:** All functions renamed to `RML_COMM_<Group>_<Function>` convention for better organization and autocomplete. See migration table below.
- **Breaking:** `RML_ASSERT` renamed to `RML_COMM_ASSERT`, `RML_ASSERT_ENABLE` renamed to `RML_COMM_ASSERT_ENABLE`
- Added `RML_COMM_Assert_SetCallback()` to register a callback that executes before halt on assertion failure
- Assert macro now logs the failed expression string in addition to file and line number
- Added WiFi wrapper functions with auto-retry and auto-restart support:
    - `RML_COMM_WiFi_Connect()` - Connect with configurable retry attempts, delay, and optional reboot on failure
    - `RML_COMM_WiFi_Reconnect()` - Reconnect using stored credentials and retry settings
    - `RML_COMM_WiFi_Disconnect()` - Clean disconnect
    - `RML_COMM_WiFi_IsConnected()` - Check connection status
- Added `RunInBackground` parameter to `RML_COMM_OTA_Setup()` (default: true) - OTA now runs in a FreeRTOS background task automatically
- Added new LED functions: `RML_COMM_LED_Off()`, `RML_COMM_LED_SetBrightness()`, `RML_COMM_LED_SetPixels()`
- Added `#include <WiFi.h>` to header
- Created new `WiFi_Example.ino` and `LED_Example.ino` examples
- Updated all existing examples to use new function names

**Migration Table (v1.x → v2.0):**

| Old Name (v1.x) | New Name (v2.0) |
|-----------------|-----------------|
| `RML_COMM_LoggerInit()` | `RML_COMM_Log_Init()` |
| `RML_COMM_LogMsg()` | `RML_COMM_Log_Msg()` |
| `RML_COMM_LogLevelSet()` | `RML_COMM_Log_SetLevel()` |
| `RML_COMM_EnableColorLogs()` | `RML_COMM_Log_EnableColor()` |
| `RML_COMM_LogStackUsage()` | `RML_COMM_Debug_LogStackUsage()` |
| `RML_COMM_utoa()` | `RML_COMM_String_utoa()` |
| `RML_COMM_itoa()` | `RML_COMM_String_itoa()` |
| `RML_COMM_ftoa()` | `RML_COMM_String_ftoa()` |
| `RML_COMM_ReverseString()` | `RML_COMM_String_Reverse()` |
| `RML_COMM_SetupArduinoOTA()` | `RML_COMM_OTA_Setup()` |
| `RML_COMM_HandleArduinoOTA()` | `RML_COMM_OTA_Handle()` |
| `RML_COMM_LED_SetLEDColor()` | `RML_COMM_LED_SetColor()` |

*Note: `RML_COMM_printf()` and `RML_COMM_vprintf()` unchanged.*

### v1.5:
- Updated BLE logging to use the new `Remal_BLE_Serial` library API
- Added BLE buffering support using `Begin_Buffer()` and `Flush_Buffer()` for more efficient BLE transmission
- Removed usage of Arduino `String` class in OTA handler, replaced with `const char*`
- Added `CurrentLogProtocol` tracking variable to track which logging protocol is active
- Minor code cleanup and documentation updates

### v1.4:
- Fixed bugs and edge cases in `RML_COMM_utoa()` and `RML_COMM_itoa()` and added proper error handling
- Updated logging functions to use `const char*` which safely accept string literals and remove `-Wwrite-strings` warnings
- Replaced the critical-section spinlock when on ESP32 with a FreeRTOS mutex in logging functions. This keeps interrupts enabled, preventing CPU starvation and interrupt-watchdog timeouts during heavy logging while still serializing output
- Added mutex to `RML_COMM_printf()` to avoid interleaved output and race conditions
- Removed max baud rate limitation for logger 
- Created new enum `LogProtocol_Enum` to select between different logging protocols (USB, UART, or BLE)
- Removed `STM32` support as it's unused, this library will exclusively support `ESP32` going forward, we might add other MCUs in the future based on board support
- Removed `GenericUART_Struct`
- Removed `CurrentMCU_Enum` and related code as they are no longer needed
- Removed MCU specific defines and includes: `ESP32`, `STM32H725xx`, `STM32H735xx`
- Big rework of logger functions, `RML_COMM_LoggerInit()` now has different overloads to support different logging protocols => USB, UART, BLE
- Removed usage of `PUTCHAR_FUNC` and `PUTCHAR_N_FUNC` defines
- Created new internal functions `Main_putc()` and `Main_puts()` that dynamically get assigned based on the selected logging protocol backend at runtime
- Updated `_RML_COMM_Assert()` to yield instead of busy-waiting in an infinite loop to avoid CPU starvation
- Removed `#define ENABLE_COLOR_SUPPORT` and replaced its use with function call to enable/disable colored logs at runtime: `RML_COMM_EnableColorLogs()`
- Updated log level colors to be bold and fatal log level to be red-on-white background
- Added new ANSI color defines red-on-white background and white-on-red background: `ANSI_WHITEONREDBG` and `ANSI_REDONWHITEBG`
- Updated README.md
- Updated `Assert_Example.ino` to use new logger initialization function
- Removed usage of Arduino `String` class in all examples and replaced with standard C++ `string` class
- Split `Logger_FullDemo.ino` example into three separate examples: `Logger_FullDemo_USB.ino`, `Logger_FullDemo_UART.ino`, and `Logger_FullDemo_BluetoothLE.ino` to demonstrate different logging protocols

### v1.3:
- Created new wrapper functions for the `Adafruit NeoPixel` library to make it simpler to init and use addressable LEDs
- Fixed function names and added `COMM_` prefix
- Added new function `RML_COMM_LogStackUsage()` to easily log current stack usage for debugging purposes

### v1.2:
- Added Arduino OTA wrapper functions: `RML_SetupArduinoOTA()` and `RML_HandleArduinoOTA()`
- Added new example `OTA_Example.ino` to demonstrate OTA functionality

### v1.1.0:
- Fixed bug in `RML_COMM_ftoa()` function affecting precision
- Updated `RML_COMM_vprintf()` to support double precision floating-point numbers up to 15 decimal places from 6
- Updated `Logger_FullDemo.ino` example to demonstrate double precision floating-point numbers
- Added new example `Assert_Example.ino` to demonstrate assert handling using `RML_ASSERT()`

### v1.0.0 - Git Release:
- Initial release to GitHub with full Arduino IDE support and examples.


## Contributing
We welcome contributions! If you wish to contribute, please submit a pull request with a clear description of your changes.
