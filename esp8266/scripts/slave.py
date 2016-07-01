# I2C Slave Driver for MicroPython on ESP8266
# MIT license; Copyright (c) 2016 Neil Coyle

import esp

class I2C_SLAVE:
    def __init__(self, scl, sda):
        self.scl = scl
        self.sda = sda

    def read(self, no_of_bytes):
        out = esp.slave_readin(self.scl,self.sda,0x35, no_of_bytes)
        return out

