#include <SoftwareSerial.h>
// Variables globales
const unsigned long heartbeatLEDInterval = 1000;
const int heartbeatLED = 7;
const int ErrorTHLED = 8;

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
   pinMode(heartbeatLED,OUTPUT);
   pinMode(ErrorTHLED,OUTPUT);
}
void loop() {
   GroundCMDControl(); // Llama a la funcion que lee los comandos de la GUI de python
   if (mySerial.available()>0) {//comprueba si hay datos por leer
      String satFeed = mySerial.readString();//lectura datos
      SatFeedControl(); // Llama a la funcion que se encarga de gestionar los datos (el feed) que viene del satelite
      if(heartbeat = true && (millis()-lastHeartbeatTurnOFF >= heartbeatLEDInterval)){
         heartbeat = false;
         digitalWrite(heartbeatLED,LOW);
      }
      ErrorCheck();
   }
}
void GroundCMDControl(){
   char GroundCMD = Serial.read();
   if (mensaje == 'i' || mensaje == 'r'){ //Si recibe de la interfaz 'i' (Iniciar) o 'r' (Reanudar), envía una "i" al emisor
      mySerial.println("i");
   } else if (mensaje == 'p'){ //Si recibe de la interfaz 'p' (Parar), envía una "p" al emisor
      mySerial.println("p");
   }
}
void SatFeedControl(){ //Controla los datos (el feed) que llega del satelite
   if(satFeed == "Heartbeat"){ 
      heartbeat = true;
      digitalWrite(heartbeatLED,HIGH);
      lastHeartbeatTurnOFF = millis();
   }(satFeed == "ErrorTH"){ 
      errorTH = true;
   } else {
      errorTH = false;
      Serial.print(data);//muestra los datos recibidos
   }
}
void ErrorCheck(){ //Comprueba si hay alguna flag de un error activa
   if(errorTH = true){ // Comprueba si el flag de error del sensor de temperatura y enciende el LED correspondiente
      digitalWrite(ErrorTHLED,HIGH);
   } else {
      digitalWrite(ErrorTHLED,LOW);
   }
}