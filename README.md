# Weather Balloon

Aussie high-altitude balloon payload: an airborne controller that logs telemetry and photos, and a ground station that receives the telemetry over LoRa.

## Projects

| Project | Role | Board |
|---|---|---|
| [payload-controller](payload-controller/README.md) | Flies on the balloon. Reads GPS + BME280, logs telemetry to SD, captures photos, transmits telemetry over LoRa. | Arduino MKR WAN 1310 (SAMD21) |
| [payload-receiver](payload-receiver/README.md) | Stays on the ground. Listens for LoRa packets from the controller and prints them to serial. | Arduino Uno R3 (ATmega328P) |

Each is a standalone PlatformIO project — see its own README for hardware wiring, build/flash commands, and dependencies.

## System overview

```
[GPS + BME280 + Camera] → payload-controller (MKR WAN 1310) --LoRa 915MHz--> payload-receiver (Uno R3) → serial
                                    │
                                    └→ SD card (telemetry log + photos)
```

The controller logs every telemetry sample to the SD card regardless of GPS state, but only transmits over LoRa and captures a photo once it has a GPS fix. The receiver is stateless — it just demodulates incoming packets and prints packet + RSSI.

## Board choices

**payload-controller — Arduino MKR WAN 1310** (`board = mkrwan1310`, `platform = atmelsam`, per [platformio.ini](payload-controller/platformio.ini))
Chosen for its onboard Murata CMWX1ZZABZ LoRa module, wired internally on a dedicated SPI1 bus — no separate LoRa shield or extra CS/RESET/DIO0 wiring to fit alongside the camera and SD card, which already share the main SPI bus. The SAMD21 core's extra RAM/flash over an Uno also has headroom for the camera library, SD library, GPS parsing, and Time library running together.

**payload-receiver — Arduino Uno R3** (`board = uno`, `platform = atmelavr`, per [platformio.ini](payload-receiver/platformio.ini))
The ground station only needs to receive LoRa packets and print them to serial, so a plain Uno R3 with a Duinotech XC4392 LoRa shield is sufficient — no onboard radio, camera, or storage requirements to justify a more capable board.

## Docs

- [payload-controller/CLAUDE.md](payload-controller/CLAUDE.md) / [payload-receiver/CLAUDE.md](payload-receiver/CLAUDE.md) — architecture notes for AI-assisted development
- [payload-controller/docs/DIARY.md](payload-controller/docs/DIARY.md) — build/project diary
