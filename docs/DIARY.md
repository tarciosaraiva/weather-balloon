# Weather Balloon — Project Diary

## 2026-03-18

The project started tonight. The goal: build a payload for a high-altitude balloon that can take photos and record atmospheric data on the way up.

The first working sketch was straightforward — an Arduino Uno running PlatformIO, an Arducam Mega for photos, and an SD card to save them. Every 10 seconds the payload would take a full HD JPEG and write it to the SD card, naming the file after the millisecond timestamp since boot. A small LED blinked during capture to give a visual heartbeat. Nothing fancy, but it captured the core loop: wake, shoot, save, sleep.

---

## 2026-03-19

A small but important fix today. The filename generation had a bug — corrected the `sprintf` format so filenames come out correctly from `millis()`.

---

## 2026-03-24

A busy day of upgrades. The classic Arduino Uno was swapped out for the **Uno R4 Minima** (Renesas RA4M1). The motivations were twofold: more processing headroom, and proper hardware UART support for GPS — the classic Uno's two serial pins are shared with USB, which made GPS wiring awkward.

Several things changed in one go:

- The PlatformIO build target switched from `atmelavr/uno` to `renesas-ra/uno_r4_minima`.
- The `ezLED` library was dropped in favour of the **Adafruit BME280** library, adding temperature, humidity, and barometric pressure readings to the loop.
- The SD card CS pin was corrected from 10 to 9.
- The AVR-specific `freeRam()` helper was removed — it's incompatible with the RA4M1 architecture.
- `while(!Serial)` was added to `setup()` to wait for the USB CDC connection to come up before continuing.
- The BME280 sensor is read each loop and its values printed over serial alongside the capture timing.

Also added today: a local **GPSParser** library (`lib/GPSParser/`). It's a custom NMEA sentence parser for UART GPS modules, converted from a MicroPython implementation. It reads `$GPRMC`, `$GPGGA`, and `$GPGSA` sentences and exposes fix status, coordinates, altitude, speed, satellite count, and dilution-of-precision values. The library was added but GPS integration in the main sketch was left disabled — the wiring wasn't sorted yet.

Documentation was updated to match: `CLAUDE.md` and `README.md` now reflect the R4 Minima pin assignments and architecture.

---

## 2026-03-25

GPS integration came together today. The module in use is a **U-blox NEO-6M**, wired to the R4 Minima's hardware UART (`Serial1`, pins 0 and 1).

Testing started indoors — the NEO-6M's blue LED was blinking at 1 Hz, which confirmed the module was powered and searching for satellites. No fix indoors is expected; the module needs a clear sky view. For production use the balloon will obviously be outside.

Wiring settled on:

| NEO-6M | Uno R4 Minima |
|--------|---------------|
| VCC    | 5V            |
| GND    | GND           |
| TX     | D0 (RX1)      |
| RX     | not connected (read-only) |

The 3.3 V logic from the NEO-6M TX is read cleanly by the R4 Minima. Since we're only receiving NMEA data and not sending commands back to the module, the GPS RX line was left unconnected.

In code: `Serial1.begin(9600)` in `setupSerial()`, and `GPSReader gps(Serial1)` as a global — fixing the earlier "most vexing parse" bug where `GPSReader gps()` had been a function declaration rather than an object. The loop now reads GPS data each iteration and prints fix status, coordinates, satellite count, time, and date alongside the BME280 atmospheric readings.

All the old commented-out TinyGPS++ and SoftwareSerial code was cleaned out at the same time.
