# RP2040 Mouse Jiggler

A USB mouse jiggler for the Waveshare RP2040 Zero, built with Arduino-Pico,
FreeRTOS, and Adafruit TinyUSB.

## Features

- Enumerates as a standard USB HID mouse with no custom driver required.
- Moves the pointer by one pixel every 60 seconds.
- Chooses up, right, down, or left using the RP2040 hardware random generator.
- Sends an opposite movement report to return the pointer to its original position.
- Runs a full-range RGB breathing effect from brightness 0 to 255 while cycling
  through colours.
- Keeps a USB serial interface available for subsequent firmware updates.

## Hardware

- Waveshare RP2040 Zero
- USB data cable

The onboard WS2812 RGB LED uses GPIO 16.

## Build

[PlatformIO](https://platformio.org/) is required. The included
`platformio.ini` selects the Waveshare RP2040 Zero, the Arduino-Pico core,
Adafruit TinyUSB, FreeRTOS, and the Adafruit NeoPixel dependency.

```sh
pio run -e waveshare_rp2040_zero
```

The generated UF2 is written to:

```text
.pio/build/waveshare_rp2040_zero/firmware.uf2
```

## Deploy

### From a serial port

Replace `COM9` with the port assigned to the board:

```sh
pio run -e waveshare_rp2040_zero -t upload --upload-port COM9
```

### Using BOOTSEL

1. Hold the board's **BOOT** button while connecting it, or reset it into
   BOOTSEL mode.
2. Confirm that the `RPI-RP2` removable drive appears.
3. Copy `firmware.uf2` to that drive.
4. The board will reboot automatically and enumerate as a serial device and
   HID-compliant mouse.

On Windows, UF2 copying is a reliable fallback when `picotool` cannot access
the BOOTSEL interface because a compatible WinUSB driver is not installed.

## Configuration

The main settings are near the top of
`src/mouse_jiggler/mouse_jiggler.ino`:

- `deltaMove`: pointer movement distance, currently one pixel.
- `moveInterval`: delay between movements, currently 60,000 ms.
- `PIN`: onboard RGB LED pin, currently GPIO 16.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE).
