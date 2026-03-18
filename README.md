# Weather Balloon

Arduino Uno payload for a high-altitude balloon launch.

## Hardware

| Component | Pin |
|---|---|
| Red LED | 9 (PWM) |
| Arducam Mega (CS) | 7 |
| SD Card module (CS) | 10 |

## What it does

On boot, initialises serial, SD card, and camera. Then loops every 10 seconds:
takes a FHD JPEG photo, writes it to the SD card, and turns the LED off.
The LED blinks at 300ms intervals during capture to indicate activity.

Images are saved as `<elapsed_ms>.jpg` (milliseconds since boot).

## Build & flash

Uses [PlatformIO](https://platformio.org/) targeting Arduino Uno (ATmega328P).

```bash
pio run              # build
pio run -t upload    # flash to board
pio device monitor   # serial output at 9600 baud
```

## Dependencies

- [Arducam_Mega](https://github.com/ArduCAM/Arducam_Mega)
- [ezLED](https://github.com/ArduinoGetStarted/ezLED)
- SD (built-in Arduino library)
