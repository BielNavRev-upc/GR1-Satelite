#include <Servo.h>
#include <DHT.h>
#include <SoftwareSerial.h>

// Definir pines
const int trigPin = 6;
const int echoPin = 5;
const int servoPin = 3;
const int DHTPin = 2;

// Intervalos
const unsigned long intervalRAD = 50; // 50ms cada paso de angulo del servo
const unsigned long intervalHB = 3000; // Heartbeat cada 3s
const unsigned long intervalORB = 1000; // Time in milliseconds between each orbit simulation update
unsigned long intervalDHT = 2000; // Intervalo modificable del DHT

// Variables de contadores
unsigned long LastRAD = 0;
unsigned long LastHB = 0; 
unsigned long LastDHT = 0;
unsigned long LastOrbUpdate = 0;

// Variables del servo
int Angulo = 0;
int Direccion = 1; 
Servo myServo;

// Variables bool globales
bool SendRadar = false;
bool SendDHT = false;
bool calcularEnSatelite = false;

// Medias
float ultimos10Temp[10];
float limiteTempMedia;
int indiceTemp = 0;
int totalTemperaturas = 0;

int contadorMediasAltas = 0;
bool alarmaMediaActiva = false;

//Constantes de orbita
const double G = 6.67430e-11;  // Gravitational constant (m^3 kg^-1 s^-2)
const double M = 5.97219e24;   // Mass of Earth (kg)
const double R_EARTH = 6371000;  // Radius of Earth (meters)
const double ALTITUDE = 400000;  // Altitude of satellite above Earth's surface (meters)
const double EARTH_ROTATION_RATE = 7.2921159e-5;  // Earth's rotational rate (radians/second)
const double  TIME_COMPRESSION = 90.0; // Time compression factor (90x)
const double inclination = 0;
bool ecef = true;

//Variables de orbita
double real_orbital_period;  // Real orbital period of the satellite (seconds)
double r;  // Total distance from Earth's center to satellite (meters)

DHT dht(DHTPin,DHT11);
SoftwareSerial mySerial(10, 11);

void setup() {
  mySerial.begin(9600);
  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  myServo.attach(servoPin);
  myServo.write(Angulo); 
  dht.begin();
}

void loop() {
  unsigned long Now = millis();
  SatComm();
  Heartbeat(Now);
  if (SendDHT){
    EnviarDHT(Now);
  }
  if (SendRadar){
    Radar(Now);
  }
  SimularOrbita(Now, inclination, ecef);
}
void SatComm(){
  if (mySerial.available() > 0) {
    String cmd = mySerial.readStringUntil('\n');
    cmd.trim(); 
    if (cmd.length() >= 3) {
        char id = cmd.charAt(0);       // First char (2 or 6)
        if (id == '2') {
            char accion = cmd.charAt(2);
            if (accion == 'i') SendDHT = true;
            else if (accion == 'p') SendDHT = false;
        } 
        else if (id == '3') limiteTempMedia = cmd.substring(2).toFloat();
        if(id == '4'){
            char accion = cmd.charAt(2);
            if (accion == '1'){ //Si recibe de la interfaz 'i' (Iniciar) o 'r' (Reanudar), envía una "i" al emisor
                calcularEnSatelite = true;
            } else if (accion == '0'){ //Si recibe de la interfaz 'p' (Parar), envía una "p" al emisor
                calcularEnSatelite = false;
            }
        }
        else if (id == '6') {
            char accion = cmd.charAt(2);
            if (accion == 'i') SendRadar = true;
            else if (accion == 'p') SendRadar = false;
        } 
        else if (id == '7'){
            intervalDHT = cmd.substring(2).toInt();
        }
    }
  }
}
// Funciones del DHT
void EnviarDHT(unsigned long Now){
    if (Now - LastDHT >= intervalDHT) {
        LastDHT = Now;
        float h = dht.readHumidity();
        float t = dht.readTemperature();
        if (!isnan(h) && !isnan(t)){
            mySerial.print("2:"); 
            mySerial.print(t);
            mySerial.print(";");
            mySerial.println(h);
            if (calcularEnSatelite){
                ProcesarMediaSatelite(t);
            }
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

// Funciones del Radar 
void Radar(unsigned long Now) {
    if (Now - LastRAD >= intervalRAD) {
        LastRAD = Now;
        
        Angulo += Direccion;
        
        if (Angulo >= 180) {
          Angulo = 180;
          Direccion = -1;
        } else if (Angulo <= 0) {
          Angulo = 0;
          Direccion = 1;
        }
        
        myServo.write(Angulo);
        EnviarRAD(Angulo);
    }
}

void Heartbeat(unsigned long Now) {
  if (Now - LastHB >= intervalHB) {
    LastHB = Now;
    Serial.println("1:");
  }
}

void EnviarRAD(int angulo) {
  float dist = getDistancia();
  mySerial.print("6:");
  mySerial.print(angulo);
  mySerial.print(";");
  mySerial.println(dist);
}

float getDistancia() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long echo = pulseIn(echoPin, HIGH, 25000); 
  
  if (echo == 0) {
    return 0;
  }
  return echo * 0.034 / 2;
}

void SimularOrbita(unsigned long Now, double inclination, int ecef){
  if(Now-LastOrbUpdate>=intervalORB){
    LastOrbUpdate=Now;
    double time = (Now / 1000) * TIME_COMPRESSION;  // Real orbital time
    r = R_EARTH + ALTITUDE;
    real_orbital_period = 2 * PI * sqrt(pow(r, 3) / (G * M));
    double angle = 2 * PI * (time / real_orbital_period);  // Angle in radians
    double x = r * cos(angle);  // X-coordinate (meters)
    double y = r * sin(angle) * cos(inclination);  // Y-coordinate (meters)
    double z = r * sin(angle) * sin(inclination);  // Z-coordinate (meters)

    if (ecef) {
        double theta = EARTH_ROTATION_RATE * time;
        double x_ecef = x * cos(theta) - y * sin(theta);
        double y_ecef = x * sin(theta) + y * cos(theta);
        x = x_ecef;
        y = y_ecef;
    }
    mySerial.print("8:");
    mySerial.print(time);
    mySerial.print(";");
    mySerial.print(x);
    mySerial.print(";");
    mySerial.print(y);
    mySerial.print(";");
    mySerial.println(z);

  }
}