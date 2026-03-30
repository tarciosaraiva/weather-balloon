# Payload Receiver

Arduino Uno R3 receiver for the weather balloon payload.

## Hardware

| Component | Pin |
|---|---|
| LoRa Shield XC4392 (SS) | 10 |
| LoRa Shield XC4392 (RESET) | 9 |
| LoRa Shield XC4392 (DIO0) | 2 |

## What it does

On boot, initialises serial and LoRa radio. Then listens for incoming packets from the payload controller and prints each packet and its RSSI to serial.

Expected packet format:

```
LAT:<v>,LON:<v>,ALT:<v>,TMP:<v>,HUM:<v>,PRS:<v>
```

## Build & flash

Uses [PlatformIO](https://platformio.org/) targeting Arduino Uno R3 (ATmega328P).

```bash
pio run                  # build
pio run -t upload        # flash to board
pio device monitor       # serial output at 9700 baud
pio run -t clean         # clean build artifacts
```

## Dependencies

- [LoRa](https://github.com/sandeepmistry/arduino-LoRa)
