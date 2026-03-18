# CLAUDE.md

This file provides guidance to Claude Code when working with this repository.

## Commands

```bash
pio run                  # build
pio run -t upload        # flash to board
pio device monitor       # open serial monitor at 9600 baud
pio run -t clean         # clean build artifacts
```

## Architecture

Single-file Arduino sketch (`src/main.cpp`) targeting Arduino Uno (ATmega328P) via PlatformIO.

**Pin assignments:**
- `RED_LED_PIN 9` — PWM-capable, driven by ezLED
- `CAM_CS 7` — Arducam Mega chip select
- `SD_CS 10` — SD card chip select (also the hardware SPI SS pin)

**Execution flow:**
- `setup()` — initialises serial, SD card, camera
- `loop()` — calls `captureImage()` with no delay (delay is managed inside the capture flow)

**Key functions:**
- `captureImage()` — opens file, starts blink, triggers `takePicture`, calls `saveImage`
- `saveImage()` — drains camera FIFO via `readBuff` in 255-byte chunks, writes to SD, turns LED off
- `openImageFile()` — generates filename from `millis() - start`, opens file for write
- `setupSerial()` / `setupSD()` / `setupCamera()` — single-responsibility helpers

## Known constraints

- `readBuff` max transfer size is 255 bytes (hardware SPI limit)
- `SD_CS` pinMode/digitalWrite is commented out — SD library handles this via `SD.begin(SD_CS)`
- Two SPI devices share the bus (camera + SD); CS pins must be HIGH when not in use
- `led.loop()` must be called in tight loops for ezLED to drive non-blocking blink/fade
