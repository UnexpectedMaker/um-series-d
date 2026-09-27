# Basic FXL6408 8-bit I2C GPIO expander library for Unexpected Maker products
# 2026 Seon Rozenblum, Unexpected Maker
#
# Project home:
#   https://unexpectedmaker.com
#

from adafruit_bus_device.i2c_device import I2CDevice


class FXL6408:
    _FXL6408_ADDRESS = 0x43  # 0x44 when ADDR is tied high

    _DEVICE_ID_REGISTER = 0x01
    _IO_DIRECTION_REGISTER = 0x03
    _OUTPUT_STATE_REGISTER = 0x05
    _OUTPUT_HIGH_Z_REGISTER = 0x07
    _INPUT_DEFAULT_STATE_REGISTER = 0x09
    _PULL_ENABLE_REGISTER = 0x0B
    _PULL_SELECT_REGISTER = 0x0D
    _INPUT_STATUS_REGISTER = 0x0F
    _INTERRUPT_MASK_REGISTER = 0x11
    _INTERRUPT_STATUS_REGISTER = 0x13

    _MANUFACTURER_ID = 0b101
    _SW_RESET = 0x01

    IN = 0
    OUT = 1
    PULL_DOWN = 0
    PULL_UP = 1

    def __init__(self, i2c, address=_FXL6408_ADDRESS):
        self.i2c_device = I2CDevice(i2c, address)
        self._buf = bytearray(2)
        # Reading the device ID also clears the reset interrupt flag
        if (self.device_id >> 5) != self._MANUFACTURER_ID:
            raise OSError("FXL6408 not found")

    def _read_register(self, register):
        self._buf[0] = register
        with self.i2c_device as i2c:
            i2c.write_then_readinto(self._buf, self._buf, out_end=1, in_start=1)
        return self._buf[1]

    def _write_register(self, register, value):
        self._buf[0] = register
        self._buf[1] = value & 0xFF
        with self.i2c_device as i2c:
            i2c.write(self._buf)

    def _write_bit(self, register, pin, state):
        value = self._read_register(register)
        if state:
            value |= 1 << pin
        else:
            value &= ~(1 << pin)
        self._write_register(register, value)

    @property
    def device_id(self):
        """The device ID and control register."""
        return self._read_register(self._DEVICE_ID_REGISTER)

    def config(self, pin, mode, pull=None, value=None):
        """
        Configure an expander pin (0-7) as FXL6408.IN or FXL6408.OUT.
        pull can be None, FXL6408.PULL_UP or FXL6408.PULL_DOWN.
        For outputs, value sets the initial level before the pin is driven.
        """
        if pull is None:
            self._write_bit(self._PULL_ENABLE_REGISTER, pin, 0)
        else:
            self._write_bit(self._PULL_SELECT_REGISTER, pin, pull)
            self._write_bit(self._PULL_ENABLE_REGISTER, pin, 1)

        if mode == self.OUT:
            if value is not None:
                self._write_bit(self._OUTPUT_STATE_REGISTER, pin, value)
            self._write_bit(self._IO_DIRECTION_REGISTER, pin, 1)
            self._write_bit(self._OUTPUT_HIGH_Z_REGISTER, pin, 0)
        else:
            self._write_bit(self._OUTPUT_HIGH_Z_REGISTER, pin, 1)
            self._write_bit(self._IO_DIRECTION_REGISTER, pin, 0)

    def value(self, pin, state=None):
        """Read an expander pin, or set its output level if state is given."""
        if state is None:
            return (self._read_register(self._INPUT_STATUS_REGISTER) >> pin) & 1
        self._write_bit(self._OUTPUT_STATE_REGISTER, pin, state)

    def reset(self):
        """Software reset the chip back to its power-on defaults."""
        self._write_register(self._DEVICE_ID_REGISTER, self._SW_RESET)
