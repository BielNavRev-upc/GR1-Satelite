# Importar modulos
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import customtkinter as ctk
import serial
import threading
import time

#Declaracion estilo GUI
ctk.set_appearance_mode("dark")
ctk.set_default_color_theme("blue")
tituloFrame1 = "Grafica de temperatura y humedad"

# Diccionario colores configurables
ColoresConfigurablesOriginales = {
    "graficaBordes": "#FFFFFF",
    "graficaFondo": "#313D49",
    "colorLTemp": "#C72020",
    "colorLHum": "#2C2CAD"
}
ColoresConfigurables = ColoresConfigurablesOriginales.copy() # El programa usará una copia de los colores originales para cambiarlos.

#Variables globales
puntosMAX = 150 #limite puntos que se muestran en la grafica
dibujar = True #Dibuja el gràfico
datosTemp = []
datosHum = []
ErrorSensorDHT = False
ser = None
SerOpen = False
# Contador HB
ultimoHeartbeat = 0

# Diccionario donde se guardan las referencias a los indicadores que se crearan mas abajo.
Indicadores = {}

#Inicio del serial
def AbrirSerial():
    global ser, SerOpen
    try:
        ser = serial.Serial("/dev/ttyACM0",baudrate=9600, timeout = 1) # ComX en windows, substituir X por el que toque
        SerOpen = True
    except:
        print("No se ha podido abrir el puerto serie")
        SerOpen = False

#Grafica
graf = Figure(figsize=(5,4),dpi=100)
ax = graf.add_subplot(111)
graf.patch.set_facecolor("#242526")

#Funciones
def HeartbeatIndicator(): # Indicador heartbeat, misma logica que en el arduino.
    global ultimoHeartbeat
    ultimoHeartbeat = time.time()
    
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
    if len(datosTemp) > puntosMAX:
        datosTemp.pop(0) #elimina el dato mas antiguo
        datosHum.pop(0) 

def LeerDatos ():
    global ErrorSensorDHT
    while SerOpen:
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
            
threading.Thread(target=LeerDatos, daemon = True).start() #El daemon=True indica que son codigos que pueden no considerarse acabados al cerrar el programa de python
# Como el hilo de LeerDatos es diferente al del hilo principal que gestiona la ventana, hay que separar funciones que actualicen el GUI de las que funcionan en el fondo para
# que no haya errores entre ellas

def ActualizarInterfaz(): # Funcion que se encarga de actualizar la GUI con los datos nuevos y con la informacion de las alarmas?
    global dibujar, datosTemp,datosHum,ultimoHeartbeat
    if dibujar:
        ax.clear() #limpia el grafico
        ax.set_facecolor(ColoresConfigurables["graficaFondo"])
        ax.plot(range(len(datosTemp)),datosTemp,color=ColoresConfigurables["colorLTemp"],label="Temperatura") #dibuja Temperatura
        ax.plot(range(len(datosHum)),datosHum,color=ColoresConfigurables["colorLHum"],label="Humedad") #dibuja Humedad
        ax.legend()
        ColorBorde = ColoresConfigurables["graficaBordes"]
        ax.set_title("Datos Temperatura y humedad",color=ColorBorde)
        ax.set_ylabel("Temperatura y humedad",color=ColorBorde)
        ax.set_xlabel("Tiempo",color=ColorBorde)
        ax.set_xlim(0,puntosMAX)
        ax.tick_params(axis='both',which='both',color=ColorBorde,labelcolor=ColorBorde)
        for spine in ax.spines.values():
            spine.set_color(ColorBorde)
        ax.tick_params(axis='x', which='both', length=0, labelbottom=False)
        ax.set_ylim(10, 90)
        canvas.draw() #actualiza la grafica

        # Configuracion cuadrados de las alarmas
        # El cuadrado se ha guardado como un objeto, para cambiar su informacion hay que usar la funcion de ".configure", en este caso cambia el color.
        # Los primeros if buscan si el sensor/heartbeat que queremos cambiar esta en el diccionario donde hemos guardado las referencias a los cuadrados.
        colorHB = "grey"
        colorDHT = "grey"
        if("Heartbeat" in Indicadores):
            if ((time.time() - ultimoHeartbeat) < 1.0):
                colorHB = "#4890D3"
        if("Sensor DHT" in Indicadores):
            if ErrorSensorDHT:
                colorDHT = "#EB3D3D"

        # El .configure busca en el diccionario donde esta guardado lo que qqueremos cambiar, va a el objeto y le cambia el color.
        Indicadores["Heartbeat"].configure(fg_color=colorHB)
        Indicadores["Sensor DHT"].configure(fg_color=colorDHT)

    window.after(100,ActualizarInterfaz) #actualiza cada 100 milisegundos

class BotonColores(ctk.CTkButton):
    def __init__(self, master,titulo,LlaveColor,CambioCallback,**kwargs): # El '**kwargs' son opciones adicionales del boton como el color, el tamaño etc, se usa para que quede limpio
        '''El super va a el parent de la clase, el "ctk.CTkButon", y hace que haga su setup, es decir, como si haces x = ctk.CTkButon(master) normalmente(fuera de la clase)'''
        super().__init__(master,text=titulo, **kwargs)

        # Guarda el "DNI" del color (la llave) y el callback dentro del objeto
        self.LlaveColor = LlaveColor
        self.CambioCallback = CambioCallback
        # Asigna a que funcion apunta el comando
        self.configure(command=self._Click)

    def _Click(self):
        dialog = ctk.CTkInputDialog(text="Introduzca un color en hexadecimal '#XXXXXX'",title="Color Nuevo")
        ColorNuevo = dialog.get_input()

        #Comprobación de que el input es un codigo valido
        if ColorNuevo.startswith('#') and len(ColorNuevo) == 7: 
            #Una vez verificado, el "callback" pone el color nuevo donde dice la llave (su DNI)
            self.CambioCallback(self.LlaveColor,ColorNuevo)

class ConfiColoresDialog(ctk.CTkToplevel): 
    #Crear la clase ConfiColoresDialog, que hereda todo lo que "ctk.CTkToplevel" puede hacer (lo que se usa para hacer una ventana de popup)
    def __init__(self, parent,opciones,CambioCallback,ResetCallback): # Setup de la clase
        '''El super va a el parent de la clase, el "ctk.CTkToplevel", y hace que haga su setup, es decir, como si haces x = ctk.CTkToplevel(parent) normalmente(fuera de la clase)'''
        super().__init__(parent) 
        self.title("Selección de colores")
        self.geometry("400x500")

        # Hooks de la ventana
        self.CambioCallback = CambioCallback
        self.ResetCallback = ResetCallback
        self.opciones = opciones # Lista de opciones

        self._setupUi()

    def _setupUi(self):
        #Boton de reset arriba del todo
        self.BotonReset = ctk.CTkButton(self, text="Reset de colores",command=self.ResetCallback,font=("Roboto",18),fg_color="#C0392B",hover_color="#962D22")
        self.BotonReset.pack(pady=15,padx=20,fill='x')

        # Marco con barra de scroll
        self.MarcoScroll = ctk.CTkScrollableFrame(self)
        self.MarcoScroll.pack(expand = True, fill="both",padx=20,pady=(0,20))

        # Creación de botones de manera modular
        for titulo, llave in self.opciones: # como options esta compuesta de tuples (parejas de 2 elementos), el bucle for le asigna titulo al primer elemento y llave al 2o
            BotonColores(self.MarcoScroll,titulo=titulo,LlaveColor=llave,CambioCallback=self.CambioCallback,font=("Roboto",14),width=250).pack(pady=5,padx=10,fill='x')

# Puente entre la UI y los datos
def CambiarColor(llave, nuevoColor):
    global ColoresConfigurables
    ColoresConfigurables[llave] = nuevoColor

def ColorReset():
    global ColoresConfigurables
    ColoresConfigurables = ColoresConfigurablesOriginales.copy() # No hace falta declarar coloresConfigurablesOriginales una global pq solo esta leyendo, no reasignando.

def AbrirMenuColores():
    '''Sitio donde se define la estructura del menu de cambiar colores'''
    opciones = [
        ("Borde de la gráfica", "graficaBordes"),
        ("Fondo de la gráfica", "graficaFondo"),
        ("Color de la linea de temperatura","colorLTemp"),
        ("Color de la linea de humedad","colorLHum")
    ]
    ConfiColoresDialog(window, opciones,CambiarColor,ColorReset)
# Fin del puente

#Funciones botones
def IniciarClick ():
    if SerOpen:
        ser.write("i".encode('utf-8')) #enviamos el mensaje de iniciar
def PararClick ():
    if SerOpen:
        ser.write("p".encode('utf-8')) #enviamos el mensaje de parar
def ReanudarClick ():
    if SerOpen:
        ser.write("r".encode('utf-8')) #enviamos el mensaje de reanudar

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
#Try inicial de abrir serial
AbrirSerial()
#GUI
window = ctk.CTk()
window.title("ControlSat")
window.geometry("1920x1080") # Tamaño de la ventana en pixeles
window.grid_rowconfigure(0,weight=0)
window.grid_rowconfigure(1,weight=3)
window.grid_rowconfigure(2,weight=2)
window.grid_rowconfigure(2,weight=2)
window.grid_columnconfigure(0,weight=1)

#Configuración del titulo
tituloLabel = ctk.CTkLabel(window, text="Sistema satelital",font=("Roboto",22))
tituloLabel.grid(row=0,column=0,columnspan=3,padx=5,pady=5,sticky="nsew")

#Marco de la grafica
frame_graf = ctk.CTkFrame(window)
frame_graf.grid(row=1,column=0,padx=5,pady=5,sticky="nsew")
frame_graf.rowconfigure(0,weight=0)
frame_graf.rowconfigure(1,weight=1)
frame_graf.columnconfigure(0,weight=1)
frame_graf.columnconfigure(1,weight=4)
tituloGrafica = ctk.CTkLabel(frame_graf,text=tituloFrame1,font=("Roboto",15))
tituloGrafica.grid(row=0,column=1,sticky="ew",padx=15,pady=(8,0))
tituloAlarmas = ctk.CTkLabel(frame_graf,text="Heartbeat y Alarmas",font=("Roboto",15))
tituloAlarmas.grid(row=0,column=0,sticky="ew",padx=15,pady=(8,0))

#Grafico
canvas = FigureCanvasTkAgg(graf,master=frame_graf)
canvas.get_tk_widget().grid(row=1,column=1,columnspan=3,padx=5,pady=(0,5),sticky="nsew")

#Alarmas
ScrollAlarmas = ctk.CTkScrollableFrame(frame_graf)
ScrollAlarmas.grid(row=1,column=0,padx=5,pady=5,sticky="nsew")
AgregarIndicador("Heartbeat")
AgregarIndicador("Sensor DHT")

#Marco superior de los botones
frame_botones = ctk.CTkFrame(window)
frame_botones.grid(row=2,column=0,padx=20,pady=10,sticky="nsew")
frame_botones.rowconfigure(0,weight=0)
frame_botones.rowconfigure(1,weight=1)
frame_botones.columnconfigure(0,weight=1)
frame_botones.columnconfigure(1,weight=1)
frame_botones.columnconfigure(2,weight=1)
titulobotones = ctk.CTkLabel(frame_botones,text="Controles sensor DHT",font=("Roboto",15),anchor="w")
titulobotones.grid(row=0,column=0,columnspan=3,sticky="ew",padx=15,pady=(8,0))

#Botones Marco Superior
InicioBoton = ctk.CTkButton(frame_botones,text="Iniciar",command=IniciarClick, font=("Roboto",18),fg_color="pale green",text_color="black",)
InicioBoton.grid(row=1,column=0,padx=5,pady=5,sticky="nsew")
PararBoton = ctk.CTkButton(frame_botones,text="Parar",command=PararClick, font=("Roboto",18),fg_color="tomato",text_color="black",)
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
InicioBoton = ctk.CTkButton(frameInf_botones,text="Abrir Puerto serie",command=AbrirSerial, font=("Roboto",18))
InicioBoton.grid(row=0,column=0,padx=5,pady=5,sticky="nsew")
PararBoton = ctk.CTkButton(frameInf_botones,text="Abrir config de colores",command=AbrirMenuColores, font=("Roboto",18))
PararBoton.grid(row=0,column=1,padx=5,pady=5,sticky="nsew")

window.after(100,ActualizarInterfaz) #llama a la funcion que hace que el grafico y las alarmas tengan la nueva informacion

window.mainloop()

