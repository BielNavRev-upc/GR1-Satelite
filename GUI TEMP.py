
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import serial
from tkinter import *
import threading
import time

ser = serial.Serial("/dev/ttyACM0",baudrate=9600, timeout = 1) # ComX en windows, substituir X por el que toque
puntosMAX = 100

dibujar = True
datostemp = []

fig = Figure(figsize=(5, 4), dpi=100)
ax = fig.add_subplot(111)


def LeerDatos ():
    global dibujar, datostemp
    while True:
        try:
            linea = ser.readline().decode('utf-8').strip()
            if ":" in linea:
                TempHum = linea.split(":")
                Temp = float(TempHum[0])
                Hum = float(TempHum[1])
                print(f"Temp: {Temp} | Hum: {Hum}")
                
                datostemp.append(Temp)
                
                # Para tener la lista con los puntos especificados en puntosMAX
                if len(datostemp) > puntosMAX:
                    datostemp.pop(0) 
        except Exception as e:
            print(f"Error: {e}")
        time.sleep(0.1)

def ActualizarGrafico():
    global dibujar, datostemp
    if dibujar and len(datostemp) > 0:
        ax.clear() 
        #ax.plot(posicionx, datostemp)
        ax.plot(range(len(datostemp)),datostemp)
        ax.set_title("Datos Temperatura")
        ax.set_ylabel("Temperatura")
        ax.set_xlabel("Tiempo")
        ax.set_xlim(0,puntosMAX)
        ax.set_ylim(15, 35) 
        canvas.draw() 

    window.after(100,ActualizarGrafico) 
    

threading.Thread(target=LeerDatos, daemon = True).start() #El daemon=True indica que son codigos que pueden no 
                                                          # considerarse acabados al cerrar el programa de python

def IniciarClick ():
    global dibujar
    dibujar = True
    ser.write("i".encode('utf-8'))
def PararClick ():
    global dibujar
    dibujar = False
    ser.write("p".encode('utf-8'))
def ReanudarClick ():
    global dibujar
    dibujar = True
    ser.write("r".encode('utf-8'))

window = Tk()
window.geometry("1200x800")
window.rowconfigure(0, weight=1)
window.rowconfigure(1, weight=2)
window.rowconfigure(2, weight=2)
window.columnconfigure(0, weight=1)
window.columnconfigure(1, weight=1)
window.columnconfigure(2, weight=1)

tituloLabel = Label(window, text = "Mi programa", font=("Courier", 20, "italic"))
tituloLabel.grid(row=0, column=0, columnspan=3, padx=5, pady=5, sticky=N + S + E + W)

#fraseEntry = Entry(window)
#fraseEntry.grid(row=1, column=0, columnspan = 3, padx=5, pady=5, sticky=N + S + E + W)

InicioButton = Button(window, text="Inicio", bg='green', fg="white",command=IniciarClick)
InicioButton.grid(row=2, column=0, padx=5, pady=5, sticky=N + S + E + W)
PararButton = Button(window, text="Parar", bg='red', fg="white", command = PararClick)
PararButton.grid(row=2, column=1, padx=5, pady=5, sticky=N + S + E + W)
ReanudarButton = Button(window, text="Reanudar", bg='yellow', fg="black", command = ReanudarClick)
ReanudarButton.grid(row=2, column=2, padx=5, pady=5, sticky=N + S + E + W)

canvas = FigureCanvasTkAgg(fig, master=window)
canvas.get_tk_widget().grid(row=1, column=0, columnspan=3, padx=10, pady=10, sticky=N + S + E + W)

window.after(100,ActualizarGrafico)

window.mainloop()