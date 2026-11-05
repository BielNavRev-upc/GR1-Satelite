// Variables globales
bool sendTH = false;
bool errorTH = false;

const long intervalTH=2000;
const long intervalHeartbeat = 3000;
const long THerrorTimeout = 5000;
const long SendErrorTHInterval = 1000;

unsigned long lastTH = 0;
unsigned long lastHeartbeat = 0;
unsigned long lastSentErrorTH = 0;
unsigned long lastErrorTH = 0;

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
        mySerial.println("Empezamos");
        // Inicializa el puerto serie por hardware
        Serial.begin(9600);
        // Inicialización del sensor
        dht.begin();
}

void loop() {
        Now = millis();
        SatComm(); // Lee si hay algún mensaje de la interfaz
        if(Now-lastheartbeat >= intervalHearbeat){
                Heartbeat(Now);
        }
        if(sendTH && (Now-lastTH >= intervalTH)){
                THControl(Now);
        }
}

void SatComm(){
        char mensaje = mySerial.read();
        // Activa el envío de datos si recibe 'i' (Inicio)
        if(mensaje == 'i'){
                enviar = true;
        } else if (mensaje == 'p'){ // Desactiva el envío de datos si recibe 'p' (Parar)
                enviar = false;
        }
}
void Heartbeat(unsigned long Now){
        mySerial.println("Heartbeat");
        lastHeartbeat = Now;

}
void THControl(unsigned long Now){
        lastTH = Now;
        float h = dht.readHumidity(); //Lee humedad
        float t = dht.readTemperature(); //Lee temperatura
        if (isnan(h) || isnan(t)) // Comprueba si ha ocurrido algún error al leer
                errorTH = true;
                lastErrorTH = Now;
        else { // Envía los datos
                mySerial.print(t);
                mySerial.print(":");
                mySerial.println(h);
                errorTH = false;
        }
        if (errorTH){
                if (Now - lastErrorTH >= THerrorTimeout){
                if (Now - lastSentErrorTH >= SendErrorTHInterval){
                        mySerial.println("ErrorTH");
                        lastSentErrorTH = Now;
                }
        }
        }
}