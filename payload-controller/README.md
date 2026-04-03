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

**Hardware SPI** (camera — Arducam Mega)

| Signal | Pin | Color |
|---|---|---|
| MOSI | 11 | Yellow |
| MISO | 12 | Red |
| SCK | 13 | White |

**Software SPI** (SD card — SdFat SoftSpiDriver)

| Signal | Pin | Color |
|---|---|---|
| MOSI | 5 | Yellow |
| MISO | 6 | Red |
| SCK | 8 | White |

**I2C** (BME280 atmospheric sensor)

| Signal | Pin | Color |
|---|---|---|
| SCL | SCL | Green |
| SDA | SDA | Purple |

## What it does

On boot, initialises serial (USB + Serial1 for GPS), SD card, camera, and BME280 atmospheric sensor. LoRa initialisation (`setupLoRa()`) and transmission (`transmitData()`) are currently commented out. Then loops every 10 seconds: takes a FHD JPEG photo (1920x1080, ~5 seconds capture), writes it to the SD card, prints GPS fix status, coordinates, satellite count, time and date, and prints temperature, humidity, and pressure over serial.

Images are saved under `pictures/` as `YYYYMMDD_HHMMSS.jpg`, where the timestamp is GPS UTC time converted to AEST (UTC+10).

## LoRa transmission

Transmits at 915 MHz (Australia) using a Duinotech XC4392 (SX1276) shield in transmitter mode. `setupLoRa()` and `transmitData()` are currently commented out. Packet format when enabled:

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
- [SdFat by greiman](https://github.com/greiman/SdFat) (^2.3.0)
- [paulstoffregen/Time](https://github.com/PaulStoffregen/Time) (^1.6.1)
- GPSParser (local library — `lib/GPSParser/`)
