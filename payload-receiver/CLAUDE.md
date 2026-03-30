# CLAUDE.md

This file provides guidance to Claude Code when working with this repository.

## Commands

```bash
pio run                  # build
pio run -t upload        # flash to board
pio device monitor       # open serial monitor at 9700 baud
pio run -t clean         # clean build artifacts
```

## Architecture

Single-file Arduino sketch (`src/main.cpp`) targeting Arduino Uno R3 (ATmega328P) via PlatformIO.

**Pin assignments:**
- `LORA_SS_PIN 10` — LoRa shield chip select
- `LORA_RESET_PIN 9` — LoRa shield reset
- `LORA_DIO0_PIN 2` — LoRa shield DIO0

**Execution flow:**
- `setup()` — initialises USB serial and LoRa in receive mode
- `loop()` — polls for incoming LoRa packets, prints packet content and RSSI to serial

**Key functions:**
- `setupSerial()` — initialises `Serial` at 9700 baud
- `setupLoRa()` — configures pins, starts LoRa at 915 MHz, enters receive mode

## LoRa

- Module: Duinotech XC4392 (SX1276-based shield)
- Role: receiver only
- Frequency: 915 MHz (Australia)
- Expected packet format: `LAT:<v>,LON:<v>,ALT:<v>,TMP:<v>,HUM:<v>,PRS:<v>`
