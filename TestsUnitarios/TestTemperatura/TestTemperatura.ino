#include <SoftwareSerial.h>
#include <DHT.h>
#define DHTPIN 2
#define DHTTYPE DHT11
int nextMillis1;
const long interval1=1000;
int lastSend=0;
DHT dht(DHTPIN, DHTTYPE);
void setup() {
  Serial.begin(9600);
  dht.begin();

}

void loop() {
  
  if(millis()==nextMillis1){
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t))
      Serial.println("Error al leer el sensor DHT11");
  else {
    Serial.print(t);
    Serial.print(":");
    Serial.println(h);
  }
  nextMillis1 = millis() + interval1;
    
  }


 
}
