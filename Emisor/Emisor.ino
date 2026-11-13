#include <SoftwareSerial.h>
#include <DHT.h>

#define DHTPIN 2
#define DHTTYPE DHT11
#define RELE_PIN 7

DHT dht(DHTPIN, DHTTYPE);
SoftwareSerial mySerial(10, 11);

// Variables globales
bool sendTH = false;
bool errorTH = false;
bool calcularEnSatelite = false;

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

float ultimos10Temp[10];
int indiceTemp = 0;
int totalTemperaturas = 0;

int contadorMediasAltas = 0;
bool alarmaMediaActiva = false;

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
        pinMode(RELE_PIN, OUTPUT);
        digitalWrite(RELE_PIN, LOW);

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
void SatComm(unsigned long Now) {
    if (mySerial.available() > 0) {
        String comando = mySerial.readStringUntil('\n');
        comando.trim(); 
        
        if (comando.length() > 0) {
            int fin = comando.indexOf(':', 0);
            if (fin != -1) {
                // COMANDOS CON FORMATO "CODIGO:VALOR"
                int codigo = comando.substring(0, fin).toInt();
                String valorStr = comando.substring(fin + 1);
                
                if (codigo == 2) {
                    // Control específico del DHT si se envía con prefijo 2:
                    if (valorStr == "i") {
                        sendTH = true;
                        lastValidTH = Now;
                    } else if (valorStr == "p") {
                        sendTH = false;
                    } else {
                        intervalTH = valorStr.toInt();
                    }
                } else if (codigo == 3) {
                    // ACTUALIZAR LIMITE DE TEMPERATURA MEDIA DESDE TIERRA
                    limiteTempMedia = valorStr.toFloat();
                } else if (codigo == 4) {
                    // SELECCIONAR MODO DE CALCULO (0: Python, 1: Satélite)
                    int modo = valorStr.toInt();
                    calcularEnSatelite = (modo == 1);
                    // Reiniciar contadores al cambiar de modo
                    totalTemperaturas = 0;
                    indiceTemp = 0;
                }
            } else {
                // COMANDOS SIMPLES DE TEXTO DIRECTO
                if (comando == "iTH" || comando == "i") {
                    sendTH = true;
                    lastValidTH = Now; 
                } else if (comando == "pTH" || comando == "p") {
                    sendTH = false;
                } else if (comando == "rTH") {
                    sendTH = true;
                }
            }
        }
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
                if (calcularEnSatelite) {
                ProcesarMediaSatelite(t);
              }
        }
}

void ProcesarMediaSatelite(float nuevaTemp) {
    ultimos10Temp[indiceTemp] = nuevaTemp;
    indiceTemp = (indiceTemp + 1) % 10;
    if (totalTemperaturas < 10) {
        totalTemperaturas++;
    }

    
    if (totalTemperaturas == 10) {
        float suma = 0;
        for (int i = 0; i < 10; i++) {
            suma += ultimos10Temp[i];
        }
        float mediaActual = suma / 10.0;

        mySerial.print("5:");
        mySerial.println(mediaActual);

        if (mediaActual > limiteTempMedia) {
            contadorMediasAltas++;
            if (contadorMediasAltas >= 3) {
                alarmaMediaActiva = true;
                digitalWrite(RELE_PIN, HIGH); //activa sistema refrigeracion
            }
        } else {
            contadorMediasAltas = 0;
            alarmaMediaActiva = false;
            digitalWrite(RELE_PIN, LOW); //desactiva sistema refrigeracion
        }
    }
}
