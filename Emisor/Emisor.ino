// Variables globales
bool sendTH = false;
bool errorTH = false; 

const long intervalTH=2000; //Intervalo de envio de datos del sensor DHT
const long intervalHeartbeat = 3000; // Intervalo de señal de vida
const long THerrorTimeout = 5000; // Timeout del sensor DHT
const long SendErrorTHInterval = 1000; //Cada cuantos segundos envia el señal de error

unsigned long lastTH = 0;
unsigned long lastHeartbeat = 0;
unsigned long lastSentErrorTH = 0;
unsigned long lastErrorTH = 0;
unsigned long lastValidTH = 0;
unsigned long Now;

#include <SoftwareSerial.h>
#include <DHT.h>
// Definición de pines y tipo para el sensor DHT11
#define DHTPIN 2
#define DHTTYPE DHT11
// Instancia del sensor DHT (en el pin 2)
DHT dht(DHTPIN, DHTTYPE);
SoftwareSerial mySerial(10, 11); //// Configuración de puerto serie; RX, TX

void setup() {
        // Inicializa el puerto serie por software
        mySerial.begin(9600);
        // Inicializa el puerto serie por hardware
        Serial.begin(9600);
        // Inicialización del sensor
        dht.begin();
}

void loop() {
        Now = millis(); // Hace que los relojes esten sincronizados
        SatComm(); // Llama a la funcion que se comunica con la estación de tierra
        if(Now-lastHeartbeat >= intervalHeartbeat){ // Llama a la funcion de señal de vida cada intervalo y da de parametro "Now"
                Heartbeat(Now);
        }
        if(sendTH && (Now-lastTH >= intervalTH)){ // Llama a la funcion del sensor DHT cada intervalo con el parametro "Now"
                THControl(Now);
        }
        if(sendTH){ // Si esta el envio de datos activo llama a la funcion que comprueba si hay un error en el DHT con el parametro "Now"
                ErrorTHCheck(Now);
        }
}

void SatComm(){ // Lee si hay algún mensaje de la interfaz
        char GroundCMD = mySerial.read();
        // Activa el envío de datos si recibe 'i' (Inicio)
        if(GroundCMD == 'i'){
                sendTH = true;
                lastValidTH = millis(); //Arranca la ventana de 5s de timeout del DHT
        } else if (GroundCMD == 'p'){ // Desactiva el envío de datos si recibe 'p' (Parar)
                sendTH = false;
        }
}
void Heartbeat(unsigned long Now){ // Controla el envio de la señal de vida a la estacion de tierra
        mySerial.println("Heartbeat");
        lastHeartbeat = Now;
}
void THControl(unsigned long Now){ // Controla la lectura de datos del sensor DHT
        lastTH = Now; // Registra cuando empieza la función
        float h = dht.readHumidity(); //Lee humedad
        float t = dht.readTemperature(); //Lee temperatura
        if (isnan(h) || isnan(t)){ // Comprueba si ha ocurrido algún error al leer
         // Si no lee nada no actualiza la lectura valida
        } else { // Envía los datos
                mySerial.print(t);
                mySerial.print(":");
                mySerial.println(h);
                lastValidTH = Now; // Actu Ultima lectura valida
        }
}
void ErrorTHCheck(unsigned long Now){ // Comprueba si ha pasado el timeout del error del DHT
        if(Now-lastValidTH >= THerrorTimeout){
                if(Now - lastErrorTH >= SendErrorTHInterval){
                        mySerial.println("ErrorTH");
                        lastErrorTH = Now;
                }
        }
}
