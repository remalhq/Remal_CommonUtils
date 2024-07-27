# Remal CommonUtils Library
The Remal CommonUtils Library provides custom logger functionality that is portable across multiple microcontroller units (MCUs). The library ensures that if a certain MCU is not supported, the logger will route the output to the native system printf() function. Additionally, the library provides a set of utilities that are commonly used in projects.

Our main focus is to support and enhance the development on our Remal boards.

## Features
- Portable logger across various MCUs.
- Fallback to native printf() for unsupported systems.
- Custom UART initialization for embedded systems.
- Lightweight printf() and vprintf() implementations for embedded systems.
- Supports multiple log levels (DEBUG, INFO, WARNING, ERROR, FATAL).
- String conversion utilities for integers and floating-point numbers.

## Supported Processors
- Native (PC)
- Espressif Systems ESP32 (Remal Shabakah v3.x, v4)
- STM32 STM32F411xE
- [WORKING] STM32 STM32H7 and STM32F4

## v1.0 - Git Release:
- Added code to Git and started tracking changes here