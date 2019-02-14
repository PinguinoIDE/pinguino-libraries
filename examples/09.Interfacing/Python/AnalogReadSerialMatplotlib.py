# -*- coding: utf-8 -*-
"""/*	---------------------------------------------------------------------
	Pinguino example to graph analog signal read on pin 13 of a Pinguino 
	with the matplotlib library through serial communication using the 
	pynguino communication library and CDC_8bit.pde from Yeison Cardona 
	to be seen in AnalogReadSerialMatplotlib.py

	author			Angel Urquia 
	email			urquia22@gmail.com
	-----------------------------------------------------------------------
	will need
	-----------------------------------------------------------------------
	1)cdc_8bit.pde
	2)Pynguino
	3)Matplotlib
	----------------------------------------------------------------------*/"""
import numpy as np                             # importamos librerias necesarias // import necessary libraries
import matplotlib.pyplot as plt		      
from collections import deque
from pynguino import PynguinoCDC               # importamos Pynguino CDC de libreria pynguino // import Pynguino CDC from pynguino library

pinguino = PynguinoCDC()		       # iniciamos comunicacion via serial  // start communication via serial with pinguino
Leer = 13             			       # asignamos a la variable Leer = 13   //                          
pinguino.pinMode(Leer, "INPUT")		       # pin 13 como entrada // pin 13 as input	
	       				       
onda = deque([0]*10 , maxlen=10)               # asignamos a onda = deque con longitud máxima  "10" // onda = deque with maximum length "10"
y = pinguino.analogRead(Leer, float)           # y = lectura analogica en el pin 13 // assign to variable y = analog reading on pin 13

fig, ax = plt.subplots()                       # creamos la figura  // create the figure                   
linea, = ax.plot(onda)			       # dibujamos la figura // draw the figure

while True:
    x = np.linspace(0, 10, 100)                # 100 valores espaciados uniformemente de 0 a 10 // 100 values ​​evenly spaced from 0 to 10
    y = (pinguino.analogRead(Leer, float))/40  # valor de la lectura analogica /40  // value of the analog reading / 40
    onda.append(y)                             # agregamos "y" a onda //  add "y" to onda 
    linea.set_ydata(onda)                      # actualiza data de "y" // update data from "y"
    ax.set_ylim(-10, 40)		       # fija limites de "y" // fixed limits of "y"
    plt.pause(0.001)                           # pausa la graficacion y almacena valores // Pause the graphing and store values
plt.show()  				       # Mustrar la grafica // Show the graph
