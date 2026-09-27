# Code - Unexpected Maker - Series[D] Boards 
Code examples, shipping files and helper libraries for Series[D] boards - TinyS3[D], ProS3[D], FeatherS3[D], EdgeS3[D], TinyPICO[D] and TinyC6[D].

Currently includes:

## Arduino
Check the [arduino folder](arduino/) for a link to the Series[D] Helper library and example code for the Arduino IDE, to be installed via the library manager. The helper library supports all Series[D] boards, including TinyPICO[D] and TinyC6[D].

## ESP-IDF
The [esp-idf folder](esp-idf/) contains the `um_seriesd` helper component and an example project, for all Series[D] boards. Requires ESP-IDF v5.3 or later.

Set the target for your board (`esp32s3` for TinyS3[D], ProS3[D], FeatherS3[D] and EdgeS3[D], `esp32` for TinyPICO[D] and `esp32c6` for TinyC6[D]) with `idf.py set-target`, then select your board under `UM Series[D] Helper` in `idf.py menuconfig`.

On the TinyC6[D] the RF switch is on the FXL6408 IO expander (XIO0). `um_begin()` sets up the I2C bus and IO expander, and the `um_iox_*` functions give you access to the other IO expander pins.

## MicroPython
Series[D] support for all Series[D] boards is coming to MicroPython as new board variants - the PR has been submitted but not merged yet.

Until then, TinyS3[D], ProS3[D] and FeatherS3[D] can use the original TinyS3, ProS3 and FeatherS3 builds available at [micropython.org](https://micropython.org/download/?vendor=Unexpected%20Maker).

Helper libraries and example code will be available soon.

## CircuitPython
Shipping files include the CircuitPython helper library and `code.py` for each board, plus a `lib` folder populated with the requirements for the `Adafruit MAX17048` Battery FG IC library.

Shipping files are available for TinyS3[D], ProS3[D], FeatherS3[D], EdgeS3[D], TinyPICO[D] and TinyC6[D]. The TinyC6[D] and EdgeS3[D] `lib` folders also include `fxl6408.py` for their IO expander (on the TinyC6[D] the RF switch is on XIO0, on the EdgeS3[D] XIO0-XIO7 are available as `edges3d.io_expander`), and the TinyPICO[D] `lib` folder includes `neopixel`, as the TinyPICO build doesn't have it built in.

CircuitPython support for EdgeS3[D] (that the EdgeS3[D] ships with) has been PR'd and merged into the CircuitPython repo.

There are no Series[D] board specific CircuitPython builds for TinyS3[D], ProS3[D], FeatherS3[D], TinyPICO[D] and TinyC6[D] - They use their original non Series[D] board CircuitPython builds.

Due to this, not every IO used on a Series[D] board is available in the `board` import, but all IO can be accessed using `microcontroller.pin`, for example, to access IO11 on the ProS3 build running on the ProS3[D], you use the following syntax:

``` python
# Setup the RF switch pin
ant_selection = DigitalInOut(microcontroller.pin.GPIO11)
ant_selection.direction = Direction.OUTPUT
```

## PlatformIO
The Arduino helper library can also be used with PlatformIO, see the platformio platform information at https://unexpectedmaker.com/develop.php?platform=platformio#platformio-setup

You can find out more about my Series[D] boards at https://unexpectedmaker.com#series-d

Please refer to the included license. 
