# EdgeS3[D] Example Code
# 2026 Seon Rozenblum, Unexpected Maker
#
# Project home:
#   https://edges3.io
#

import time, gc, os
import edges3d

# Say hello
print("\nHello from EdgeS3[D]!")
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
print([hex(addr) for addr in edges3d.i2c.scan()])
print("FXL6408 device ID: {}\n".format(hex(edges3d.io_expander.device_id)))

# Show the battery every 2 seconds
while True:
    print("Battery: {:.2f}V  {:.1f}%".format(
        edges3d.get_bat_voltage(), edges3d.get_state_of_charge()
    ))
    time.sleep(2)
