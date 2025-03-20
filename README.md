# Remal CommonUtils Library

## Overview
The **Remal Common Utilities** library provides a set of essential tools and utilities used by Remal developers across various hardware platforms. The library includes a portable logging mechanism compatible with multiple microcontrollers (MCUs). If a specific MCU is not supported, the logger defaults to using the system's native `printf()` function. Additionally, the library provides a set of utilities that are commonly used in projects.

## Features
- **Cross-platform Logging**: Supports logging on various MCUs, with a fallback to `printf()` on unsupported systems.
- **Assert Handling**: Customizable assert function to handle errors with detailed file and line number reporting.
- **Lightweight `printf()` Implementation**: Optimized for embedded systems, reducing overhead while maintaining functionality.
- **String Conversion Utilities**: Functions to convert integers and floating-point numbers to strings, with support for various bases and precision.

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
    GenericUART_Struct logger = { .RX_Pin = 0, .TX_Pin = 0, .BaudRate = 115200 };
    RML_COMM_LoggerInit(&logger);
}

void loop() 
{
    RML_COMM_LogMsg("main", e_INFO, "Logging example message.");
}
```

## Contributing
We welcome contributions! If you wish to contribute, please submit a pull request with a clear description of your changes.

## Changelog
### v1.1.0:
- Fixed bug in `RML_COMM_ftoa()` function affecting precision
- Updated `RML_COMM_vprintf()` to support double precision floating-point numbers up to 15 decimal places from 6
- Updated `Logger_FullDemo.ino` example to demonstrate double precision floating-point numbers
- Added new example `Assert_Example.ino` to demonstrate assert handling using `RML_ASSERT()`

### v1.0.0 - Git Release:
- Initial release to GitHub with full Arduino IDE support and examples.