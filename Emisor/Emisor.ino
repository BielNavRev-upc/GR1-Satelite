#include <SoftwareSerial.h>
#include <DHT.h>
#include<Servo.h>

#define DHTPIN 2
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
SoftwareSerial mySerial(10, 11);

// Variables globales
bool sendTH = false;
bool errorTH = false;
bool calcularEnSatelite = false;
bool actRadar = false;

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
unsigned long lastRadar = 0;

const unsigned long intervalRadar = 30; // 30ms entre señales

float ultimos10Temp[10];
float limiteTempMedia;
int indiceTemp = 0;
int totalTemperaturas = 0;

int currentAngle = 0;
int stepDirection = 1;

long duracion;
float distancia;

Servo myServo;

int contadorMediasAltas = 0;
bool alarmaMediaActiva = false;

const int trigPin = 6;
const int echoPin = 5;
const int servoPin = 9;

void setup() {
        // Inicializa el puerto serie por software
        mySerial.begin(9600);
        // Inicializa el puerto serie por hardware
        Serial.begin(9600);
        // Inicialización del sensor
        dht.begin();
  
        pinMode(trigPin, OUTPUT);
        pinMode(echoPin, INPUT);
        
        myServo.attach(servoPin);
        myServo.write(currentAngle);

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
        if(actRadar && (Now-lastRadar >= intervalRadar)){
                Radar(Now);
        }
}

void SatComm(unsigned long Now) {
    if (mySerial.available() > 0) {
        String comando = mySerial.readStringUntil('\n');
        comando.trim(); 
        String ID = comando.substring(0,2);
        comando.remove(0,2);
        if(ID="2:"){
            if (comando == "i"){ //Si recibe de la interfaz 'i' (Iniciar) o 'r' (Reanudar), envía una "i" al emisor
                sendTH = true;
            } else if (comando == "p"){ //Si recibe de la interfaz 'p' (Parar), envía una "p" al emisor
                sendTH = false;
            }
        }
        if(ID="4:"){
            if (comando == "1"){ //Si recibe de la interfaz 'i' (Iniciar) o 'r' (Reanudar), envía una "i" al emisor
                calcularEnSatelite = true;
            } else if (comando == "0"){ //Si recibe de la interfaz 'p' (Parar), envía una "p" al emisor
                calcularEnSatelite = false;
            }
        }
        //limiteTempMedia
        if(ID="3:"){
            limiteTempMedia = comando.toFloat();
        }
        /*
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
                if (comando == "i") {
                    sendTH = true;
                } else if (comando == "p") {
                    sendTH = false;
                }
            }
        }*/
        if(ID="6:"){
            if (comando == "i"){ //Si recibe de la interfaz 'i' (Iniciar) o 'r' (Reanudar), envía una "i" al emisor
                actRadar = true;
            } else if (comando == "p"){ //Si recibe de la interfaz 'p' (Parar), envía una "p" al emisor
                actRadar = false;
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

void ProcesarMediaSatelite(float t) {
    ultimos10Temp[indiceTemp] = t;
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

            }
        } else {
            contadorMediasAltas = 0;
            alarmaMediaActiva = false;
        }
    }
}

void Radar(unsigned long Now){
    lastRadar = Now;
    currentAngle += stepDirection;
    if(currentAngle>=180){
        currentAngle = 180;
        stepDirection = -1;
    } else if(currentAngle<=0){
        currentAngle=0;
        stepDirection = 1;
    }
    myServo.write(currentAngle);
 
    // Enviar un pulso de 10 microsegundos al Trigger
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Leer el tiempo que tarda el eco en regresar (en microsegundos)
    duracion = pulseIn(echoPin, HIGH);

    // Calcular la distancia en cm (velocidad del sonido ~ 343m/s -> 0.0343 cm/us / 2)
    distancia = duracion * 0.034 / 2;

    // Mostrar el resultado en el monitor serie
    mySerial.print("6: ");
    mySerial.print(distancia);
    mySerial.print(";");
    mySerial.println(currentAngle);
}
