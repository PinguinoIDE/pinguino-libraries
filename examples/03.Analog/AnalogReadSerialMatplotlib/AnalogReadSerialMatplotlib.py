# -*- coding: utf-8 -*-

"""/*-------------------------------------------------------------------
    Pinguino example to graph analog signal read on pin 13 of a Pinguino 
    with the matplotlib library through serial communication using the 
    pynguino communication library and CDC_8bit.pde from Yeison Cardona. 

    author  Angel Urquia 
    email   urquia22@gmail.com
    --------------------------------------------------------------------
    You will need :
    1) CDC_8bit.pde running on a Pinguino (should be in the same folder) 
    2) Pynguino Python Module (pip install pynguino)
    3) Matplotlib Python Module (pip install matplotlib)
    ---------------------------------------------------------------*/"""

import numpy as np                      # import necessary libraries
import matplotlib.pyplot as plt
from collections import deque
from pynguino import PynguinoCDC        # import Pynguino CDC from pynguino library

pinguino = PynguinoCDC()                # start communication via serial with pinguino
Leer = 13                               # asignamos a la variable Leer = 13
pinguino.pinMode(Leer, "INPUT")         # pin 13 as input

onda = deque([0]*10 , maxlen=10)        # onda = deque with maximum length "10"
y = pinguino.analogRead(Leer, float)    # assign to variable y = analog reading on pin 13

fig, ax = plt.subplots()                # create the figure                   
linea, = ax.plot(onda)                  # draw the figure

while True:
    x = np.linspace(0, 10, 100)         # 100 values ​​evenly spaced from 0 to 10
    y = (pinguino.analogRead(Leer, float))/40  # value of the analog reading / 40
    onda.append(y)                      # add "y" to onda 
    linea.set_ydata(onda)               # update data from "y"
    ax.set_ylim(-10, 40)                # fixed limits of "y"
    plt.pause(0.001)                    # Pause the graphing and store values
plt.show()                              # show the graph
