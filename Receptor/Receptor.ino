#include <SoftwareSerial.h>
// Variables globales
const unsigned long heartbeatLEDInterval = 1000; // Intervalo del LED de señal de vida del satelite
const int heartbeatLED = 7; // Pin del LED de señal de vida
const int ErrorTHLED = 8; // Pin del LED de Error del sensor DHT

bool heartbeat = false;
bool errorTH = false;

unsigned long lastHeartbeatTurnOFF = 0;
unsigned long lastErrorTHRec = 0;

// Configura los pines digitales 10 y 11
SoftwareSerial mySerial(10, 11); // RX, TX (azul, naranja)

void setup() {
   // Inicializa el puerto serie principal
   Serial.begin(9600);
   // Inicializa el puerto serie
   mySerial.begin(9600);
   // Configuración de los pines de los LEDs
   pinMode(heartbeatLED,OUTPUT);
   pinMode(ErrorTHLED,OUTPUT);
}
void loop() {
   GroundCMDControl(); // Llama a la funcion que lee los comandos de la GUI de python
   if (mySerial.available()>0) {//comprueba si hay datos por leer
      String satFeed = mySerial.readStringUntil('\n');//lectura datos
      satFeed.trim(); // Limpia los datos
      SatFeedControl(satFeed); // Llama a la funcion que se encarga de gestionar los datos (el feed) que viene del satelite
   }
   if(heartbeat && (millis()-lastHeartbeatTurnOFF >= heartbeatLEDInterval)){ // Apaga el LED de señal de vida despues de 1s de haber recibido la señal
      heartbeat = false;
      digitalWrite(heartbeatLED,LOW);
   }
   ErrorCheck(); // Llama a la funcion que comprueba si esta activo el error del sensor DHT
}
void GroundCMDControl(){
   char GroundCMD = Serial.read();
   if (GroundCMD == 'i' || GroundCMD == 'r'){ //Si recibe de la interfaz 'i' (Iniciar) o 'r' (Reanudar), envía una "i" al emisor
      mySerial.println("i");
   } else if (GroundCMD == 'p'){ //Si recibe de la interfaz 'p' (Parar), envía una "p" al emisor
      mySerial.println("p");
   }
}
void SatFeedControl(String satFeed){ //Controla los datos (el feed) que llega del satelite 
   if(satFeed == "Heartbeat"){ 
      heartbeat = true;
      digitalWrite(heartbeatLED,HIGH); // Enciende el LED de señal de vida
      lastHeartbeatTurnOFF = millis();
   }else if(satFeed == "ErrorTH"){ 
      errorTH = true; // Da la señal de error del sensor DHT
   } else {
      errorTH = false;
      Serial.println(satFeed);//muestra los datos recibidos
   }
}
void ErrorCheck(){ //Comprueba si hay alguna flag de un error activa
   if(errorTH){ // Comprueba si el flag de error del sensor de temperatura y enciende el LED correspondiente
      digitalWrite(ErrorTHLED,HIGH);
   } else {
      digitalWrite(ErrorTHLED,LOW);
   }
}