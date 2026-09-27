# TinyPICO[D] Helper Library
# 2026 Seon Rozenblum, Unexpected Maker
#
# Project home:
#   https://tinypico.com
#

# Import required libraries
import board, microcontroller
import adafruit_max1704x
from digitalio import DigitalInOut, Direction

# The TinyPICO build's board pins are for the original TinyPICO (APA102),
# so the TinyPICO[D] pins are used directly
PIXEL_DATA = microcontroller.pin.GPIO2

i2c = board.I2C()  # uses board.SCL and board.SDA
max17 = adafruit_max1704x.MAX17048(i2c)

# Setup the RF switch pin
ant_selection = DigitalInOut(microcontroller.pin.GPIO12)
ant_selection.direction = Direction.OUTPUT

# Setup the NeoPixel power pin
pixel_power = DigitalInOut(microcontroller.pin.GPIO13)
pixel_power.direction = Direction.OUTPUT

# Setup the VBUS sense pin
vbus_sense = DigitalInOut(microcontroller.pin.GPIO9)
vbus_sense.direction = Direction.INPUT


# Helper functions

def set_antenna_external( state ):
    """Set the RF switch to the external uFL connector."""
    ant_selection.value = state

def set_pixel_power(state):
    """Enable or Disable power to the onboard NeoPixel to either show colour, or to reduce power for deep sleep."""
    global pixel_power
    pixel_power.value = state

def get_battery_voltage():
    """Get the approximate battery voltage."""
    return max17.cell_voltage

def get_battery_percentage():
    """Get the approximate battery percentage."""
    return max17.cell_percent

def get_vbus_present():
    """Detect if VBUS (5V) power source is present"""
    global vbus_sense
    return vbus_sense.value

def rgb_color_wheel(wheel_pos):
    """Color wheel to allow for cycling through the rainbow of RGB colors."""
    wheel_pos = wheel_pos % 255

    if wheel_pos < 85:
        return 255 - wheel_pos * 3, 0, wheel_pos * 3
    elif wheel_pos < 170:
        wheel_pos -= 85
        return 0, wheel_pos * 3, 255 - wheel_pos * 3
    else:
        wheel_pos -= 170
        return wheel_pos * 3, 255 - wheel_pos * 3, 0
