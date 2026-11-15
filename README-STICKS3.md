# M5Stack StickS3

Build target for the **M5Stack StickS3 (K150)**, with an ESP32-S3, 8MB flash,
8MB octal PSRAM and a 135x240 ST7789P3 display. Rotation 1 exposes the original
240x135 Seeder layout. StickC, StickC Plus and StickC Plus2 use different
hardware and cannot use this target.

The existing green artwork, BIP39 calculations, seed-page order and QR exports
are retained. Display initialization uses M5Unified/M5GFX for the StickS3's
M5PM1 power controller and panel offsets; LilyGO targets continue using the
vendored TFT_eSPI library. M5GFX 0.3.0 and the M5Unified 0.3.0 release commit are
pinned in `platformio.ini`. Their MIT notices are in `THIRD-PARTY-NOTICES.txt`.

## Build and controls

From this repository, with PlatformIO installed:

```sh
pio run -e sticks3
```

The application is `.pio/build/sticks3/firmware.bin`, at offset **0x10000**.
GitHub Actions also prepares `seeder-sticks3-merged.bin`, a complete image for
offset **0x0**, using the same release pipeline as the LilyGO targets.

- **B**, side button / GPIO12: MOVE; next seed page.
- **A**, front button / GPIO11: OK; previous seed page.
- The existing A/OK holds discard a partial capture or leave the Exit page.
- The separate power key keeps the manufacturer's behavior: single press
  powers on or resets; double press powers off.

The application does not initialize Wi-Fi or Bluetooth. M5Unified's serial,
sensor, microphone, speaker and external-accessory initialization are disabled.
This does not remove the ESP32-S3's radio hardware. No sleep or wake behavior
is added by this target.

## Flash and verify

Use a complete release image at **0x0**, or use PlatformIO's upload target for
the split build files. The application alone must not be flashed at 0x0.
For a complete image, esptool 4.9.0 commands are:

```sh
python -m esptool --chip esp32s3 --port PORT --after no_reset write_flash 0x0 seeder-sticks3-merged.bin
python -m esptool --chip esp32s3 --port PORT --before no_reset --after hard_reset --no-stub verify_flash 0x0 seeder-sticks3-merged.bin
```

Replace `PORT` with the serial port, such as `/dev/cu.usbmodem...` on macOS or
`COM3` on Windows. The first command leaves the chip in its ROM download mode.
The second verifies the full image before starting the application.

If connection fails, connect a USB-C data cable, hold the separate power button
until the green LED blinks, then release it to enter download mode.

Arduino initializes NVS before the application starts, and M5GFX can write the
board hint `M5GFX / AUTODETECT = 26`. On a previously tested StickS3 port, ROM
readback matched the bootloader, partition table and application; only this NVS
hint differed from the erased bytes of the original merged image. No seed data
is stored there. Consequently a full merged-image comparison after normal boot
can fail on NVS even when the code is unchanged. Verify the original split
build's immutable segments instead:

```sh
python -m esptool --chip esp32s3 --port PORT --no-stub verify_flash \
  0x0 .pio/build/sticks3/bootloader.bin \
  0x8000 .pio/build/sticks3/partitions.bin \
  0x10000 .pio/build/sticks3/firmware.bin
```

The custom partition table contains only NVS, OTA boot metadata and one
application. It has no filesystem or core-dump partition. This is deliberate:
the precompiled ESP32-S3 SDK enables crash dumps to flash, which must not create
a destination for RAM containing a seed. OTA metadata is bootloader data; the
application does not add OTA updating.

## Validation

Build and geometry checks cover all three targets. Related StickS3 firmware has
been flashed and its code verified through the ROM loader, with display/button
operation observed. That is not complete validation of this exact green build.
Check both 12/24-word coin/dice paths, QR scanning, hold/release cancellation,
battery startup and power-off behavior using public test seeds before real use.
The existing seed-clearing implementation is unchanged; this port does not
claim to prove complete erasure of every library or stack copy.

Hardware and recovery references:

- [Official StickS3 documentation](https://docs.m5stack.com/en/core/StickS3)
- [Official button examples](https://docs.m5stack.com/en/arduino/m5sticks3/button)
