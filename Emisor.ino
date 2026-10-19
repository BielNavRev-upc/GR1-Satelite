const int led1=12;
bool enviar = false;
const long interval1=1000;
bool stateLed1=LOW;

#include <SoftwareSerial.h>
#include <DHT.h>
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
SoftwareSerial mySerial(10, 11); // RX, TX 
void setup() {
   pinMode(led1, OUTPUT);
   //nextMillis1 = millis() + interval1;
   mySerial.begin(9600);
   mySerial.println("Empezamos");
   Serial.begin(9600);
   // Inicialización del sensor
   dht.begin();
}
void loop() {
   
        // Ya toca cambiar
        if (stateLed1 == LOW)
             stateLed1 = HIGH;
        else
             stateLed1 = LOW;
        digitalWrite(led1, stateLed1);
        // Siguiente instante en el que habrá que cambiar
        //nextMillis1 = millis() + interval1;
   
   char mensaje = mySerial.read();
   if(mensaje == 'i'){
      enviar = true;
   } else if (mensaje == 'p'){
      enviar = false;
   }
   if(enviar){
      float h = dht.readHumidity();
      float t = dht.readTemperature();
      if (isnan(h) || isnan(t))
         Serial.println("Error al leer el sensor DHT11");
      else {
         mySerial.print(t);
         mySerial.print(":");
         mySerial.println(h);
      }
   }
   delay(1000);
}