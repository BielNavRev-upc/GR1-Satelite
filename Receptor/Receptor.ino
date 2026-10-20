#include <SoftwareSerial.h>
// Configura los pines digitales 10 y 11
SoftwareSerial mySerial(10, 11); // RX, TX (azul, naranja)
void setup() {
   // Inicializa el puerto serie principal
   Serial.begin(9600);
   // Inicializa el puerto serie
   mySerial.begin(9600);
}
void loop() {
   char mensaje = Serial.read();
   if (mensaje == 'i' || mensaje == 'r'){ //Si recibe de la interfaz 'i' (Iniciar) o 'r' (Reanudar), envía una "i" al emisor
      mySerial.println("i");
   } else if (mensaje == 'p'){ //Si recibe de la interfaz 'p' (Parar), envía una "p" al emisor
      mySerial.println("p");
   }
   if (mySerial.available()) {//comprueba si hay datos por leer
      String data = mySerial.readString();//lectura datos
      Serial.print(data);//muestra los datos recibidos
   }
}