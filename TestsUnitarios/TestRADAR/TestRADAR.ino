#include <Servo.h>

// Define pins
const int trigPin = 9;
const int echoPin = 10;
const int servoPin = 11;

// Constants
const unsigned long interval = 30; // 30ms between steps

// Variables for timing
unsigned long previousMillis = 0;

// Variables for Servo and Sensor
int currentAngle = 0;
int stepDirection = 1; // 1 for increasing, -1 for decreasing
Servo myServo;

// Function prototypes
float getDistance();
void sendData(int angle);

void setup() {
  Serial.begin(9600);
  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  myServo.attach(servoPin);
  myServo.write(currentAngle); // Start at 0
  
  Serial.println("Angle;Distance(cm)");
}

void loop() {
  // Get the current time
  unsigned long currentMillis = millis();

  // Check if it is time to take a step (every 30ms)
  if (currentMillis - previousMillis >= interval) {
    // Save the last time we performed an action
    previousMillis = currentMillis;

    // 1. Move the Servo
    currentAngle += stepDirection;

    // 2. Check if we hit the limits (0 or 180) and change direction
    if (currentAngle >= 180) {
      currentAngle = 180;
      stepDirection = -1;
    } else if (currentAngle <= 0) {
      currentAngle = 0;
      stepDirection = 1;
    }

    myServo.write(currentAngle);

    // 3. Measure distance and send data
    sendData(currentAngle);
  }

  // Because there are no delay() calls, the code reaches this point 
  // thousands of times per second, leaving room for other tasks.
}

void sendData(int angle) {
  float dist = getDistance();
  Serial.print(angle);
  Serial.print(";");
  Serial.println(dist);
}

float getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // We use a short timeout (25000us) so the sensor doesn't 
  // block the loop for too long if no object is found.
  long duration = pulseIn(echoPin, HIGH, 25000); 
  
  if (duration == 0) {
    return 0;
  }
  
  return duration * 0.034 / 2;
}