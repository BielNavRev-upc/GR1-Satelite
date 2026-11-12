// Variables globales
bool sendTH = false;
bool errorTH = false;

long intervalTH=2000;
const long intervalHeartbeat = 3000;
const long THerrorTimeout = 5000;
const long SendErrorTHInterval = 1000;

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
        Now = millis();
        SatComm(Now); // Lee si hay algún mensaje de la interfaz
        if(Now-lastHeartbeat >= intervalHeartbeat){
                Heartbeat(Now);
        }
        if(sendTH && (Now-lastTH >= intervalTH)){
                THControl(Now);
        }
}

void SatComm(unsigned long Now){
        String GroundCMD = mySerial.readStringUntil('\n');
        GroundCMD.trim();
        String ID = GroundCMD.substring(0,2);
        GroundCMD.remove(0,2);
        if (ID == "2:"){
                DHTCMDController(GroundCMD);
        }
}
void DHTCMDController(String GroundCMD){
        if(GroundCMD == "i"){
                sendTH = true;
                lastValidTH = Now; //Arranca la ventana de 5s de timeout del DHT
        } else if (GroundCMD == "p"){ // Desactiva el envío de datos si recibe 'p' (Parar)
                sendTH = false;
        } else {
                intervalTH = GroundCMD.toInt();
                //intervalTH = NewInterval;
        }
}
void Heartbeat(unsigned long Now){
        mySerial.println("1:");
        lastHeartbeat = Now;
}
void THControl(unsigned long Now){
        lastTH = Now;
        float h = dht.readHumidity(); //Lee humedad
        float t = dht.readTemperature(); //Lee temperatura
        if (isnan(h) || isnan(t)){ // Comprueba si ha ocurrido algún error al leer
         // Si no lee nada no actualiza la lectura valida
        } else { // Envía los datos
                mySerial.print("2:");
                mySerial.print(t);
                mySerial.print(";");
                mySerial.println(h);
        }
}