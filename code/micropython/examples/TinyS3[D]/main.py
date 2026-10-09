# TinyS3[D] Example Code
# 2026 Seon Rozenblum, Unexpected Maker
#
# Project home:
#   https://tinys3.io
#

import time, gc, os
import neopixel
from machine import Pin
import tinys3d

# Create a NeoPixel instance
# Brightness of 0.3 is ample for the 1010 sized LED
pixel = neopixel.NeoPixel(Pin(Pin.board.RGB_DATA), 1)

# Say hello
print("\nHello from TinyS3[D]!")
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

# Show what is on the I2C bus - expect 0x36 (MAX17048)
print("I2C Scan")
print("---------------------------")
print([hex(addr) for addr in tinys3d.i2c.scan()])
print()

print("Pixel Time!\n")

# Create a colour wheel index int
color_index = 0

# Turn on the power to the NeoPixel
tinys3d.set_pixel_power(True)

# Show the battery every 2 seconds
last_report = time.ticks_ms()

# Rainbow colours on the NeoPixel
while True:
    if time.ticks_diff(time.ticks_ms(), last_report) >= 2000:
        last_report = time.ticks_ms()
        print("Battery: {:.2f}V  {:.1f}%".format(
            tinys3d.get_bat_voltage(), tinys3d.get_state_of_charge()
        ))

    # Get the R,G,B values of the next colour
    r,g,b = tinys3d.rgb_color_wheel( color_index )
    # Set the colour on the NeoPixel
    pixel[0] = ( r, g, b, 0.5)
    pixel.write()
    # Increase the wheel index
    color_index += 1

    # Sleep for 15ms so the colour cycle isn't too fast
    time.sleep(0.015)
