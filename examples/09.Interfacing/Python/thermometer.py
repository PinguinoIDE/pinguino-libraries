#! /usr/bin/python
# -*- coding: utf-8 -*-

#-------------------------------------------------------------------------------
# File: thermometer.pyw
# Creator/Owner - Regis Blanchot rblanchot@gmail.com
# Displays the date, time and temperature
# The temperature is sent by Pinguino device though the USB bus
# 
# Last change: 3/11/17  - Dave Maners dmaners@gmail.com
# 3/1/17 - added date, and time, change colors
# 3/11/17 - remove unecessary leading 0's
#-------------------------------------------------------------------------------

import usb		# requires pyusb available at https://sourceforge.net/projects/pyusb/files/
import sys
import time
import Tkinter	# requires python-tk (apt-get install python-tk)
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
# Pinguino Class by Marin Purgar (marin.purgar@gmail.com)
#-------------------------------------------------------------------------------

class Pinguino():

	VENDOR = 0x04D8
	PRODUCT = 0xFEAA
	CONFIGURATION = 1
	INTERFACE = 0
	ENDPOINT_IN = 0x81
	ENDPOINT_OUT = 0x01

	device = None
	handle = None

	def __init__(self,):
		for bus in usb.busses():
			#print bus
			for dev in bus.devices:
				#print dev
				if dev.idVendor == self.VENDOR and dev.idProduct == self.PRODUCT:
					self.device = dev
					print "Pinguino found!"
		return None

	def open(self):
		if not self.device:
			print >> sys.stderr, "Unable to find device!"
			return None

		self.handle = self.device.open()
		if self.handle:
			try:
				self.handle.releaseInterface()
				# WINDOWS USERS : COMMENT THE NEXT LINE ------
				# self.handle.detachKernelDriver(self.INTERFACE)
				# --------------------------------------------
			except:
				pass
			self.handle.setConfiguration(self.CONFIGURATION)
			self.handle.claimInterface(self.INTERFACE)
		else:
			self.handle = None
		return self.handle

	def close(self):
		try:
			self.handle.releaseInterface()
		except Exception, err:
			print >> sys.stderr, err
		self.handle, self.device = None, None

	def read(self, length, timeout = 0):
		return self.handle.bulkRead(self.ENDPOINT_IN, length, timeout)

	def write(self, buffer, timeout = 0):
		return self.handle.bulkWrite(self.ENDPOINT_OUT, buffer, timeout)

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
	if newtime != curtime:					 # Windows users use ('%#I:%M:%S %p'), this removes the leading zeros
		curtime = newtime
		if timeline:
			Temp18x20.canvas.delete(timeline)			
		timeline = Temp18x20.canvas.create_text(200, 65, text=curtime, font=("Helvetica", "24", "bold"), fill='green')

	# recall every 500ms
	Temp18x20.canvas.after(INTERVAL, update)

	# get date
	curdate = time.strftime("%m/%d/%Y") # Use "%#m/%#d/%Y" for Windows or use ("%#x") for "Saturday, March 11, 2017" format
	if dateline:						  # Use "%-m/%-d/%Y" for Linux, this removes the leading zeros
		Temp18x20.canvas.delete(dateline)
	dateline = Temp18x20.canvas.create_text(200, 25, text=curdate, font=("Helvetica", "24", "bold"), fill='orange')
	#Temp18x20.canvas.after(100000, update)

	

if __name__ == "__main__":
	root = Tkinter.Tk()
	root.title('Temp18x20')
	Temp18x20 = Window(root)
	pinguino = Pinguino()
	if pinguino.open() == None:
		print >> sys.stderr, "Unable to open Pinguino device!"
		sys.exit()
	update()
	root.mainloop()
	pinguino.close()

