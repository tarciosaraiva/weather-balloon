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

**Software SPI pins for SD card (SdFat SoftSpiDriver):**
- MOSI: pin 5
- MISO: pin 6
- SCK: pin 8
- CS: pin 4 (`SDCARD_CS`)

**Execution flow:**
- `setup()` — in order: set all CS pins HIGH, `setupSD()`, construct `Arducam_Mega` on the heap (stored as `myCAM` pointer), 200 ms delay, `setupCamera()`, `setupBME280()`. `setupLoRa()` is currently commented out.
- `loop()` — calls `captureImage()`, prints GPS data, prints BME280 readings, delays 10 seconds. `transmitData()` is currently commented out.

**Key functions:**
- `captureImage()` — opens file, triggers `takePicture`, calls `saveImage`
- `saveImage()` — drains camera FIFO via `readBuff` in 255-byte chunks, writes to SD
- `openImageFile()` — calls `buildFilename()` to derive the filename from GPS time, opens file for write
- `setupSerial()` — initialises `Serial` (USB, 9600) and `Serial1` (GPS, 9600)
- `setupSD()` / `setupCamera()` / `setupBME280()` / `setupLoRa()` — single-responsibility helpers
- `transmitData()` — transmits GPS (lat, lon, alt) and BME280 (temp, humidity, pressure) as CSV over LoRa at 915 MHz
- `toEpoch(gps_data)` — converts GPS date/time strings to `time_t` using `tmElements_t` + `makeTime()` from TimeLib. Returns `0` if either string is too short.
- `buildFilename(gps_data, name)` — adds a 10-hour AEST offset to the `time_t` from `toEpoch()`, then calls `breakTime()` to decompose the result, ensuring correct month/year rollover. Writes `pictures/YYYYMMDD_HHMMSS.jpg` into `name`.

## Camera

- `myCAM` is an `Arducam_Mega*` pointer constructed on the heap in `setup()` after CS pins are set HIGH
- Image mode is `CAM_IMAGE_MODE_FHD` (1920x1080); capture takes approximately 5 seconds
- `myCAM->begin()` returns `CAM_ERR_SUCCESS` (value `0`) on success; the check is `== CAM_ERR_SUCCESS`, not a boolean truth test

## LoRa

- Module: Duinotech XC4392 (SX1276-based shield)
- Role: transmitter only
- Frequency: 915 MHz (Australia)
- Packet format: `LAT:<v>,LON:<v>,ALT:<v>,TMP:<v>,HUM:<v>,PRS:<v>`
- `setupLoRa()` and `transmitData()` are currently commented out

## Known constraints

- `readBuff` max transfer size is 255 bytes (hardware SPI limit)
- Camera uses hardware SPI (pins 11/12/13); SD card uses software SPI (pins 5/6/8) via SdFat's `SoftSpiDriver`. This is necessary because the Arducam Mega does not tristate MISO when its CS is HIGH, which would corrupt SD reads on a shared hardware SPI bus. The `SPI_DRIVER_SELECT=2` build flag in `platformio.ini` enables `SoftSpiDriver` in SdFat.
- GPS uses the local `GPSParser` library (`lib/GPSParser/`); reads `$GPRMC`, `$GPGGA`, `$GPGSA` sentences from Serial1
- On Renesas RA4M1, `time_t` is `long long int`. `buildTelemetryMessage()` casts the result of `toEpoch()` to `uint32_t` before appending to an Arduino `String` to resolve the ambiguous overload.
