// Variables globales
const int led1=12;
bool send = false;
const long interval=2000;
bool ErrorTH=false;

unsigned long lastHeartbeat = 0;
unsigned long heartbeatInterval = 3000; // 3s señal de vida
unsigned long lastTHTime = 0;
unsigned long lastValidTH = 0;
const unsigned long TimeoutTH = 5000; // 5s de timeout
unsigned long lastErrorTH = 0;
const unsigned long IntervalErrorTH = 500; // 500ms entre envios de señal de error

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
        lastValidTH = millis();
}
void loop() {
        unsigned long Now = millis();
        ReadCommand();
        THControl(Now);
        Heartbeat(Now);
}
void ReadCommand(){
        if (mySerial.available()>0){
                char mensaje = mySerial.read();
                // Activa el envío de datos si recibe 'i' (Inicio)
                if(mensaje == 'i'){
                        send = true;
                } else if (mensaje == 'p'){ // Desactiva el envío de datos si recibe 'p' (Parar)
                        send = false;
                }
        }
}
void THControl(unsigned long Now){
        if(Now-lastTHTime >= interval){
                lastTHTime = Now;
                float h = dht.readHumidity(); //Lee humedad
                float t = dht.readTemperature(); //Lee temperatura

                if (isnan(h) || isnan(t)) // Comprueba si ha ocurrido algún error al leer
                        if(Now - lastValidTH > TimeoutTH){
                                ErrorTH = true;
                        }
                else { // Envía los datos
                        ErrorTH = false;
                        lastValidTH = Now;
                        if(send){
                        mySerial.print(t);
                        mySerial.print(":");
                        mySerial.println(h);
                        }
                }
                if(ErrorTH){
                        if(Now-lastErrorTH >=IntervalErrorTH){
                                lastErrorTH = Now;
                                mySerial.println("ErrorTH");
                        }
                }
        }
}
// Enviar heartbeat cada 3s (heartbeatInterval)
void Heartbeat(unsigned long Now){
        if(Now-lastHeartbeat >= heartbeatInterval){
                lastHeartbeat = Now;
                mySerial.println("Heartbeat");
        }
}