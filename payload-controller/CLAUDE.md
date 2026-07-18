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

Single-file Arduino sketch (`src/main.cpp`) targeting Arduino MKR WAN 1310 (SAMD21) via PlatformIO.

**Pin assignments:**
- `CAMERA_CS 6` — Arducam Mega chip select
- `SDCARD_CS 4` — SD card chip select
- LoRa is the onboard module, wired internally on a dedicated SPI1 bus; no CS/RESET/DIO0 pins to assign
- `Serial1` (pins 0/1) — hardware UART connected to U-blox NEO-6M GPS (9600 baud, RX only)
- Camera and SD card share the board's main hardware SPI bus (distinct CS pins) — see shared-bus caveat below

**Execution flow:**
- `setup()` — in order: `setupSerial()`, set `SDCARD_CS` HIGH, `setupLoRa()`, `setupSD()`, construct `Arducam_Mega` on the heap (stored as `myCAM` pointer), 200 ms delay, `setupCamera()` (sets `CAMERA_CS` HIGH, then loops `myCAM->begin()`), `setupBME280()`.
- `loop()` — reads GPS, builds telemetry message, logs it to SD unconditionally, then if `has_fix`: prints "Fix acquired!", captures an image, and transmits telemetry over LoRa; otherwise prints "No fix yet, retrying...". Delays 10 seconds.

**Key functions:**
- `captureImage()` — opens file, triggers `takePicture`, calls `saveImage`
- `saveImage()` — drains camera FIFO via `readBuff` in 255-byte chunks, writes to SD
- `openImageFile()` — calls `buildFilename()` to derive the filename from GPS time, opens file for write
- `logTelemetry(msg)` — appends a telemetry string to `obs/data.txt` on the SD card; called every loop regardless of fix status
- `buildTelemetryMessage(gps_data)` — builds a CSV string: `TS:<epoch>,LAT:<v>,LON:<v>,ALT:<v>,TMP:<v>,HUM:<v>,PRS:<v>`; epoch cast to `uint32_t`
- `transmitData(msg)` — transmits the telemetry string over LoRa; called only when `has_fix`
- `setupSerial()` — initialises `Serial` (USB, 9600) and `Serial1` (GPS, 9600)
- `setupSD()` / `setupCamera()` / `setupBME280()` / `setupLoRa()` — single-responsibility helpers
- `toEpoch(gps_data)` — converts GPS date/time strings to `time_t` using `tmElements_t` + `makeTime()` from TimeLib. Returns `0` if either string is too short.
- `buildFilename(gps_data, name)` — adds a 10-hour AEST offset to the `time_t` from `toEpoch()`, then calls `breakTime()` to decompose the result, ensuring correct month/year rollover. Writes `pics/HHMMSS.jpg` into `name` (basename kept to 6 chars — see 8.3 filename constraint below).

## Camera

- `myCAM` is an `Arducam_Mega*` pointer constructed on the heap in `setup()`; `CAMERA_CS` is set HIGH afterward, inside `setupCamera()`
- Image mode is `CAM_IMAGE_MODE_WQXGA2` (2592x1944, 5MP)
- `myCAM->begin()` returns `CAM_ERR_SUCCESS` (value `0`) on success; the check is `== CAM_ERR_SUCCESS`, not a boolean truth test
- Dependency is a fork, not upstream: `https://github.com/tarciosaraiva/Arducam_Mega.git#samd21-support`. Upstream (`ArduCAM/Arducam_Mega`, both v2.0.9 and v3.0.0) never added SAMD21 to `Platform.h`'s HAL dispatch, so `myCAM->begin()` etc. fail to link on the MKR WAN 1310 with "undefined reference to `arducamSpiCsPinLow`/`arducamSpiCsPinHigh`/`arducamCsOutputMode`". The fork adds `defined(ARDUINO_ARCH_SAMD)` to the existing `ArduinoHal.h` branch in `Platform.h` — that HAL is already generic `SPI.h`/`digitalWrite()`/`pinMode()`, so it works unmodified once selected. If this dependency is ever repointed at upstream, reapply that one-line patch.

## LoRa

- Module: onboard Murata CMWX1ZZABZ (SX1276-based), dedicated SPI1 bus
- Role: transmitter only
- Frequency: 915 MHz (Australia)
- Packet format: `TS:<epoch>,LAT:<v>,LON:<v>,ALT:<v>,TMP:<v>,HUM:<v>,PRS:<v>`
- `setupLoRa()` and `transmitData()` are active; transmission happens only when GPS has a fix
- `setupLoRa()` does not call `LoRa.setPins()` — `sandeepmistry/LoRa` auto-detects `ARDUINO_SAMD_MKRWAN1310` and wires SPI1 + internal CS/RESET/DIO0 itself

## Known constraints

- `readBuff` max transfer size is 255 bytes (hardware SPI limit)
- Camera (`CAMERA_CS` 6) and SD card (`SDCARD_CS` 4) share the board's main hardware SPI bus. The Arducam Mega does not tristate MISO when its CS is high, which can corrupt SD transactions on a shared bus — if intermittent SD read/write failures show up, this is the first thing to check.
- SD card uses `arduino-libraries/SD` (the classic library, wrapping an old SdFat core), which only supports 8.3 short filenames — no VFAT/long-filename support. Every directory and file name must fit an 8-char basename + 3-char extension (e.g. `pics`, `obs`, `data.txt`, `HHMMSS.jpg`). A name that's too long doesn't error — `SdFile::make83Name()` just returns `false`, so `SD.mkdir()`/`SD.open()` silently fail.
- GPS uses the local `GPSParser` library (`lib/GPSParser/`); reads `$GPRMC`, `$GPGGA`, `$GPGSA` sentences from Serial1
- `time_t` width varies by Arduino core. `buildTelemetryMessage()` casts the result of `toEpoch()` to `uint32_t` before appending to an Arduino `String` to keep the overload resolution unambiguous regardless of core.
