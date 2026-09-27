# TinyC6[D] Helper Library
# 2026 Seon Rozenblum, Unexpected Maker
#
# Project home:
#   https://tinyc6.io
#

# Import required libraries
import board, microcontroller
import adafruit_max1704x
from digitalio import DigitalInOut, Direction
from fxl6408 import FXL6408

# IO expander pin assignments
ANTENNA_SELECT = 0  # High = external uFL, Low = onboard

i2c = board.I2C()  # uses board.SCL and board.SDA
max17 = adafruit_max1704x.MAX17048(i2c)

# The RF switch is on the FXL6408 IO expander - start on the onboard antenna
io_expander = FXL6408(i2c)
io_expander.config(ANTENNA_SELECT, FXL6408.OUT, value=False)

# Setup the NeoPixel power pin
pixel_power = DigitalInOut(board.NEOPIXEL_POWER)
pixel_power.direction = Direction.OUTPUT

# Setup the VBUS sense pin
# board.VBUS_SENSE in the TinyC6 build is for the original TinyC6, so use the pin directly
vbus_sense = DigitalInOut(microcontroller.pin.GPIO5)
vbus_sense.direction = Direction.INPUT


# Helper functions

def set_antenna_external( state ):
    """Set the RF switch to the external uFL connector."""
    io_expander.value(ANTENNA_SELECT, state)

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
