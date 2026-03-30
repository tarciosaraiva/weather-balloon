# Weather Balloon

Arduino Uno R4 Minima payload for a high-altitude balloon launch.

## Hardware

| Component | Pin |
|---|---|
| Arducam Mega (CS) | 7 |
| SD Card module (CS) | 4 |
| U-blox NEO-6M GPS (TX → RX1) | 0 |
| LoRa Shield XC4392 (SS) | 10 |
| LoRa Shield XC4392 (RESET) | 9 |
| LoRa Shield XC4392 (DIO0) | 2 |

| SPI | Pin | Color |
|---|---|---|
| MOSI | 11 | Yellow |
| MISO | 12 | Red |
| SCK | 13 | White |
| SCL | SCL | Green |
| SDA | SDA | Purple |

## What it does

On boot, initialises serial (USB + Serial1 for GPS), SD card, camera, BME280 atmospheric sensor, and LoRa radio. Then loops every 10 seconds:
takes a FHD JPEG photo, writes it to the SD card, prints GPS fix status, coordinates, satellite count, time and date, prints temperature, humidity, and pressure over serial, then transmits GPS and atmospheric data via LoRa.

Images are saved as `<elapsed_ms>.jpg` (milliseconds since boot).

## LoRa transmission

Transmits at 915 MHz (Australia) using a Duinotech XC4392 (SX1276) shield in transmitter mode. Packet format:

```
LAT:<v>,LON:<v>,ALT:<v>,TMP:<v>,HUM:<v>,PRS:<v>
```

## Build & flash

Uses [PlatformIO](https://platformio.org/) targeting Arduino Uno R4 Minima (Renesas RA4M1).

```bash
pio run              # build
pio run -t upload    # flash to board
pio device monitor   # serial output at 9600 baud
```

## Dependencies

- [Arducam_Mega](https://github.com/ArduCAM/Arducam_Mega)
- [Adafruit BME280 Library](https://github.com/adafruit/Adafruit_BME280_Library)
- [LoRa](https://github.com/sandeepmistry/arduino-LoRa)
- GPSParser (local library — `lib/GPSParser/`)
- SD (built-in Arduino library)
