#! /usr/bin/python
# -*- coding: utf-8 -*-

# ----------------------------------------------------------------------
# bmp280.py
# Regis Blanchot
# Test program to try out the graph and pinguino classes
# Draw and scroll a graph
# Data are read from a Pinguino through the USB port
# The Pinguino should run the 09.Interfacing/Python/bmp280.pde example
# Based on Roger Woollett graph class
# Based on Marin Purgar pinguino class
# ----------------------------------------------------------------------

from sys import version_info
if version_info[0] < 3:
    import Tkinter as tk
else:
    import tkinter as tk
    
import graph    as g
import pinguino as p
import array

# time period for generating dada points 
DELAY = 1000

class Mainframe(tk.Frame):
    def __init__(self,master,*args,**kwargs):
        tk.Frame.__init__(self,master,*args,**kwargs)
        
        # search and open the pinguino
        self.pinguino = p.Pinguino()
        if self.pinguino.open() == None:
            print >> sys.stderr, "No Pinguino found !"
            sys.exit()
        else:
            print "Pinguino found !"

        # create the scroll graph 
        self.graph = g.ScrollGraph(self, width = 600, height = 1000)
        self.graph.grid(row = 0, column = 0)
                
        # add a thin red pressure trace
        self.graph.add_trace('Pressure', 900, 1100, 'red', size = 1)
          
        # add a thick blue temperature trace
        self.graph.add_trace('Temperature', -50, 50, 'blue', size = 3)
          
        # add a quit button
        tk.Button(self,text = 'Quit', width = 15, command = master.destroy) \
        .grid(row = 1, column = 0)

        # start the process of adding data to traces
        self.dodata()
        
    def dodata(self):
        # wait for the Pinguino to send the START condition
        self.s = ""
        while self.s != "START":
            self.s = "".join(map(chr, self.pinguino.read(5)))
            print self.s
            
        # get pressure from the Pinguino and add data to the trace
        self.data = self.pinguino.read(4)
        self.p = self.pinguino.signed32(self.data)
        print("P=%d" % (self.p))
        self.graph.scroll('Pressure', self.p/100) #hPa

        # get temperature from the Pinguino and add data to the trace
        self.data = self.pinguino.read(4)
        self.t = self.pinguino.signed32(self.data)
        print("T=%d" % (self.t))
        self.graph.scroll('Temperature', self.t/100)
            
        # call this function again after DELAY milliseconds
        self.after(DELAY, self.dodata)

class App(tk.Tk):
    def __init__(self):
        tk.Tk.__init__(self)
        
        self.title('Pressure / Temperature')
    
        Mainframe(self).pack()
                     
App().mainloop()
