// Variables globales
const int led1=12;
bool enviar = false;
const long interval1=1000;
bool stateLed1=LOW;

#include <SoftwareSerial.h>
#include <DHT.h>
// Definición de pines y tipo para el sensor DHT11
#define DHTPIN 2
#define DHTTYPE DHT11
// Instancia del sensor DHT (en el pin 2)
DHT dht(DHTPIN, DHTTYPE);
SoftwareSerial mySerial(10, 11); //// Configuración de puerto serie; RX, TX 
void setup() {
// Configura el pin del LED como salida digital
   pinMode(led1, OUTPUT);
   //nextMillis1 = millis() + interval1;
   // Inicializa el puerto serie por software
   mySerial.begin(9600);
   mySerial.println("Empezamos");
   // Inicializa el puerto serie por hardware
   Serial.begin(9600);
   // Inicialización del sensor
   dht.begin();
}
void loop() {
   
        // Ya toca cambiar, alterna el estado del led
        if (stateLed1 == LOW)
             stateLed1 = HIGH;
        else
             stateLed1 = LOW;
        // Aplica el cambio de estado
        digitalWrite(led1, stateLed1);
        // Siguiente instante en el que habrá que cambiar
        //nextMillis1 = millis() + interval1;
   // Lee si hay algún mensaje de la interfaz
   char mensaje = mySerial.read();
   // Activa el envío de datos si recibe 'i' (Inicio)
   if(mensaje == 'i'){
      enviar = true;
   } else if (mensaje == 'p'){ // Desactiva el envío de datos si recibe 'p' (Parar)
      enviar = false;
   }
   if(enviar){
      float h = dht.readHumidity(); //Lee humedad
      float t = dht.readTemperature(); //Lee temperatura
      if (isnan(h) || isnan(t)) // Comprueba si ha ocurrido algún error al leer
         Serial.println("Error al leer el sensor DHT11");
      else { // Envía los datos
         mySerial.print(t);
         mySerial.print(":");
         mySerial.println(h);
      }
   }
   delay(1000);
}