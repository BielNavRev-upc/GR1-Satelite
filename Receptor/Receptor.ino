#include <SoftwareSerial.h>
// Configura los pines digitales 10 y 11
SoftwareSerial mySerial(10, 11); // RX, TX 
const int ledHeartbeat = 7;
const int ledErrorTH = 8;

unsigned long heartbeatLedStart = 0;
bool heartbeatLED = false;
const unsigned long heartbeatDuration = 1000; // Dura 1s

unsigned long lastErrorHTTime = 0;
const unsigned long errorTimeoutTH = 1000;
void setup() {
   // Inicializa el puerto serie principal
   Serial.begin(9600);
   // Inicializa el puerto serie
   mySerial.begin(9600);
   pinMode(ledHeartbeat,OUTPUT);
   pinMode(ledErrorTH,OUTPUT);
   digitalWrite(ledHeartbeat,LOW);
   digitalWrite(ledErrorTH,LOW);
}
void loop() {
   unsigned long Now = millis();

   SatComm();
   GUICommands();
   updateLED(Now);
}
void SatComm(){
   if(mySerial.available() > 0){
      String infoSat = mySerial.readStringUntil('\n');
      infoSat.trim();
      if(infoSat == "Heartbeat"){
         digitalWrite(ledHeartbeat,HIGH);
         heartbeatLedStart = millis();
         heartbeatLED = true;
      } else if (infoSat == "ErrorTH"){
         lastErrorHTTime = millis();
         digitalWrite(ledErrorTH,HIGH);
      } else {
         Serial.println(infoSat);
      }
   }
}
void GUICommands(){
   if (Serial.available()>0){
      char GUICommand = Serial.read();
      if (GUICommand == 'i' || GUICommand == 'r'){ //Si recibe de la interfaz 'i' (Iniciar) o 'r' (Reanudar), envía una "i" al emisor
         mySerial.println("i");
      } else if (GUICommand == 'p'){ //Si recibe de la interfaz 'p' (Parar), envía una "p" al emisor
         mySerial.println("p");
      }
   }
}
void updateLED(unsigned long Now){
   // Apagar LED del heartbeat despues de 1s
   if (heartbeatLED && (Now-heartbeatLedStart >= heartbeatDuration)){
      digitalWrite(ledHeartbeat,LOW);
      heartbeatLED = false;
   }

   // Apagar LED del Error si no llega ErrorTH
   if(digitalRead(ledErrorTH)==HIGH){
      if(Now-lastErrorHTTime > errorTimeoutTH){
         digitalWrite(ledErrorTH, HIGH);
      }
   }
}