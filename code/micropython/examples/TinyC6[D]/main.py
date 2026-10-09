# TinyC6[D] Shipping Code
# 2026 Seon Rozenblum, Unexpected Maker
#
# Project home:
#   https://tinyc6.io
#

import time, gc, os
import neopixel
from machine import Pin
import tinyc6d

# Create a NeoPixel instance
# Brightness of 0.3 is ample for the 1010 sized LED
pixel = neopixel.NeoPixel(Pin(Pin.board.RGB_DATA), 1)

# Say hello
print("\nHello from TinyC6[D]!")
print("--------------------\n")

# Show available memory
print("Memory Info - gc.mem_free()")
print("---------------------------")
print("{} Bytes\n".format(gc.mem_free()))

flash = os.statvfs('/')
flash_size = flash[0] * flash[2]
flash_free = flash[0] * flash[3]
# Show flash size
print("Flash - os.statvfs('/')")
print("---------------------------")
print("Size: {} Bytes\nFree: {} Bytes\n".format(flash_size, flash_free))

# Show what is on the I2C bus - expect 0x36 (MAX17048) and 0x43 (FXL6408)
print("I2C Scan")
print("---------------------------")
print([hex(addr) for addr in tinyc6d.i2c.scan()])
print("FXL6408 device ID: {}\n".format(hex(tinyc6d.io_expander.device_id)))

print("Pixel Time!\n")

# Create a colour wheel index int
color_index = 0

# Turn on the power to the NeoPixel
tinyc6d.set_pixel_power(True)

# Show the battery every 2 seconds
last_report = time.ticks_ms()

# Rainbow colours on the NeoPixel
while True:
    if time.ticks_diff(time.ticks_ms(), last_report) >= 2000:
        last_report = time.ticks_ms()
        print("Battery: {:.2f}V  {:.1f}%".format(
            tinyc6d.get_bat_voltage(), tinyc6d.get_state_of_charge()
        ))

    # Get the R,G,B values of the next colour
    r,g,b = tinyc6d.rgb_color_wheel( color_index )
    # Set the colour on the NeoPixel
    pixel[0] = ( r, g, b, 0.5)
    pixel.write()
    # Increase the wheel index
    color_index += 1

    # Sleep for 15ms so the colour cycle isn't too fast
    time.sleep(0.015)
