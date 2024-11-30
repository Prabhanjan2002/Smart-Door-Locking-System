#include "BluetoothSerial.h" // Include Bluetooth library
#include <Stepper.h> // Include Stepper library

// Bluetooth setup
BluetoothSerial SerialBT; // BluetoothSerial instance

// Constants for stepper motors
const float stepAngle = 1.8;
const int stepsPerRevolution = 360 / stepAngle;
Stepper mainStepper(stepsPerRevolution, 14, 27, 12, 26); // Main motor pins
Stepper lockStepper(stepsPerRevolution, 18, 17, 5, 16); // Lock motor pins

// LED pin definitions
const int redPin = 2;
const int greenPin = 0;
const int bluePin = 15;

// Variables to track motor state
int stepperSpeed = 100; // Motor speed in RPM
bool lockStatus = true; // Lock state (true = locked, false = unlocked)
bool doorOpen = false;  // Track door open state

void setup() {
  Serial.begin(115200); 
  SerialBT.begin("ESP32_Door_Lock");

  mainStepper.setSpeed(stepperSpeed);
  lockStepper.setSpeed(stepperSpeed);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin, LOW);
}

void loop() {
  if (Serial.available() > 0) {
    char command = Serial.read();

    switch (command) {
      case 'G': 
        if (!lockStatus) {
          digitalWrite(greenPin, HIGH);
          digitalWrite(redPin, LOW);
          digitalWrite(bluePin, LOW);
        }
        break;

      case 'O': 
        if (!lockStatus) {
          Serial.println("Opening door...");
          for (int i = 0; i < 11; i++) mainStepper.step(stepsPerRevolution / 4); 
          doorOpen = true;
        }
        break;

      case 'C': 
        if (!lockStatus && doorOpen) { 
          Serial.println("Closing door...");
          for (int i = 0; i < 11; i++) mainStepper.step(-stepsPerRevolution / 4);
          doorOpen = false;
        }
        break;

      case 'N': 
        Serial.println("Door is open. Wait to close lock.");
        break;

      default:
        Serial.println("Unknown command received.");
        break;
    }
  }

  if (SerialBT.available()) {
    char btCommand = SerialBT.read();

    switch (btCommand) {
      case '1': 
        if (lockStatus && !doorOpen) { // Only unlock if currently locked and door is closed
          Serial.println("Unlocking door lock...");
          for (int i = 0; i < 12; i++) lockStepper.step(stepsPerRevolution / 4);
          lockStatus = false;
        } else if (!lockStatus) {
          Serial.println("Door is already unlocked.");
        }
        break;

      case '0': 
        if (!lockStatus && !doorOpen) { // Only lock if currently unlocked and door is closed
          Serial.println("Locking door lock...");
          for (int i = 0; i < 12; i++) lockStepper.step(-stepsPerRevolution / 4);
          lockStatus = true;
        } else if (lockStatus) {
          Serial.println("Door is already locked.");
        } else {
          Serial.println("Door is open. Wait to close lock.");
        }
        break;

      default:
        Serial.println("Unknown Bluetooth command received.");
        break;
    }
  }
}