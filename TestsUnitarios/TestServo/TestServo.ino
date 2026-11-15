/*  SG90 + angle telemetry
 *  Sends the current angle (0–180) as an ASCII number followed by '\n'.
 *  Works at 9600 baud (change if you prefer).
 */

#include <Servo.h>

Servo myservo;
int angle  = 0;   // current angle
int step   = 1;   // direction of motion

void setup() {
  Serial.begin(9600);      // <-- match this in Python
  myservo.attach(9);       // SG90 PWM pin -> Arduino pin 9
  delay(2000);             // give the PC time to open the port
}

void loop() {
  myservo.write(angle);          // move servo
  Serial.println(angle);         // <-- **newline!**  <-- send angle

  angle += step;
  if (angle >= 180 || angle <= 0) step = -step;   // bounce

  delay(20);   // 20 ms → 50 Hz update rate
}