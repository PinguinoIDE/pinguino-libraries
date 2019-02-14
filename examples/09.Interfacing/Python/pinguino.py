#! /usr/bin/python
# -*- coding: utf-8 -*-

# ----------------------------------------------------------------------
# pinguino.py
# class to read/write data from/to a Pinguino device through the USB
# written by Marin Purgar (marin.purgar@gmail.com)
# requires pyusb available at https://sourceforge.net/projects/pyusb/files/
# or install it with pip pyusb install
# ----------------------------------------------------------------------

import sys
import time
import usb
import struct
import numpy

# ----------------------------------------------------------------------
# Pinguino 
# ----------------------------------------------------------------------

class Pinguino():

    VENDOR = 0x04D8
    P8  = 0xFEAA
    P32 = 0x003C
    CONFIGURATION = 1
    INTERFACE = 0
    ENDPOINT_IN = 0x81
    ENDPOINT_OUT = 0x01

    device = None
    handle = None
    arch   = None
    
    def __init__(self,):
        for bus in usb.busses():
            #print bus
            for dev in bus.devices:
                #print dev
                if dev.idVendor == self.VENDOR:
                    if dev.idProduct == self.P8:
                        self.device = dev
                        self.arch = 8
                        #print "8-bit Pinguino found!"
                    if dev.idProduct == self.P32:
                        self.device = dev
                        self.arch = 32
                        #print "32-bit Pinguino found!"
        return None

    def open(self):
        if not self.device:
            #print >> sys.stderr, "Unable to find device!"
            return None

        self.handle = self.device.open()
        if self.handle:
            try:
                self.handle.releaseInterface()
                # WINDOWS USERS : COMMENT THE NEXT LINE ------
                self.handle.detachKernelDriver(self.INTERFACE)
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

    def signed8(self, unsigned8):
        if unsigned8 > 127:
            return (unsigned8 - 255)
        else:
            return unsigned8
        #return numpy.int8(struct.unpack("<L", bytearray(unsigned8))[0])

    def unsigned8(self, unsigned8):
        return numpy.uint8(struct.unpack("<L", bytearray(unsigned8))[0])
    
    def signed16(self, unsigned16):
        if unsigned16 > 32767:
            return (unsigned16 - 0xFFFF)
        else:
            return unsigned16
        #return numpy.int16(struct.unpack("<L", bytearray(unsigned16))[0])

    def unsigned16(self, unsigned16):
        return numpy.uint16(struct.unpack("<L", bytearray(unsigned16))[0])

    def signed32(self, unsigned32):
        return numpy.int32(struct.unpack("<L", bytearray(unsigned32))[0])

    def unsigned32(self, unsigned32):
        return numpy.uint32(struct.unpack("<L", bytearray(unsigned32))[0])
