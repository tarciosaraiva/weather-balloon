# Weather Balloon — Payload Receiver

Arduino Uno R3 ground station for the weather balloon payload. Listens for LoRa telemetry packets sent by [payload-controller](../payload-controller/README.md) and prints them to serial. See the [repo README](../README.md) for the system as a whole.

## Board choice

`board = uno`, `platform = atmelavr` (see [platformio.ini](platformio.ini)) — Arduino Uno R3, ATmega328P.

The receiver only needs to demodulate incoming LoRa packets and print them to serial, so a plain Uno R3 paired with a Duinotech XC4392 LoRa shield is enough — no onboard radio, storage, or camera requirements that would call for a more capable board.

## Hardware

| Component | Pin |
|---|---|
| LoRa Shield XC4392 (SS) | 10 |
| LoRa Shield XC4392 (RESET) | 9 |
| LoRa Shield XC4392 (DIO0) | 2 |

## What it does

On boot, initialises serial and LoRa radio (spreading factor 11, 250 kHz signal bandwidth — matches the controller's high-range settings). Then listens for incoming packets from the payload controller and prints each packet and its RSSI to serial.

Expected packet format:

```
TS:<epoch>,LAT:<v>,LON:<v>,ALT:<v>,TMP:<v>,HUM:<v>,PRS:<v>
```

## Build & flash

Uses [PlatformIO](https://platformio.org/) targeting Arduino Uno R3 (ATmega328P, `board = uno`).

```bash
pio run                  # build
pio run -t upload        # flash to board
pio device monitor       # serial output at 9700 baud
pio run -t clean         # clean build artifacts
```

## Dependencies

- [LoRa](https://github.com/sandeepmistry/arduino-LoRa)
