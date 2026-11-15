/*
 * Ultrasonic Sensor (HC-SR04) Test Sketch
 * Measures distance and prints it to the Serial Monitor.
 */

// Define pins
const int trigPin = 10;
const int echoPin = 9;

// Variables for calculations
long duration;
float distanceCm;
float distanceInch;

void setup() {
  // Set pin modes
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);  // Sets the echoPin as an Input
  
  // Initialize Serial communication
  Serial.begin(9600); 
  Serial.println("Ultrasonic Sensor Test Initialized...");
  delay(500);
}

void loop() {
  // 1. Clear the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // 2. Trigger the sensor by sending a 10 microsecond pulse
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // 3. Read the echoPin (returns the sound wave travel time in microseconds)
  duration = pulseIn(echoPin, HIGH);

  // 4. Calculate the distance
  // Speed of sound is ~340 m/s or 0.034 cm/uS
  // We divide by 2 because the sound travels to the object and back
  distanceCm = duration * 0.034 / 2;
  distanceInch = duration * 0.0133 / 2;

  // 5. Print the results to the Serial Monitor
  if (duration == 0) {
    Serial.println("Out of range / No sensor detected!");
  } else {
    Serial.print("Distance: ");
    Serial.print(distanceCm);
    Serial.print(" cm  |  ");
    Serial.print(distanceInch);
    Serial.println(" in");
  }

  // Wait a bit before the next measurement
  delay(200);
}