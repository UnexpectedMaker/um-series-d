# UM Series[D] MicroPython firmware - 2026-10-05

Series[D] support for MicroPython is current in a PR waiting for merge and inclusion in the next MicroPython release, so while we wait for that, I have provided firmware here built against IDF5.5.5 and the last MP main ranch as at October 5, 2026

Each `.bin` is the full merged image (bootloader + partition table + app).
Replace `PORT` with your serial port, e.g. `/dev/cu.usbmodem101`.

Erase first, then write.
Instrucitons below use esptool v5.

## ESP32-S3 boards (offset 0x0)

Download mode: hold BOOT, press and release RESET, release BOOT.

### TinyS3[D]
    esptool --chip  esp32s3 --port PORT erase-flash
    esptool --chip  esp32s3 --port PORT write-flash 0x0 UM_TINYS3D.bin

### ProS3[D]
    esptool --chip  esp32s3 --port PORT erase-flash
    esptool --chip  esp32s3 --port PORT write-flash 0x0 UM_PROS3D.bin

### FeatherS3[D]
    esptool --chip  esp32s3 --port PORT erase-flash
    esptool --chip  esp32s3 --port PORT write-flash 0x0 UM_FEATHERS3D.bin

### EdgeS3[D]
    esptool --chip  esp32s3 --port PORT erase-flash
    esptool --chip  esp32s3 --port PORT write-flash 0x0 UM_EDGES3D.bin

## ESP32-C6 boards (offset 0x0)

Download mode: hold BOOT while connecting USB.
These boards stay in the bootloader after flashing - press RESET to start.

### TinyC6[D]
    esptool --chip esp32c6 --port PORT  erase-flash
    esptool --chip esp32c6 --port PORT  write-flash 0x0 UM_TINYC6D.bin

## ESP32 boards (offset 0x1000)

### TinyPICO[D]
    esptool --chip esp32 --port PORT erase-flash
    esptool --chip esp32 --port PORT write-flash 0x0x1000 UM_TINYPICOD.bin


## Examples

The `examples` folder has a `main.py` for each Series[D] board. Each one uses the board's helper module (`tinys3d`, `pros3d`, `feathers3d`, `edges3d`, `tinyc6d` or `tinypicod`), which is built into the firmware above.

TinyC6[D] and TinyPICO[D] ship with MicroPython, so their `main.py` is the shipping code. TinyS3[D], ProS3[D], FeatherS3[D] and EdgeS3[D] ship with CircuitPython, so theirs are examples for when you put MicroPython on them.

Every example prints a hello, memory and flash info, and what's on the I2C bus, then cycles the RGB LED through a rainbow and prints the battery voltage and charge every 2 seconds. The TinyC6[D] and EdgeS3[D] also print the FXL6408 IO expander device ID.

The EdgeS3[D] has no RGB LED, so its example does the same without the rainbow.

To use one, copy it to your board as `main.py` and it will run at boot, for example:

`mpremote cp main.py :main.py`
