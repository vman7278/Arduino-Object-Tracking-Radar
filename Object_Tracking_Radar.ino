/*
 * Arduino Object-Tracking Radar
 * SG90 servo: signal D4, power 5V, ground GND
 * HC-SR04: TRIG D9, ECHO D10, power 5V, ground GND
 * Serial Monitor: 9600 baud
 * z = enter zero mode, +/- = adjust center, r = resume
 */
#include <Servo.h>

Servo radar;

const int trigPin = 9;
const int echoPin = 10;
const int servoPin = 4;

const int minAngle = 20;
const int maxAngle = 160;

// The calibrated physical servo angle corresponding to straight ahead.
// Change this default after finding your preferred calibration.
int zeroPosition = 90;
bool zeroMode = false;

int targetAngle = 90;
int missed = 0;
bool locked = false;

void pointServo(int angle) {
  int actualAngle = angle + (zeroPosition - 90);
  radar.write(constrain(actualAngle, 0, 180));
}

void checkCommands() {
  while (Serial.available()) {
    char command = Serial.read();

    if (command == 'z' || command == 'Z') {
      zeroMode = true;
      pointServo(90);
      Serial.println("ZERO MODE: +/- to adjust; r to resume");
    } else if (command == '+' && zeroMode) {
      // Limited offset keeps the 20-160 sweep within servo travel.
      zeroPosition = min(110, zeroPosition + 1);
      pointServo(90);
      Serial.print("Center: ");
      Serial.println(zeroPosition);
    } else if (command == '-' && zeroMode) {
      zeroPosition = max(70, zeroPosition - 1);
      pointServo(90);
      Serial.print("Center: ");
      Serial.println(zeroPosition);
    } else if (command == 'r' || command == 'R') {
      zeroMode = false;
      locked = false;
      missed = 0;
      targetAngle = 90;
      Serial.println("Tracking resumed");
    }
  }
}

float distanceAt(int angle) {
  pointServo(angle);
  delay(75);

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  unsigned long duration = pulseIn(echoPin, HIGH, 12000);
  if (duration == 0) return -1;

  float distance = duration * 0.0343 / 2.0;
  if (distance < 8 || distance > 65) return -1;

  return distance;
}

int findTarget() {
  int bestAngle = -1;
  float closest = 1000;

  for (int angle = minAngle; angle <= maxAngle; angle += 8) {
    float d = distanceAt(angle);
    if (d > 0 && d < closest) {
      closest = d;
      bestAngle = angle;
    }
  }

  return bestAngle;
}

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  radar.attach(servoPin);
  pointServo(90);
  delay(1000);

  Serial.println("Radar ready!");
  Serial.println("Commands: z=zero, +/-=adjust, r=resume");
}

void loop() {
  checkCommands();

  if (zeroMode) {
    delay(20);
    return;
  }

  if (!locked) {
    Serial.println("SEARCHING...");
    int found = findTarget();

    if (found == -1) {
      delay(200);
      return;
    }

    targetAngle = found;
    locked = true;
    missed = 0;
  }

  int leftAngle = max(minAngle, targetAngle - 12);
  int rightAngle = min(maxAngle, targetAngle + 12);

  float center = distanceAt(targetAngle);
  float left = distanceAt(leftAngle);
  float right = distanceAt(rightAngle);

  // Preserve the original working left/right tracking behavior.
  if (right > 0 && left < 0) {
    targetAngle = min(maxAngle, targetAngle + 5);
  } else if (left > 0 && right < 0) {
    targetAngle = max(minAngle, targetAngle - 5);
  }

  if (center > 0 || left > 0 || right > 0) {
    missed = 0;
  } else {
    missed++;
  }

  if (missed >= 3) {
    locked = false;
    Serial.println("TARGET LOST");
    return;
  }

  // Obtain a current reading at the estimated tracking direction.
  float currentDistance = distanceAt(targetAngle);

  Serial.print("TRACKING | Angle: ");
  Serial.print(targetAngle);
  Serial.print(" deg | Distance: ");
  if (currentDistance > 0) {
    Serial.print(currentDistance, 1);
    Serial.println(" cm");
  } else {
    Serial.println("no valid center reading");
  }
}
