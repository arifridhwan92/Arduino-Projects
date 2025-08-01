/*
  Aquaponics Automation System (V4.1 - L293D Driver with Digital GND)
  
  This sketch controls a simple aquaponics setup using an Arduino Uno.
  This version is configured to use a standard L293D motor driver module
  and uses digital pin 12 as a ground for the LED.
  
  Features:
  1. Water Level Sensing:
     - Uses an Analog Water Level Sensor to monitor the water level.
     - If the water level falls below a set threshold (20% full), it deactivates the water pump and sounds an alarm.
  2. Pump Control:
     - An L293D Motor Driver module controls a 12V DC water pump.
     - The pump runs continuously unless the water level is critically low.
  3. Automated Lighting:
     - Uses a DS3231 Real-Time Clock (RTC) module to track time.
     - An LED grow light is switched on at 8:00 PM (20:00) and off at 6:30 AM.
  4. Alarm System:
     - A buzzer activates when the water level is too low.

  Hardware Required:
  - Arduino Uno
  - Analog Water Level Sensor
  - L293D Motor Driver Module
  - 12V DC Water Pump
  - 12V Power Supply (for the L293D and pump)
  - DS3231 RTC Module
  - Active or Passive Buzzer
  - LED (or a relay to control a larger grow light)
  - Jumper Wires
  - Breadboard

  Wiring Connections:
  - Analog Water Level Sensor:
    - S (Signal) -> A0 on Arduino
    - + (VCC)   -> 5V on Arduino
    - - (GND)   -> GND on Arduino
  - L293D Motor Driver Module:
    - VCC / Vmotor -> Positive (+) terminal of your 12V Power Supply
    - 5V / Vlogic  -> 5V on Arduino
    - GND          -> Negative (-) terminal of your 12V Power Supply AND a GND pin on Arduino (for common ground)
    - ENA (Enable A) -> Pin 3 (PWM Pin) on Arduino
    - IN1            -> Pin 7 on Arduino
    - IN2            -> Pin 5 on Arduino
    - OUT1 & OUT2    -> Connect to your 12V DC Water Pump terminals
  - Buzzer:
    - Positive (+) -> Pin 4
    - Negative (-) -> GND on Arduino
  - LED:
    - Positive (long leg) -> Pin 6
    - Negative (short leg) -> Pin 12 (This pin is configured as a virtual GND)
  - RTC Module (DS3231):
    - VCC -> 5V on Arduino
    - GND -> GND on Arduino
    - SDA -> A4 (SDA) on Arduino
    - SCL -> A5 (SCL) on Arduino

  Libraries to Install:
  - "RTClib" by Adafruit. You can install this from the Arduino IDE Library Manager.
*/

// Include necessary libraries
#include <Wire.h>
#include "RTClib.h"

// Create an RTC object
RTC_DS3231 rtc;

// --- Pin Definitions ---
const int PUMP_IN1_PIN = 3;  // L293D Motor Driver Input 1
const int PUMP_IN2_PIN = 4;  // L293D Motor Driver Input 2
const int PUMP_ENABLE_PIN = 5; // L293D Enable Pin for Motor A (must be PWM)
const int PUMP_IN3_PIN = 8;  // L293D Motor Driver Input 1
const int PUMP_IN4_PIN = 7;  // L293D Motor Driver Input 2
const int ENABLEB_PIN = 6; // L293D Enable Pin for Motor A (must be PWM)
const int BUZZER_PIN = 11;     // Pin connected to the buzzer
const int LED_PIN = 13;        // Pin connected to the LED grow light (provides power)
const int LED_GND_PIN = 12;   // Digital pin used as ground for the LED
const int WATER_LEVEL_PIN = A1; // Analog pin for the water level sensor

// --- System Configuration ---
// Water Level Sensor Calibration
// IMPORTANT: You must calibrate these values for your sensor.
// 1. With the sensor completely dry, check the value printed in the Serial Monitor. This is your DRY_VALUE.
// 2. With the sensor submerged to the "full" level, check the value. This is your WET_VALUE.
const int SENSOR_DRY_VALUE = 0;    // Raw analog reading when sensor is dry (default: 0)
const int SENSOR_WET_VALUE = 600;  // Raw analog reading when sensor is at max water level (e.g., 600)
const int LOW_WATER_THRESHOLD_PERCENT = 20; // Alert when water is below 20%

// --- Global Variables ---
int waterLevelPercent;
int rawWaterLevel;

void setup() {
  // Start serial communication for debugging purposes
  Serial.begin(9600);
  Serial.println("Aquaponics Control System Initializing...");

  // Initialize the RTC module
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC! Check wiring.");
    while (1); // Halt the program if RTC is not found
  }

  // NOTE: The time should be set using a separate sketch.
  // This sketch will now READ the time from the RTC, not set it.

  // Set pin modes
  pinMode(PUMP_IN1_PIN, OUTPUT);
  pinMode(PUMP_IN2_PIN, OUTPUT);
  pinMode(PUMP_ENABLE_PIN, OUTPUT);
  pinMode(PUMP_IN3_PIN, OUTPUT);
  pinMode(PUMP_IN4_PIN, OUTPUT);
  pinMode(ENABLEB_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_GND_PIN, OUTPUT); // Set pin 12 as an output

  // Set initial states
  digitalWrite(BUZZER_PIN, LOW);    // Buzzer off
  digitalWrite(LED_PIN, LOW);       // LED off
  pinMode(LED_GND_PIN, OUTPUT); // Set pin 12 as an output   // Set pin 12 to LOW to act as ground
  
  // Stop the pump initially
  digitalWrite(PUMP_IN1_PIN, LOW);
  digitalWrite(PUMP_IN2_PIN, LOW);
  analogWrite(PUMP_ENABLE_PIN, 0); // Disable motor output
  digitalWrite(PUMP_IN3_PIN, LOW);
  digitalWrite(PUMP_IN4_PIN, LOW);
  analogWrite(ENABLEB_PIN, 0); // Disable motor output

  Serial.println("Initialization complete. System is running.");
}

void loop() {
  // --- Main Tasks ---
  // 1. Read the water level
  readWaterLevel();
  
  // 2. Control the pump and buzzer based on water level
  controlPumpAndBuzzer();

  // 3. Control the LED light based on the time from the RTC
  controlLighting();

  // Print a status update to the Serial Monitor for debugging
  printStatus();

  // Wait for 2 seconds before the next loop
  delay(2000);
}

// --- Function to read water level from the analog sensor ---
void readWaterLevel() {
  // Read the raw analog value from the sensor
  rawWaterLevel = analogRead(WATER_LEVEL_PIN);

  // Map the raw value to a percentage (0-100%)
  waterLevelPercent = map(rawWaterLevel, SENSOR_DRY_VALUE, SENSOR_WET_VALUE, 0, 100);

  // Constrain the value between 0 and 100 to handle potential errors
  waterLevelPercent = constrain(waterLevelPercent, 0, 100);
}

// --- Function to control the pump and buzzer ---
void controlPumpAndBuzzer() {
  if (waterLevelPercent < LOW_WATER_THRESHOLD_PERCENT) {
    // Water level is LOW
    Serial.println("ALERT: Water level is low!");
    
    // Turn the pump OFF by disabling the motor channel
    analogWrite(PUMP_ENABLE_PIN, 0);
    
    // Turn the buzzer ON
    digitalWrite(BUZZER_PIN, HIGH);
    
  } else {
    // Water level is OK
    
    // Set pump direction to forward
    digitalWrite(PUMP_IN1_PIN, HIGH);
    digitalWrite(PUMP_IN2_PIN, LOW);
    // Turn the pump ON at full speed
    analogWrite(PUMP_ENABLE_PIN, 255); 
    
    // Turn the buzzer OFF
    digitalWrite(BUZZER_PIN, LOW);
  }
}

// --- Function to control the lighting schedule ---
void controlLighting() {
  // Get the current time from the RTC
  DateTime now = rtc.now();

  // Define on/off times in minutes from midnight
  const int lightOnTimeMinutes = 8 * 60;      // 8:00 PM = 1200 minutes
  const int lightOffTimeMinutes = 16 * 60; // 4:00 PM = 390 minutes

  // Get the current time in minutes from midnight
  int currentTimeMinutes = now.hour() * 60 + now.minute();
  
  // Logic for overnight lighting
  if (currentTimeMinutes >= lightOnTimeMinutes || currentTimeMinutes < lightOffTimeMinutes) {
    // Time to turn the light ON
    digitalWrite(LED_PIN, HIGH);
  } else {
    // Time to turn the light OFF
    digitalWrite(LED_PIN, LOW);
  }
}


// --- Function to print system status to Serial Monitor ---
void printStatus() {
  DateTime now = rtc.now();
  
  Serial.print("----------------------\n");
  Serial.print("Time: ");
  Serial.print(now.year(), DEC); Serial.print('/');
  Serial.print(now.month(), DEC); Serial.print('/');
  Serial.print(now.day(), DEC); Serial.print(" ");
  Serial.print(now.hour(), DEC); Serial.print(':');
  if(now.minute() < 10) Serial.print('0');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  if(now.second() < 10) Serial.print('0');
  Serial.print(now.second(), DEC);
  Serial.println();

  Serial.print("Raw Water Sensor: ");
  Serial.print(rawWaterLevel); // Print raw value for calibration
  Serial.print(" | Water Level: ");
  Serial.print(waterLevelPercent);
  Serial.println("%");

  Serial.print("Pump Status: ");
  // Pump is ON if the enable pin has a value greater than 0
  Serial.println(analogRead(PUMP_ENABLE_PIN) > 0 ? "ON" : "OFF");

  Serial.print("Light Status: ");
  Serial.println(digitalRead(LED_PIN) == HIGH ? "ON" : "OFF");
  Serial.print("----------------------\n\n");
}
