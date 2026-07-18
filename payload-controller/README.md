# Weather Balloon

Arduino MKR WAN 1310 (SAMD21) payload for a high-altitude balloon launch.

## Hardware

| Component | Pin |
|---|---|
| Arducam Mega (CS) | 6 |
| SD Card module (CS) | 4 |
| U-blox NEO-6M GPS (TX → RX1) | 0 |
| LoRa | onboard Murata CMWX1ZZABZ, dedicated SPI1 bus — no CS/RESET/DIO0 to wire |

Camera and SD card share the board's main hardware SPI bus (distinct CS pins above, MOSI/MISO/SCK fixed by the board's SPI header).

**I2C** (BME280 atmospheric sensor)

| Signal | Pin | Color |
|---|---|---|
| SCL | SCL | Green |
| SDA | SDA | Purple |

## What it does

On boot, initialises serial (USB + Serial1 for GPS), LoRa, SD card, camera, and BME280 atmospheric sensor. Then loops every 10 seconds: reads GPS, builds a telemetry string, logs it to the SD card unconditionally. If GPS has a fix, it also captures a 5MP JPEG photo (2592x1944) to the SD card and transmits the telemetry over LoRa; otherwise it just prints that it's still waiting for a fix.

Images are saved under `pics/` as `HHMMSS.jpg`, where the timestamp is GPS UTC time converted to AEST (UTC+10).

### SD card filename limit

The SD card uses the classic `arduino-libraries/SD` library, which only supports 8.3 short filenames (8-char basename + 3-char extension, no long-filename support). Every directory and file name on the card must fit that — a longer name doesn't raise an error, it just fails silently (`SD.mkdir()`/`SD.open()` return `false`).

## LoRa transmission

Transmits at 915 MHz (Australia) via the board's onboard Murata CMWX1ZZABZ (SX1276-based) module, transmitter mode only, sent whenever GPS has a fix. Packet format:

```
TS:<epoch>,LAT:<v>,LON:<v>,ALT:<v>,TMP:<v>,HUM:<v>,PRS:<v>
```

## Build & flash

Uses [PlatformIO](https://platformio.org/) targeting Arduino MKR WAN 1310 (SAMD21).

```bash
pio run              # build
pio run -t upload    # flash to board
pio device monitor   # serial output at 9600 baud
```

## Dependencies

- [Arducam_Mega](https://github.com/tarciosaraiva/Arducam_Mega/tree/samd21-support) — fork of [ArduCAM/Arducam_Mega](https://github.com/ArduCAM/Arducam_Mega), patched to add SAMD21 to `Platform.h`'s HAL dispatch (upstream doesn't support it, at either v2.0.9 or v3.0.0 — see CLAUDE.md)
- [Adafruit BME280 Library](https://github.com/adafruit/Adafruit_BME280_Library)
- [LoRa](https://github.com/sandeepmistry/arduino-LoRa)
- [SD](https://docs.arduino.cc/libraries/sd/) (^1.3.0)
- [paulstoffregen/Time](https://github.com/PaulStoffregen/Time) (^1.6.1)
- GPSParser (local library — `lib/GPSParser/`)
