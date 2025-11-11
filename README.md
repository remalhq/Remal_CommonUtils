# Remal CommonUtils Library

## Overview
The **Remal Common Utilities** library provides a set of essential tools and utilities used by Remal developers across various hardware platforms. The library provides a set of utilities that are commonly used in projects such as logging, assert, LED control and Arduino OTA support.

**Bleeding edge development is pushed to the main branch and may contain bugs. For the latest stable version, please use the latest release.**


## Features
- **Logging**: Supports logging on Remal Shabakah boards via various protocols with multiple log levels and optional colored output: 
    - Protocols:
        - `USB CDC`: Default logging protocol, requires USB CDC to be enabled.
        - `UART`: Logs messages over UART
        - `Bluetooth Low Energy (BLE)`:  <-- Has some issues, use with caution. Will fix in future update.
    - Log Levels:
        - 🟦 `DEBUG`: Cyan
        - 🟩 `INFO`: Green  
        - 🟨 `WARNING`: Yellow  
        - 🟥 `ERROR`: Red  
        - ⬜🟥 `FATAL`: Red on White Background
- **Assert Handling**: Customizable assert function to handle errors with detailed file and line number reporting.
- **Lightweight `printf()` Implementation**: Optimized for embedded systems, reducing overhead while maintaining functionality.
- **String Conversion Utilities**: Functions to convert integers and floating-point numbers to strings, with support for various bases and precision.
- **Arduino OTA Support**: Wrapper functions for Arduino OTA updates, making it easier to implement OTA functionality in your projects.


## Supported Processors
- **Espressif Systems ESP32** (Remal Shabakah v3.x, v4)


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
    RML_COMM_LoggerInit(e_USB);     // Initialize logger with default settings (all log levels enabled)
    RML_COMM_EnableColorLogs(1);    // Enable colored log output (Make sure your terminal supports ANSI colors, Arduino Serial Monitor does not support colors)
}

void loop() 
{
    RML_COMM_LogMsg("main", e_INFO, "Logging example message.");
}
```

## Changelog
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
- Updated `_RML_COMM_Assert()` to use `vTaskDelay(portMAX_DELAY)` instead of a busy-wait infinite loop to avoid CPU starvation
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