# Remal CommonUtils Library

## Overview
The **Remal Common Utilities** library provides a set of essential tools and utilities used by Remal developers across various hardware platforms. The library includes a portable logging mechanism compatible with multiple microcontrollers (MCUs). If a specific MCU is not supported, the logger defaults to using the system's native `printf()` function. Additionally, the library provides a set of utilities that are commonly used in projects.

**Bleeding edge development is pushed to the main branch and may contain bugs. For the latest stable version, please use the latest release.**


## Features
- **Cross-platform Logging**: Supports logging on various MCUs, with a fallback to `printf()` on unsupported systems.
- **Assert Handling**: Customizable assert function to handle errors with detailed file and line number reporting.
- **Lightweight `printf()` Implementation**: Optimized for embedded systems, reducing overhead while maintaining functionality.
- **String Conversion Utilities**: Functions to convert integers and floating-point numbers to strings, with support for various bases and precision.
- **Arduino OTA Support**: Wrapper functions for Arduino OTA updates, making it easier to implement OTA functionality in your projects.

## Supported Processors
- **Native (PC)**
- **Espressif Systems ESP32** (Remal Shabakah v3.x, v4)
- **STM32 STM32H735xx**
- **STM32 STM32H725xx**

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

## Example Usage (Logger)
```cpp
#include <Remal_CommonUtils.h>

void setup()
{
    GenericUART_Struct logger = { 
                                    .RX_Pin = 0, 
                                    .TX_Pin = 0, 
                                    .BaudRate = 115200 
                                };
    RML_COMM_LoggerInit(&logger);
}

void loop() 
{
    RML_COMM_LogMsg("main", e_INFO, "Logging example message.");
}
```

## Changelog
### v1.4:
- Renamed #define `ENABLE_COLOR_SUPPORT` to `RML_COLORLOG_ENABLE` for clarity and now users can enable/disable colored logs by defining it before including the library
- Fixed bugs and edge cases in `RML_COMM_utoa()` and `RML_COMM_itoa()` and added proper error handling
- Updated logging functions to use `const char*` which safely accept string literals and remove `-Wwrite-strings` warnings
- Replaced the critical-section spinlock when on ESP32 with a FreeRTOS mutex in logging functions. This keeps interrupts enabled, preventing CPU starvation and interrupt-watchdog timeouts during heavy logging while still serializing output
- Added mutex to `RML_COMM_printf()` to avoid interleaved output and race conditions
- XXX

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