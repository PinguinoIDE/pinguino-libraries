#! /usr/bin/python
# -*- coding: utf-8 -*-

# ----------------------------------------------------------------------
# File: sinusoid.py
#
# Get 2 sine waves coordinates from a Pinguino through the USB port
# Display the points with the Matplotlib Python module.
# 
# 03/01/2017 - Regis Blanchot (rblanchot@gmail.com) - first release
# ----------------------------------------------------------------------

import sys
import time

# Pinguino class to open, read and write from/to a Pinguino device
import pinguino as p

# NumPy is the fundamental package for scientific computing with Python
import numpy as np

# Matplotlib is a Python 2D plotting library
from matplotlib import pyplot as plt
from matplotlib import animation

# Deques are a generalization of stacks and queues
# The name is short for “double-ended queue”
#from collections import deque

# ----------------------------------------------------------------------
# Main
# ----------------------------------------------------------------------

# sinusoid.pde uses Pinguino sin100 Trigonometric Integers
# functions that return values in range [-100, 100]
LIMITX = 360
LIMITY = 100

# Initialize figure and axis
fig = plt.figure('y=sin100(x)')
ax = plt.axes(xlim=(0, LIMITX), ylim=(-LIMITY, LIMITY))
line, = ax.plot([], [], linewidth=2)
#x_text = ax.text(0.02, 0.95, '', transform=ax.transAxes)
#y_text = ax.text(0.02, 0.90, '', transform=ax.transAxes)

# Create the animation background (when blit=True)
def init():
    line.set_data([],[])
    #x_text.set_text('')
    #y_text.set_text('')
    #return line, x_text, y_text
    return line,

# Get data from the Pinguino
def animate(i):
    x = []
    y = []
    line.set_data([],[])
    for n in range(LIMITX):
        p = pinguino.read(4)
        px = pinguino.signed16(p[1]*256+p[0])
        py = pinguino.signed16(p[3]*256+p[2])
        #x_text.set_text('x = %d' % px)
        #y_text.set_text('y = %d' % py)
        x.append(px)
        y.append(py)
    line.set_data(x, y)
    #return line, x_text, y_text
    return line,
 
pinguino = p.Pinguino()

if pinguino.open() == None:
    print >> sys.stderr, "No Pinguino found !"
    sys.exit()

print "Pinguino found !"

# func      : the function to call at each frame. The first argument will be the next value in frames.
# init_func : a function used to draw a clear frame.
# frames    : source of data to pass func and each frame of the animation
# interval  : delay between frames in milliseconds. Defaults to 200.
# repeat    : repeat the animation when the sequence of frames is completed. Defaults to True.

# Note : in order to persist, this object must be assigned to a variable. 
anim = animation.FuncAnimation(fig, func=animate,
                                    init_func=init,
                                    frames=LIMITX,
                                    blit=True,
                                    interval=1,
                                    repeat=True)

try:
    plt.show()                      # Show the graph
except KeyboardInterrupt:           # Quit
    plt.close()
    pinguino.close()
    sys.exit()
