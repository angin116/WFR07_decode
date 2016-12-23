#!/usr/bin/python3

import serial
import time

gr3e = serial.Serial('/dev/ttyUSB0', baudrate = 115200, bytesize = 8, parity = 'N', stopbits = 1, timeout = 1)

while True:
    time.sleep(0.1)
    gr3e.flushInput()
    getdata = list(gr3e.read(16))
    datafinal = getdata[getdata.index(1):getdata.index(1)+8]
    print(datafinal)
    
      
