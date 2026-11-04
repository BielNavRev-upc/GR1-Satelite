#include <SoftwareSerial.h>
// Configura los pines digitales 10 y 11
SoftwareSerial mySerial(10, 11); // RX, TX 
const int ledHeartbeat = 7;
const int ledErrorTH = 8;

bool errorState = false;
unsigned long heartbeatLedStart = 0;
bool heartbeatLED = false;
const unsigned long heartbeatDuration = 1000; // Dura 1s

unsigned long lastErrorHTTime = 0;
const unsigned long errorTimeoutTH = 5000;
unsigned long lastValidData = 0;

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
         errorState = true;
      } else {
         Serial.println(infoSat);
         lastValidData = millis();
         errorState = false;
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
   bool error = errorState;
   if(!error && (Now-lastValidData > errorTimeoutTH)){
      error = true;
   }
   if(error){
      digitalWrite(ledErrorTH,HIGH);
   } else {
      digitalWrite(ledErrorTH,LOW);
   }
}