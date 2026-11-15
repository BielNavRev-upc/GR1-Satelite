#include <Servo.h>

// --- Pin Definitions ---
const int SERVO_PIN = 3;
const int TRIG_PIN = 6;
const int ECHO_PIN = 6;

// --- Constants for Servo ---
const int MIN_ANGLE = 0;
const int MAX_ANGLE = 180;
const int ANGLE_STEP = 1;
const int SWEEP_DELAY = 15; // Delay between servo movements in milliseconds

// --- Constants for Ultrasonic Sensor ---
// The speed of sound is 343 m/s or 29.1 microseconds per centimeter.
// The pulseIn() function measures the round trip time.
// So, the distance in cm is (duration / 2) / 29.1 = duration / 58.2
const float SOUND_SPEED_FACTOR = 58.2; 

Servo myServo;

void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  myServo.attach(SERVO_PIN);
  Serial.begin(9600);
}

void loop() {
  // Sweep from MIN_ANGLE to MAX_ANGLE
  sweepAndMeasure(MIN_ANGLE, MAX_ANGLE, ANGLE_STEP);
  
  // Sweep back from MAX_ANGLE to MIN_ANGLE
  sweepAndMeasure(MAX_ANGLE, MIN_ANGLE, -ANGLE_STEP);
}

/**
 * @brief Sweeps the servo motor and measures the distance at each step.
 * @param startAngle The starting angle of the sweep.
 * @param endAngle The ending angle of the sweep.
 * @param step The increment or decrement for the angle.
 */
void sweepAndMeasure(int startAngle, int endAngle, int step) {
  for (int angle = startAngle; (step > 0) ? (angle <= endAngle) : (angle >= endAngle); angle += step) {
    myServo.write(angle);
    delay(SWEEP_DELAY);
    int distance = calculateDistance();
    printData(angle, distance);
  }
}

/**
 * @brief Calculates the distance using the ultrasonic sensor.
 * @return The calculated distance in centimeters.
 */
int calculateDistance() {
  // Trigger the ultrasonic sensor
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Read the echo pulse
  long duration = pulseIn(ECHO_PIN, HIGH);

  // Calculate the distance in centimeters
  return static_cast<int>(duration / SOUND_SPEED_FACTOR);
}

/**
 * @brief Prints the angle and distance data to the Serial Monitor.
 * @param angle The current angle of the servo.
 * @param distance The measured distance.
 */
void printData(int angle, int distance) {
  Serial.print(angle);
  Serial.print(",");
  Serial.print(distance);
  Serial.println(".");
}