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

Single-file Arduino sketch (`src/main.cpp`) targeting Arduino Uno R4 Minima (Renesas RA4M1) via PlatformIO.

**Pin assignments:**
- `GPS_LED LED_BUILTIN` — built-in LED used as GPS activity indicator
- `CAMERA_PIN 7` — Arducam Mega chip select
- `SD_CARD_PIN 9` — SD card chip select

**Execution flow:**
- `setup()` — initialises serial, camera, SD card, BME280 sensor; prints free RAM after each step
- `loop()` — calls `captureImage()`, prints BME280 readings, delays 5 seconds

**Key functions:**
- `captureImage()` — opens file, triggers `takePicture`, calls `saveImage`
- `saveImage()` — drains camera FIFO via `readBuff` in 255-byte chunks, writes to SD
- `openImageFile()` — generates filename from `millis() - start`, opens file for write
- `setupSerial()` / `setupSD()` / `setupCamera()` / `setupBME280()` — single-responsibility helpers

## Known constraints

- `readBuff` max transfer size is 255 bytes (hardware SPI limit)
- Two SPI devices share the bus (camera + SD); CS pins must be HIGH when not in use
- GPS (TinyGPSPlus + SoftwareSerial) is present as a dependency but all GPS code is commented out
