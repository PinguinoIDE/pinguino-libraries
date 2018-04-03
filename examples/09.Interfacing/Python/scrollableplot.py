#! /usr/bin/python
# -*- coding: utf-8 -*-

# ----------------------------------------------------------------------
# scrollableplot.py
# Regis Blanchot
# Test program to try out the graph and pinguino classes
# Draw and scroll a graph
# Data are read from a Pinguino through the USB port
# The Pinguino should run the 09.Interfacing/Python/sinusoid.pde example
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

# time period for generating dada points 
# see sinusoid.pde : delay(100)
DELAY = 50

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
        self.graph = g.ScrollGraph(self, width = 360, height = 200)
        self.graph.grid(row = 0, column = 0)
                
        # add a thick red trace
        self.graph.add_trace('sin100', 100, -100, 'red', size = 3)
        self.p = [] 
        self.x = 0 
        self.y = 0 
          
        # add a quit button
        tk.Button(self,text = 'Quit', width = 15, command = master.destroy) \
        .grid(row = 1, column = 0)

        # start the process of adding data to traces
        self.dodata()
        
    def dodata(self):
        # add data to the trace
        self.p = self.pinguino.read(4)
        self.x = self.pinguino.signed16(self.p[1]*256+self.p[0])
        self.y = self.pinguino.signed16(self.p[3]*256+self.p[2])
        self.graph.scroll('sin100', self.y)
            
        # call this function again after DELAY milliseconds
        self.after(DELAY,self.dodata)

class App(tk.Tk):
    def __init__(self):
        tk.Tk.__init__(self)
        
        self.title('y = sin100(x)')
    
        Mainframe(self).pack()
                     
App().mainloop()
