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
- `CAMERA_PIN 7` — Arducam Mega chip select
- `SD_CARD_PIN 4` — SD card chip select
- `LORA_SS_PIN 10` — LoRa shield chip select
- `LORA_RESET_PIN 9` — LoRa shield reset
- `LORA_DIO0_PIN 2` — LoRa shield DIO0
- `Serial1` (pins 0/1) — hardware UART connected to U-blox NEO-6M GPS (9600 baud, RX only)

**Execution flow:**
- `setup()` — initialises USB serial and Serial1 (GPS), SD card, camera, BME280 sensor, LoRa
- `loop()` — calls `captureImage()`, prints GPS data, prints BME280 readings, transmits via LoRa, delays 10 seconds

**Key functions:**
- `captureImage()` — opens file, triggers `takePicture`, calls `saveImage`
- `saveImage()` — drains camera FIFO via `readBuff` in 255-byte chunks, writes to SD
- `openImageFile()` — generates filename from `millis() - start`, opens file for write
- `setupSerial()` — initialises `Serial` (USB, 9600) and `Serial1` (GPS, 9600)
- `setupSD()` / `setupCamera()` / `setupBME280()` / `setupLoRa()` — single-responsibility helpers
- `transmitData()` — transmits GPS (lat, lon, alt) and BME280 (temp, humidity, pressure) as CSV over LoRa at 915 MHz

## LoRa

- Module: Duinotech XC4392 (SX1276-based shield)
- Role: transmitter only
- Frequency: 915 MHz (Australia)
- Packet format: `LAT:<v>,LON:<v>,ALT:<v>,TMP:<v>,HUM:<v>,PRS:<v>`

## Known constraints

- `readBuff` max transfer size is 255 bytes (hardware SPI limit)
- Two SPI devices share the bus (camera + SD); CS pins must be HIGH when not in use
- GPS uses the local `GPSParser` library (`lib/GPSParser/`); reads `$GPRMC`, `$GPGGA`, `$GPGSA` sentences from Serial1
