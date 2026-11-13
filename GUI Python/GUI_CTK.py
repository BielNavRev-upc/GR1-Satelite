from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import customtkinter as ctk
import serial
import time
import threading 

# Variables globales
puntosMax = 150
datosTemp = []
datosHum = []
DHTStatus = False
ErrorSensorDHT = False
ser = None
serOpen = False
serialPort = "/dev/ttyACM0"
tituloMCentral = "Grafica TH"
# Customización
# Diccionario de colores customizables
ColoresConfigurables = {
    "graficaBordes":"#FFFFFF",
    "graficaFondo":"#57636E",
    "colorLTemp": "#C72020",
    "colorLHum": "#2C2CAD",
    "colorDHTBoton": "#64BF64",
    "colorUSOBoton": "#55E9E7"
}
# Diccionario con los titulos de los botones de cambio de color
BotonesModularesColor = {
    "graficaBordes":"Color de los bordes de la grafica",
    "graficaFondo":"Color del fondo de la grafica",
    "colorLTemp": "Color de la linea de temepratura",
    "colorLHum": "Color de la linea de humedad",
    "colorDHTBoton": "Color del boton DHT",
    "colorUSOBoton": "Color del boton Ultrasonido"
}
CustomColoresOriginal = ColoresConfigurables.copy() # Si no se usa el copy el python diria que simplemente son la misma direccion de memoria y no nos quedariamos con los colores originales
ctk.set_appearance_mode("dark")
ctk.set_default_color_theme("blue")

#Contadores
ultimoHB = 0

# Diccionario de indicadores y alarmas
Indicadores = {}

#media temperatura
datosMediaTemp = []

calcular = "python"
limiteTempMedia = 30.0
alarmaLimiteMediaTemp = False
contadorMediasAltas = 0

#Gestión del puerto serie
def AbrirSerial():
    global ser, serOpen
    try:
        ser = serial.Serial(serialPort,baudrate=9600,timeout=1)
        serOpen = True
    except:
        print("No se ha podido abrir el puerto serie")
        serOpen = False

def CerrarSerial():
    global ser, serOpen
    if serOpen:
        ser.close()
        serOpen = False

#Grafica TH
Graf = Figure(figsize=(5,4),dpi=100)
ax = Graf.add_subplot(111)
Graf.patch.set_facecolor("#242526")

#Funciones SatComm
def HeartbeatIndicator():
    global ultimoHB
    ultimoHB = time.time()

def SensorDHT(Datos):
    global datosTemp,datosHum,ErrorSensorDHT
    if Datos == "E":
        ErrorSensorDHT=True
    elif ";" in Datos:
        TempHum = Datos.split(";")
        Temp = float(TempHum[0])
        Hum = float(TempHum[1])
        print(f"Temp: {Temp} | Hum: {Hum}") #muestra en terminal los datos
        datosTemp.append(Temp) #añade la temperatura a la lista de datos
        datosHum.append(Hum) #añade la humedad a la lista de datos
        ErrorSensorDHT = False
    # Para tener la lista con los puntos especificados en puntosMAX
    if len(datosTemp) > puntosMax:
        datosTemp.pop(0) #elimina el dato mas antiguo
        datosHum.pop(0) 

def LeerDatos ():
    global ErrorSensorDHT
    while serOpen:
        try: #controlamos que se reciban bien los datos
            linea = ser.readline().decode('utf-8').strip() #leemos lo recibido
            if ":" in linea: #obtenemos los datos recibidos #comprobamos el formato
                Comms = linea.split(":")
                id = int(Comms[0])
                Datos = str(Comms[1])
                if (id == 1):
                    HeartbeatIndicator()
                elif (id == 2):
                    SensorDHT(Datos)
        except Exception as e:
            print(f"Error: {e}")
            time.sleep(0.1) #pausa
def AlarmaMediaTemp(media_actual): #alarma por si la media supera el limite
    global contadorMediasAltas, alarmaLimiteMediaTemp, limiteTempMedia
    if media_actual > limiteTempMedia:
        contadorMediasAltas += 1
        if contadorMediasAltas >= 3:
            alarmaLimiteMediaTemp = True
    else:
        contadorMediasAltas = 0
        alarmaLimiteMediaTemp = False
def CalculoDatosTemp(): #calcula la media de los 10 ultimos datos de temperatura recibidos
    global calcular
    if calcular == "python":
        if len(datosTemp) >= 10:
            ultimos_10 = datosTemp[-10:]
            media_calc = sum(ultimos_10) / 10
            datosMediaTemp.append(media_calc)

            AlarmaMediaTemp(media_calc)
    elif calcular == "satelite":
        pass

def cambiarLimiteTemp(choice): #cambiar el valor del limite de la temperatura media
    global limiteTempMedia
    limiteTempMedia = float(choice)
    
#Funcion que actualiza los datos de la grafica
def ActuGraf():
    global datosHum,datosTemp
    ax.clear() #limpia el grafico
    ax.set_facecolor(ColoresConfigurables["graficaFondo"])
    ax.plot(range(len(datosTemp)),datosTemp,color=ColoresConfigurables["colorLTemp"],label="Temperatura") #dibuja Temperatura
    ax.plot(range(len(datosHum)),datosHum,color=ColoresConfigurables["colorLHum"],label="Humedad") #dibuja Humedad
    ax.legend()
    ColorBorde = ColoresConfigurables["graficaBordes"]
    ax.set_title("Datos Temperatura y humedad",color=ColorBorde)
    ax.set_ylabel("Temperatura y humedad",color=ColorBorde)
    ax.set_xlabel("Tiempo",color=ColorBorde)
    ax.set_xlim(0,puntosMax)
    ax.tick_params(axis='both',which='both',color=ColorBorde,labelcolor=ColorBorde)
    for spine in ax.spines.values():
        spine.set_color(ColorBorde)
    ax.tick_params(axis='x', which='both', length=0, labelbottom=False)
    ax.set_ylim(10, 90)
    canvas.draw() #actualiza la grafica
    
# Funcion que actualiza la GUI
def RefreshGUI():
    ActuGraf() ## Funcion que actualiza la grafica
    # Configuracion cuadrados de las alarmas
    # El cuadrado se ha guardado como un objeto, para cambiar su informacion hay que usar la funcion de ".configure", en este caso cambia el color.
    # Los primeros if buscan si el sensor/heartbeat que queremos cambiar esta en el diccionario donde hemos guardado las referencias a los cuadrados.
    colorHB = "grey"
    colorDHT = "grey"
    colorAlarmaMediaTemp = "grey"
    if("Heartbeat" in Indicadores):
        if ((time.time() - ultimoHB) < 1.0):
            colorHB = "#4890D3"
    if("Sensor DHT" in Indicadores):
        if ErrorSensorDHT:
            colorDHT = "#EB3D3D"
    if("Alarma Media Temp" in Indicadores):
        if alarmaLimiteMediaTemp:
            colorAlarmaMediaTemp = "#EB3D3D"

    if len(datosMediaTemp) > 0:
        lblMediaTiempoReal.configure(text=f"Temp. Media Actual: {datosMediaTemp[-1]:.2f} °C")
    else:
        lblMediaTiempoReal.configure(text="Temp. Media Actual: Esperando datos...")
            
    # El .configure busca en el diccionario donde esta guardado lo que qqueremos cambiar, va a el objeto y le cambia el color.
    Indicadores["Heartbeat"].configure(fg_color=colorHB)
    Indicadores["Sensor DHT"].configure(fg_color=colorDHT)
    Indicadores["Alarma Media Temp"].configure(fg_color=colorAlarmaMediaTemp)
    InicioBotonDHT.configure(fg_color=ColoresConfigurables["colorDHTBoton"])
    InicioBotonUSO.configure(fg_color=ColoresConfigurables["colorUSOBoton"])
    window.after(100, RefreshGUI)

# Creación de Botones de cambio de color modulares
# Funcion modular que crea los botones
def BotonModular(master,variableColor):
    def FuncionBoton():
        global ColoresConfigurables
        DialogoColor = ctk.CTkInputDialog(text="Introduzca un color en formato hexadecimal XXXXXX",title="Color nuevo")
        ColorNuevo = DialogoColor.get_input()
        if ColorNuevo:    
            try:
                int(ColorNuevo,16) # Comprueba que el color se pueda pasar a un entero de base 16, es decir, hexadecimal.
                if len(ColorNuevo)==6:
                    ColoresConfigurables[variableColor] = ColorNuevo
            except:
                print("Color no valido")

        if len(ColorNuevo)==6:
            ColorNuevo = f"#{ColorNuevo}"
            ColoresConfigurables[variableColor] = ColorNuevo
    BotonColor = ctk.CTkButton(master,text=BotonesModularesColor[variableColor],command=FuncionBoton,font=("Roboto",14),width=250)
    BotonColor.pack(fill='x',pady=5,padx=5)

# Funcion que hace la ventana de confi de colores
def ColorPopUp():
    ConfiColor = ctk.CTkToplevel(window)
    ConfiColor.title("Selección de colores")
    ConfiColor.geometry("500x400")
    def ColorReset():
        global ColoresConfigurables
        ColoresConfigurables = CustomColoresOriginal.copy()
    #Boton del Reset
    BotonReset = ctk.CTkButton(ConfiColor, text="Reset de colores",command=ColorReset,font=("Roboto",18),fg_color="#C0392B",hover_color="#962D22")
    BotonReset.pack(pady=15,padx=20,fill='x')
    #Frame Scrolleable con los botones
    ConfiColorScroll = ctk.CTkScrollableFrame(ConfiColor)
    ConfiColorScroll.pack(expand=True,fill="both",padx=20,pady=(0,20))
    BotonModular(ConfiColorScroll,"graficaBordes")
    BotonModular(ConfiColorScroll,"graficaFondo")
    BotonModular(ConfiColorScroll,"colorLTemp")
    BotonModular(ConfiColorScroll,"colorLHum")
    BotonModular(ConfiColorScroll,"colorDHTBoton")
    BotonModular(ConfiColorScroll,"colorUSOBoton")

# Funciones botones principales
def DHTStarStop():
    global DHTStatus
    if serOpen:
        if not DHTStatus:
            ser.write("2:i\n".encode('utf-8')) #enviamos el mensaje de iniciar
            DHTStatus = True
        else:
            ser.write("2:p\n".encode('utf-8')) #enviamos el mensaje de iniciar
            DHTStatus = False
def DHTSendInterval():
    DialogoIntervaloDHT = ctk.CTkInputDialog(text="Introduce intervalo de envío de 2000 a 10000 ms",title="Intervalo nuevo")
    try:
        IntervaloNuevo = int(DialogoIntervaloDHT.get_input())
        if IntervaloNuevo > 2000 and IntervaloNuevo < 10000:
            ser.write(f"2:{IntervaloNuevo}".encode('utf-8'))
    except:
        print("Intervalo invalido")
def ReanudarClick():
    global DHTStatus
    if serOpen:
        ser.write("2:i\n".encode('utf-8'))
        DHTStatus = True

# Funcion para crear indicadores de sensores y el heartbeat de manera modular asi no hay que escribirlo muchas veces
def AgregarIndicador(nombre_texto):
    # Crear un contenedor para la fila (para que el texto y el cuadrado estén alineados)
    fila = ctk.CTkFrame(ScrollAlarmas, fg_color="transparent")
    fila.pack(fill="x", padx=10, pady=5)
    
    # Etiqueta de texto
    lbl = ctk.CTkLabel(fila, text=nombre_texto, font=("Roboto", 13))
    lbl.pack(side="left", anchor="w")
    
    # Creacion del cuadrado, hay que tener en cuenta que "cuadrado" no es una variable si no una direccion de memoria
    cuadrado = ctk.CTkLabel(fila, text="", width=20, height=20, corner_radius=4, fg_color="grey")
    cuadrado.pack(side="right", padx=5)

    # Despues de crear el cuadrado, tenemos que poder cambiar el color, para eso hay que guardar donde esta el objeto en la memoria del ordenador
    # porque? Asi cuando queramos cambiar el color, solo hay que buscar donde esta guardado el objeto y al encontrarlo, usar funciones que permiten
    # cambiar 
    Indicadores[nombre_texto] = cuadrado

def SeleccionarPython():
    global calcular
    calcular = "python"
    if serOpen:
        ser.write("4:0\n".encode('utf-8'))

def SeleccionarSatelite():
    global calcular
    calcular = "satelite"
    if serOpen:
        ser.write("4:1\n".encode('utf-8'))

def actualizarLimite(event=None):
    global limiteTempMedia
    try:
        limiteTempMedia = float(entryLimiteTemp.get())
    except ValueError:
        pass

# Try Abrir serial
AbrirSerial()

# Inicio del hilo de leer Datos
threading.Thread(target=LeerDatos, daemon = True).start() 
#El daemon=True indica que son codigos que pueden no considerarse acabados al cerrar el programa de python
# Como el hilo de LeerDatos es diferente al del hilo principal que gestiona la ventana, hay que separar funciones que actualicen el GUI de las que funcionan en el fondo para
# que no haya errores entre ellas

# GUI
window = ctk.CTk()
window.title("ControlSat")
window.geometry("1920x1080") # Tamaño de la ventana en pixeles
window.grid_rowconfigure(0,weight=0)
window.grid_rowconfigure(1,weight=3)
window.grid_rowconfigure(2,weight=2)
window.grid_rowconfigure(3,weight=1)
window.grid_columnconfigure(0,weight=3)
window.grid_columnconfigure(1,weight=1)

# Configuración parte de Arriba
# Marco Principal
MarcoArriba = ctk.CTkFrame(window, bg_color="transparent")
MarcoArriba.grid(row=0,column=0,sticky="nsew")
# Configuración del titulo
tituloLabel = ctk.CTkLabel(MarcoArriba, text="Sistema satelital",font=("Roboto",22))
tituloLabel.pack(expand=True)
# Boton colores
ColorPopUpBoton = ctk.CTkButton(MarcoArriba,text="⚙️",command=ColorPopUp,font=("Roboto",28),width=40,height=40,)
ColorPopUpBoton.place(relx=1.0,rely=0.5,anchor="e",x=-10)

#Marco central
MarcoCentral = ctk.CTkFrame(window)
MarcoCentral.grid(row=1,column=0,padx=5,pady=5,sticky="nsew")
MarcoCentral.rowconfigure(0,weight=0)
MarcoCentral.rowconfigure(1,weight=1)
MarcoCentral.columnconfigure(0,weight=1)
MarcoCentral.columnconfigure(1,weight=4)
tituloGrafica = ctk.CTkLabel(MarcoCentral,text=tituloMCentral,font=("Roboto",15))
tituloGrafica.grid(row=0,column=1,sticky="ew",padx=15,pady=(8,0))
tituloAlarmas = ctk.CTkLabel(MarcoCentral,text="Heartbeat y Alarmas",font=("Roboto",15))
tituloAlarmas.grid(row=0,column=0,sticky="ew",padx=15,pady=(8,0))
# Grafica
canvas = FigureCanvasTkAgg(Graf,master=MarcoCentral)
canvas.get_tk_widget().grid(row=1,column=1,columnspan=3,padx=5,pady=(0,5),sticky="nsew")

#Alarmas
ScrollAlarmas = ctk.CTkScrollableFrame(MarcoCentral)
ScrollAlarmas.grid(row=1,column=0,padx=5,pady=5,sticky="nsew")
# Llamar a la funcion que crea alarmas modularmente
AgregarIndicador("Heartbeat")
AgregarIndicador("Sensor DHT")
AgregarIndicador("Alarma Media Temp")

#Marco superior de los botones
frame_botones = ctk.CTkFrame(window)
frame_botones.grid(row=2,column=0, columnspan=2, padx=20,pady=10,sticky="nsew")
frame_botones.rowconfigure(0,weight=0)
frame_botones.rowconfigure(1,weight=1)
frame_botones.columnconfigure(0,weight=1)
frame_botones.columnconfigure(1,weight=1)
frame_botones.columnconfigure(2,weight=1)
titulobotones = ctk.CTkLabel(frame_botones,text="Controles sensor DHT",font=("Roboto",15),anchor="w")
titulobotones.grid(row=0,column=0,columnspan=3,sticky="ew",padx=15,pady=(8,0))

#Botones Marco Superior
InicioBoton = ctk.CTkButton(frame_botones,text="Iniciar",command=DHTStarStop, font=("Roboto",18),fg_color="pale green",text_color="black",)
InicioBoton.grid(row=1,column=0,padx=5,pady=5,sticky="nsew")
PararBoton = ctk.CTkButton(frame_botones,text="Parar",command=DHTStarStop, font=("Roboto",18),fg_color="tomato",text_color="black",)
PararBoton.grid(row=1,column=1,padx=5,pady=5,sticky="nsew")
ReanudarBoton = ctk.CTkButton(frame_botones,text="Reanudar",command=ReanudarClick, font=("Roboto",18),fg_color="goldenrod1",text_color="black",)
ReanudarBoton.grid(row=1,column=2,padx=5,pady=5,sticky="nsew")

#Marco inferior de los botones
frameInf_botones = ctk.CTkFrame(window)
frameInf_botones.grid(row=3,column=0,padx=20,pady=10,sticky="nsew")
frameInf_botones.rowconfigure(0,weight=0)
frameInf_botones.columnconfigure(0,weight=1)
frameInf_botones.columnconfigure(1,weight=1)

#Botones marco inferior
AbrirSerialBoton = ctk.CTkButton(frameInf_botones,text="Abrir Puerto serie",command=AbrirSerial, font=("Roboto",18))
AbrirSerialBoton.grid(row=0,column=0,padx=5,pady=5,sticky="nsew")
ColorPopUpBoton = ctk.CTkButton(frameInf_botones,text="Abrir config de colores",command=ColorPopUp, font=("Roboto",18))
ColorPopUpBoton.grid(row=0,column=1,padx=5,pady=5,sticky="nsew")

#columna con valores temperatura Media:
frameDer_temp = ctk.CTkFrame(window)
frameDer_temp.grid(row=1, column=1, padx=5, pady=5, sticky="nsew")
frameDer_temp.rowconfigure(0, weight=0)
frameDer_temp.columnconfigure(0, weight=1)

titulotemp = ctk.CTkLabel(frameDer_temp, text="Configuración Media de Temp", font=("Roboto", 15))
titulotemp.pack(pady=(10, 15))

lblMetodoCalc = ctk.CTkLabel(frameDer_temp, text="Método de cálculo:", font=("Roboto", 14))
lblMetodoCalc.pack(pady=5)


boton_Python = ctk.CTkButton(frameDer_temp, text="Calcular en Python", command=SeleccionarPython, font=("Roboto", 16), fg_color="#2980B9", height=45)
boton_Python.pack(fill="x", padx=20, pady=8)

boton_Satelite = ctk.CTkButton(frameDer_temp, text="Calcular en Satélite", command=SeleccionarSatelite, font=("Roboto", 16), fg_color="#8E44AD", height=45)
boton_Satelite.pack(fill="x", padx=20, pady=8)

lblLimiteTexto = ctk.CTkLabel(frameDer_temp, text="Límite Temp Media:", font=("Roboto", 14))
lblLimiteTexto.pack(pady=(20, 5))

entryLimiteTemp = ctk.CTkEntry(frameDer_temp, font=("Roboto", 14))
entryLimiteTemp.insert(0, str(limiteTempMedia))
entryLimiteTemp.pack(fill="x", padx=20, pady=5)

# Vincular el evento de escritura o presionar Enter para actualizar el valor automáticamente
entryLimiteTemp.bind("<KeyRelease>", actualizarLimite)

lblMediaTiempoReal = ctk.CTkLabel(frameDer_temp, text="Temp. Media Actual: Esperando datos...", font=("Roboto", 14))
lblMediaTiempoReal.pack(pady=5)

window.after(100,RefreshGUI) #llama a la funcion que hace que el grafico y las alarmas tengan la nueva informacion
window.mainloop()
