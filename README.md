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
To install the library, download the latest release from the [Releases](ww) page and import it into your Arduino IDE or PlatformIO project.

#### Arduino IDE
If you're using the Arduino IDE, ensure that the USB CDC is enabled for the logger to work. Go to `Tools` > `USB CDC On Boot` and enable it.

#### PlatformIO
For PlatformIO, add the following lines to your `platformio.ini` file:
```ini
build_flags = 
    -DARDUINO_USB_MODE=1
    -DARDUINO_USB_CDC_ON_BOOT=1
```

## v1.0 - Git Release:
- Added code to Git and started tracking changes here
- Updated to match Arduino IDE library structure

## Contributing
If you wish to contribute to this library, please submit a pull request with a clear description of the changes.