# Unexpected Maker Series[D] Arduino Helper Library

This is the [helper library](https://github.com/UnexpectedMaker/seriesd_arduino_helper) for all the Unexpected Maker [Series[D] boards](https://unexpectedmaker.com/#series-d) - TinyS3[D], ProS3[D], FeatherS3[D], EdgeS3[D], TinyPICO[D] and TinyC6[D]. It can be used in the Arduino IDE or PlatformIO.

Examples can be found in the [examples directory](https://github.com/UnexpectedMaker/seriesd_arduino_helper/tree/main/examples), these can also be loaded from the examples menu in the Arduino IDE.

Examples include switching antenna output, reading the battery, controlling the RGB LED (if one is present on the board) and reading the light sensor (FeatherS3[D]).

## Selecting your Series[D] board

Apart from the EdgeS3[D], the Series[D] boards don't have their own [D] board listing in the Arduino boards list. Select the original board instead:

| Series[D] board | Board to select |
|---|---|
| TinyS3[D] | UM TinyS3 |
| ProS3[D] | UM PROS3 |
| FeatherS3[D] | UM FeatherS3 |
| TinyPICO[D] | UM TinyPICO |
| TinyC6[D] | UM TinyC6 |
| EdgeS3[D] | UM EdgeS3[D] |


## Installation

Please download the library called `UM SeriesD Helper` via the Arduino Library Manager.

## List of functions

```c++

// Initializes all UM Series[D] board peripherals
void begin();

// Set LDO2 on or off
// Only available on the ProS3[D] and FeatherS3[D] 
void setLDO2Power(bool on);

// Set RGB LED power on or off (On ProS3[D] and FeatherS3[D] it sets LDO2 on)
// Not available on the EdgeS3[D] 
void setPixelPower(bool on);

// Set RGB LED color
// Not available on the EdgeS3[D] 
void setPixelColor(uint8_t r, uint8_t g, uint8_t b);
void setPixelColor(uint32_t rgb);

// Set RGB LED brightness
// Not available on the EdgeS3[D] 
void setPixelBrightness(uint8_t brightness);

// Pack r,g,b (0-255) into a 32bit rgb color
static uint32_t color(uint8_t r, uint8_t g, uint8_t b);

// Convert a color wheel angle (0-255) to a 32bit rgb color
static uint32_t colorWheel(uint8_t pos);

// Set the blue LED on or off
// Only available on the FeatherS3[D]
void setBlueLED(bool on);

// Get the light sensor in volts (0-3.3)
// Only available on the FeatherS3[D]
float getLightSensorVoltage();

// Toggle the blue LED
// Only available on the FeatherS3[D]
void toggleBlueLED();

// Get the battery voltage in volts
// This function gets the voltage from the MAX17048
float getBatteryVoltage();

// Detect if VBUS (USB power) is present
// Not available on the EdgeS3[D] 
bool getVbusPresent();

// Set the RF Switch to external antenna
// On the TinyC6[D] the RF Switch is on the FXL6408 IO expander (XIO0)
void setAntennaExternal(bool state);

// FXL6408 IO expander (uses the TwoWire passed to FG_setup, or Wire if begin() is called first)
// Only available on the TinyC6[D]
bool IOX_begin();
void IOX_pinMode(uint8_t pin, uint8_t mode);
void IOX_digitalWrite(uint8_t pin, bool state);
bool IOX_digitalRead(uint8_t pin);
```
