# Importar modulos
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import customtkinter as ctk
import serial
import threading
import time

#Declaracion del tema visual del GUI
ctk.set_appearance_mode("dark")
ctk.set_default_color_theme("blue")

#Inicio del serial
try:
    ser = serial.Serial("/dev/ttyACM2",baudrate=9600, timeout = 1) # ComX en windows, substituir X por el que toque
    SerOpen = True
except:
    print("No se ha podido abrir el puerto serie")
    SerOpen = False

#Variables globales
puntosMAX = 150 #limite puntos que se muestran en la grafica
dibujar = True #Dibuja el gràfico
datosTemp = []
datosHum = []

#Grafica
graf = Figure(figsize=(5,4),dpi=100)
ax = graf.add_subplot(111)

graf.patch.set_facecolor("#242526")

#Funciones
def LeerDatos ():
    global dibujar, datosTemp,datosHum #uso variabes gobales
    while SerOpen:
        try: #controlamos que se reciban bien los datos
            linea = ser.readline().decode('utf-8').strip() #leemos lo recibido
            if ":" in linea: #obtenemos los datos recibidos #comprobamos el formato
                TempHum = linea.split(":")
                Temp = float(TempHum[0])
                Hum = float(TempHum[1])
                print(f"Temp: {Temp} | Hum: {Hum}") #muestra en terminal los datos

                datosTemp.append(Temp) #añade la temperatura a la lista de datos
                datosHum.append(Hum) #añade la humedad a la lista de datos

                # Para tener la lista con los puntos especificados en puntosMAX
                if len(datosTemp) > puntosMAX:
                    datosTemp.pop(0) #elimina el dato mas antiguo
                    datosHum.pop(0) 
        except Exception as e:
            print(f"Error: {e}")
        time.sleep(0.1) #pausa
def ActualizarGrafico():
    global dibujar, datosTemp,datosHum
    if dibujar:
        ax.clear() #limpia el grafico
        ax.set_facecolor("#313D49")
        ax.plot(range(len(datosTemp)),datosTemp,color="r",label="Temperatura") #dibuja Temperatura
        ax.plot(range(len(datosHum)),datosHum,color="b",label="Humedad") #dibuja Humedad
        ax.legend()
        ax.set_title("Datos Temperatura y humedad",color="#FFFFFF")
        ax.set_ylabel("Temperatura y humedad",color="#FFFFFF")
        ax.set_xlabel("Tiempo",color="#FFFFFF")
        ax.set_xlim(0,puntosMAX)
        ax.tick_params(axis='both',which='both',color="#FFFFFF",labelcolor="#FFFFFF")
        for spine in ax.spines.values():
            spine.set_color("#FFFFFF")
        ax.tick_params(axis='x', which='both', length=0, labelbottom=False)
        ax.set_ylim(10, 90)
        canvas.draw() #actualiza la grafica
    window.after(100,ActualizarGrafico) #actualiza cada 100 milisegundos
threading.Thread(target=LeerDatos, daemon = True).start() #El daemon=True indica que son codigos que pueden no considerarse acabados al cerrar el programa de python
                                                          
#Funciones botones
def IniciarClick ():
    global dibujar
    dibujar = True
    ser.write("i".encode('utf-8')) #enviamos el mensaje de iniciar
def PararClick ():
    global dibujar
    dibujar = False
    ser.write("p".encode('utf-8')) #enviamos el mensaje de parar
def ReanudarClick ():
    global dibujar
    dibujar = True
    ser.write("r".encode('utf-8')) #enviamos el mensaje de reanudar

#GUI
window = ctk.CTk()
window.title("ControlSat")
window.geometry("1200x800")
window.grid_rowconfigure(0,weight=0)
window.grid_rowconfigure(1,weight=3)
window.grid_rowconfigure(2,weight=2)
window.grid_columnconfigure(0,weight=1)

#Configuración del titulo
tituloLabel = ctk.CTkLabel(window, text="Sistema satelital",font=("Georgia",22,"italic"))
tituloLabel.grid(row=0,column=0,columnspan=3,padx=5,pady=5,sticky="nsew")

#Marco de la grafica
frame_graf = ctk.CTkFrame(window)
frame_graf.grid(row=1,column=0,padx=5,pady=5,sticky="nsew")
frame_graf.rowconfigure(0,weight=0)
frame_graf.rowconfigure(1,weight=1)
frame_graf.columnconfigure(0,weight=1)
tituloGrafica = ctk.CTkLabel(frame_graf,text="Gráfica de temperatura",font=("Georgia",10,"italic"))
tituloGrafica.grid(row=0,column=0,sticky="ew",padx=15,pady=(8,0))

#Grafico
canvas = FigureCanvasTkAgg(graf,master=frame_graf)
canvas.get_tk_widget().grid(row=1,column=0,columnspan=3,padx=5,pady=(0,5),sticky="nsew")

#Marco de los botones
frame_botones = ctk.CTkFrame(window)
frame_botones.grid(row=2,column=0,padx=20,pady=10,sticky="nsew")
frame_botones.rowconfigure(0,weight=0)
frame_botones.rowconfigure(1,weight=1)
frame_botones.columnconfigure(0,weight=1)
frame_botones.columnconfigure(1,weight=1)
frame_botones.columnconfigure(2,weight=1)
titulobotones = ctk.CTkLabel(frame_botones,text="Controles",font=("Georgia",10,"italic"),anchor="w")
titulobotones.grid(row=0,column=0,columnspan=3,sticky="ew",padx=15,pady=(8,0))

#Botones
InicioBoton = ctk.CTkButton(frame_botones,text="Iniciar",command=IniciarClick, font=("Georgia",10,"italic"),fg_color="#98FB98",hover_color="#7AC77A",text_color="black",)
InicioBoton.grid(row=1,column=0,padx=5,pady=5,sticky="nsew")
PararBoton = ctk.CTkButton(frame_botones,text="Parar",command=PararClick, font=("Georgia",10,"italic"),fg_color="#FF6347",hover_color="#CC4F39",text_color="black",)
PararBoton.grid(row=1,column=1,padx=5,pady=5,sticky="nsew")
ReanudarBoton = ctk.CTkButton(frame_botones,text="Reanudar",command=ReanudarClick, font=("Georgia",10,"italic"),fg_color="#FFC125",hover_color="#CC991E",text_color="black",)
ReanudarBoton.grid(row=1,column=2,padx=5,pady=5,sticky="nsew")

window.after(100,ActualizarGrafico) #llama a la funcion que actualiza el grafico

window.mainloop()

