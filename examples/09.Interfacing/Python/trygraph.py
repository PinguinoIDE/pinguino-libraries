#! /usr/bin/python
# -*- coding: utf-8 -*-

# ----------------------------------------------------------------------
# trygraph.py
# test program to try out the graph class
# written by Roger Woollett
# ----------------------------------------------------------------------

from sys import version_info
if version_info[0] < 3:
    import Tkinter as tk
else:
    import tkinter as tk
    
import graph as g
from math import sin,pi

DELAY = 20  # time period for generating dada points 

class Mainframe(tk.Frame):
    def __init__(self,master,*args,**kwargs):
        tk.Frame.__init__(self,master,*args,**kwargs)
        
        # create the scroll graph 
        self.graph = g.ScrollGraph(self,width = 300,height = 100)
        self.graph.grid(row = 0,column = 0)
                
        # add a thick red sine wave trace
        self.graph.add_trace('sin',1,-1,'red',size = 3)
        self.angle = 0 
          
        # add a thin green saw tooth trace
        self.graph.add_trace('saw',100,0,'green')
        self.saw = 0
        self.inc = 1
               
        # add a quit button
        tk.Button(self,text = 'Quit',width = 15,command = master.destroy) \
        .grid(row = 1,column = 0)

        # start the process of adding data to traces
        self.dodata()
        
    def dodata(self):
        # add data to both traces
        self.graph.scroll('sin',sin(self.angle))
        self.angle += 0.05
        if self.angle >= 2*pi:
            self.angle = 0
            
        self.graph.scroll('saw',self.saw)
        self.saw += self.inc
        if self.saw == 100:
            self.inc = -1
        if self.saw == 0:
            self.inc = 1
        
        # call this function again after DELAY milliseconds
        self.after(DELAY,self.dodata)

class App(tk.Tk):
    def __init__(self):
        tk.Tk.__init__(self)
        
        self.title('Try Graph')
    
        Mainframe(self).pack()
                     
App().mainloop()
