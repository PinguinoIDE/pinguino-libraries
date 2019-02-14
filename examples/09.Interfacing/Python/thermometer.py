#! /usr/bin/python
# -*- coding: utf-8 -*-

#-------------------------------------------------------------------------------
# File: thermometer.py
# Creator/Owner - Regis Blanchot rblanchot@gmail.com
# Displays the date, time and temperature in a Tkinter window
# The temperature is sent by Pinguino device though the USB bus
# 
# 03/01/17 - Dave Maners dmaners@gmail.com - added date, and time, change colors
# 03/11/17 - Dave Maners dmaners@gmail.com - remove unecessary leading 0's
#-------------------------------------------------------------------------------

# requires pyusb (sudo apt-get install python-usb)
import pinguino as p
import sys
import time
# requires python-tk (sudo apt-get install python-tk)
import Tkinter
from Tkinter import *

curtime = ''
curtemp = ''
timeline = ''
templine = ''
dateline = ''
curdate = ''

#-------------------------------------------------------------------------------
# Program window 
#-------------------------------------------------------------------------------

class Window:

    def __init__(self,root):
        self.root = root 
        self.canvas = Tkinter.Canvas(self.root)
        self.canvas.pack()
        self.canvas.config(background='black')
        self.canvas.config(width=400)
        self.canvas.config(height=200)

#-------------------------------------------------------------------------------
# update
#-------------------------------------------------------------------------------

def update():
    INTERVAL = 1000
    global curtime
    global curtemp
    global timeline
    global dateline
    global templine
    global curdate
    deg = ' ' + unichr(176).encode("utf-8") + "F"  # Change the "F" to a "C" for Celsius
    t = ''

    # get temperature
    try:
        t = pinguino.read(4, INTERVAL)
    except usb.USBError as err:
        pass
    if len(t) > 0:
        #print 't =',t	# debug
        if t[0] == 1:
            sign = '-'
        else:
            sign = ''
        newtemp = '%s%3d%s%2d%s' % (sign,t[1], '.', t[1]*256+t[3], deg)
        if newtemp != curtemp:
            curtemp = newtemp
            if templine:
                Temp18x20.canvas.delete(templine)
            templine = Temp18x20.canvas.create_text(200, 150, text=curtemp, font=("Helvetica", "48", "bold"), fill='yellow')
            
    # get time
    newtime = time.strftime('%I:%M:%S %p')  # Linux users use ('%-I:%M:%S %p'), this removes the leading zeros
    if newtime != curtime:                  # Windows users use ('%#I:%M:%S %p'), this removes the leading zeros
        curtime = newtime
        if timeline:
            Temp18x20.canvas.delete(timeline)
        timeline = Temp18x20.canvas.create_text(200, 65, text=curtime, font=("Helvetica", "24", "bold"), fill='green')

    # recall every 500ms
    Temp18x20.canvas.after(INTERVAL, update)

    # get date
    curdate = time.strftime("%m/%d/%Y") # Use "%#m/%#d/%Y" for Windows or use ("%#x") for "Saturday, March 11, 2017" format
    if dateline:                        # Use "%-m/%-d/%Y" for Linux, this removes the leading zeros
        Temp18x20.canvas.delete(dateline)
    dateline = Temp18x20.canvas.create_text(200, 25, text=curdate, font=("Helvetica", "24", "bold"), fill='orange')
    #Temp18x20.canvas.after(100000, update)

if __name__ == "__main__":
    root = Tkinter.Tk()
    root.title('Temp18x20')
    Temp18x20 = Window(root)
    pinguino = p.Pinguino()
    if pinguino.open() == None:
        print >> sys.stderr, "Unable to open Pinguino device!"
        sys.exit()
    update()
    root.mainloop()
    pinguino.close()
