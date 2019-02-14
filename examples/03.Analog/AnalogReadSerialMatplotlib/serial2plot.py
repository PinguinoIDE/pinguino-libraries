#! /usr/bin/python
# -*- coding: utf-8 -*-

# ----------------------------------------------------------------------
# Python script to read serial output
# Author: Dave Maners <dmaners@gmail.com>
# Make sure Serial.print("/r/n") gets sent
# as readline() looks for EOL.
# ----------------------------------------------------------------------

from __future__ import print_function
import numpy as np
import matplotlib.pyplot as plt
from collections import deque
import serial
import time
import sys

port = serial.Serial(
    port='/dev/ttyUSB0',
    baudrate=9600,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    timeout=0,
    )

buffer1 = deque([0]*10 , maxlen=10) # buffer1 = deque with maximum length "4"
fig, ax = plt.subplots()            # create the figure                   
linea, = ax.plot(buffer1)           # draw the figure

while True:
    time.sleep(.999)                # sleep for 1 second
    x = np.linspace(0, 100, 1000)   # 100 values ​​evenly spaced from 0 to 10
    y = int(port.readline())        # value of the serial reading
    buffer1.append(y)               # add "y" to buffer1 
    linea.set_ydata(buffer1)        # update data from "y"
    ax.set_ylim(100, 1000)          # fixed limits of "y"
    plt.pause(0.001)                # Pause the graphing and store values

try:
    plt.show()                      # Show the graph
except KeyboardInterrupt:
    # quit
    plt.close()
    sys.exit()
