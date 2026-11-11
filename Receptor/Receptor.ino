#include <SoftwareSerial.h>
// Variables globales
const unsigned long heartbeatLEDInterval = 1000;
const unsigned long SendErrorTHInterval = 750;
const unsigned long THErrorTimeout = 5000;
const int heartbeatLED = 7;
const int ErrorTHLED = 8;

bool heartbeat = false;
bool SendTH = false;

unsigned long lastHeartbeatTurnOFF = 0;
unsigned long lastErrorTHRec = 0;
unsigned long lastValidTH = 0;
unsigned long lastErrorTH = 0;

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
void loop(){
   unsigned long Now = millis();
   GroundCMDControl(); // Llama a la funcion que lee los comandos de la GUI de python
   if (mySerial.available()>0) {//comprueba si hay datos por leer
      String satFeed = mySerial.readStringUntil('\n');//lectura datos
      satFeed.trim();
      String ID = satFeed.substring(0,2);
      satFeed.remove(0,2);
      SatFeedControl(ID,satFeed,Now); // Llama a la funcion que se encarga de gestionar los datos (el feed) que viene del satelite
      if(SendTH){
         ErrorTHCheck(Now);
      }
   }
   if(heartbeat && (Now-lastHeartbeatTurnOFF >= heartbeatLEDInterval)){
      heartbeat = false;
      digitalWrite(heartbeatLED,LOW);
   }
}
void GroundCMDControl(){
   String GroundCMD = Serial.readStringUntil('\n');
   GroundCMD.trim();
   if (GroundCMD == "iTH" || GroundCMD == "rTH"){ //Si recibe de la interfaz 'i' (Iniciar) o 'r' (Reanudar), envía una "i" al emisor
      mySerial.println("i");
      SendTH = true;
   } else if (GroundCMD == "pTH"){ //Si recibe de la interfaz 'p' (Parar), envía una "p" al emisor
      mySerial.println("p");
      SendTH = false;
   }
}
void SatFeedControl(String ID,String satFeed,unsigned long Now){ //Controla los datos (el feed) que llega del satelite 
   if(ID == "1:"){
      Serial.println(ID);
      heartbeat = true;
      digitalWrite(heartbeatLED,HIGH);
      lastHeartbeatTurnOFF = Now;
   }
   if((ID == "2:") && SendTH){
      Serial.print(ID);
      Serial.println(satFeed);//muestra los datos recibidos
      lastValidTH = Now;
   }
}
void ErrorTHCheck(unsigned long Now){
   if(Now-lastValidTH >= THErrorTimeout){
      digitalWrite(ErrorTHLED,HIGH);
      if(Now-lastErrorTH >= SendErrorTHInterval){
         Serial.println("2:E");
         lastErrorTH = Now;
      }
   } else {
      digitalWrite(ErrorTHLED,LOW);
   }
}