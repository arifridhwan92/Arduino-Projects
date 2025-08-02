/*
  Aquaponics Automation System (V5.2 - Improved Status Reporting)
  
  This sketch controls an aquaponics setup with two pumps using an Arduino Uno.
  This version includes more reliable status tracking and reporting.
  
  Features:
  1. Water Level Sensing:
     - Uses an Analog Water Level Sensor to monitor the water level.
     - If the water level falls below a set threshold (20% full), it deactivates both pumps and sounds an alarm.
  2. Dual Pump Control:
     - An L293D Motor Driver module controls two 12V DC water pumps.
     - The pumps run continuously unless the water level is critically low.
  3. Automated Lighting:
     - Uses a DS3231 Real-Time Clock (RTC) module to track time.
     - An LED grow light is switched on at 8:00 AM and off at 4:00 PM (16:00).
  4. Alarm System:
     - A buzzer activates when the water level is too low.

  Hardware Required:
  - Arduino Uno
  - Analog Water Level Sensor
  - L293D Motor Driver Module
  - Two 12V DC Water Pumps
  - 12V Power Supply (for the L293D and pumps)
  - DS3231 RTC Module
  - Active or Passive Buzzer
  - LED (or a relay to control a larger grow light)
  - Jumper Wires
  - Breadboard

  Wiring Connections:
  - Analog Water Level Sensor:
    - S (Signal) -> A1 on Arduino
    - + (VCC)   -> 5V on Arduino
    - - (GND)   -> GND on Arduino
  - L293D Motor Driver Module:
    - VCC / Vmotor -> Positive (+) terminal of your 12V Power Supply
    - 5V / Vlogic  -> 5V on Arduino
    - GND          -> Negative (-) terminal of your 12V Power Supply AND a GND pin on Arduino (for common ground)
    - ENA (Enable A) -> Pin 5 (PWM Pin) on Arduino
    - IN1            -> Pin 3 on Arduino
    - IN2            -> Pin 4 on Arduino
    - ENB (Enable B) -> Pin 6 (PWM Pin) on Arduino
    - IN3            -> Pin 8 on Arduino
    - IN4            -> Pin 7 on Arduino
    - OUT1 & OUT2    -> Connect to your first 12V DC Water Pump
    - OUT3 & OUT4    -> Connect to your second 12V DC Water Pump
  - Buzzer:
    - Positive (+) -> Pin 11
    - Negative (-) -> GND on Arduino
  - LED:
    - Positive (long leg) -> Pin 13
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
// Pump 1 (Motor A on L293D)
const int PUMP1_IN1_PIN = 3;  // L293D Motor Driver Input 1
const int PUMP1_IN2_PIN = 4;  // L293D Motor Driver Input 2
const int PUMP1_ENABLE_PIN = 5; // L293D Enable Pin for Motor A (must be PWM)

// Pump 2 (Motor B on L293D)
const int PUMP2_IN3_PIN = 8;  // L293D Motor Driver Input 3
const int PUMP2_IN4_PIN = 7;  // L293D Motor Driver Input 4
const int PUMP2_ENABLE_PIN = 6; // L293D Enable Pin for Motor B (must be PWM)

// Other Components
const int BUZZER_PIN = 11;     // Pin connected to the buzzer
const int LED_PIN = 13;        // Pin connected to the LED grow light (provides power)
const int LED_GND_PIN = 12;   // Digital pin used as ground for the LED
const int WATER_LEVEL_PIN = A1; // Analog pin for the water level sensor

// --- System Configuration ---
// Water Level Sensor Calibration
const int SENSOR_DRY_VALUE = 0;    // Raw analog reading when sensor is dry
const int SENSOR_WET_VALUE = 600;  // Raw analog reading when sensor is at max water level
const int LOW_WATER_THRESHOLD_PERCENT = 20; // Alert when water is below 20%

// --- Global State Variables for Accurate Reporting ---
int waterLevelPercent;
int rawWaterLevel;
bool pump1_is_on = false;
bool pump2_is_on = false;
bool light_is_on = false;

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

  // Set pin modes
  pinMode(PUMP1_IN1_PIN, OUTPUT);
  pinMode(PUMP1_IN2_PIN, OUTPUT);
  pinMode(PUMP1_ENABLE_PIN, OUTPUT);
  pinMode(PUMP2_IN3_PIN, OUTPUT);
  pinMode(PUMP2_IN4_PIN, OUTPUT);
  pinMode(PUMP2_ENABLE_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_GND_PIN, OUTPUT); // Set pin 12 as an output

  // Set initial states
  digitalWrite(BUZZER_PIN, LOW);    // Buzzer off
  digitalWrite(LED_PIN, LOW);       // LED off
  digitalWrite(LED_GND_PIN, LOW);   // Set pin 12 to LOW to act as ground
  
  // Stop both pumps initially
  analogWrite(PUMP1_ENABLE_PIN, 0); // Disable motor 1 output
  analogWrite(PUMP2_ENABLE_PIN, 0); // Disable motor 2 output

  Serial.println("Initialization complete. System is running.");
}

void loop() {
  // --- Main Tasks ---
  readWaterLevel();
  controlPumpsAndBuzzer();
  controlLighting();
  printStatus();
  delay(2000);
}

// --- Function to read water level from the analog sensor ---
void readWaterLevel() {
  rawWaterLevel = analogRead(WATER_LEVEL_PIN);
  waterLevelPercent = map(rawWaterLevel, SENSOR_DRY_VALUE, SENSOR_WET_VALUE, 0, 100);
  waterLevelPercent = constrain(waterLevelPercent, 0, 100);
}

// --- Function to control the pumps and buzzer ---
void controlPumpsAndBuzzer() {
  if (waterLevelPercent < LOW_WATER_THRESHOLD_PERCENT) {
    // Water level is LOW
    Serial.println("ALERT: Water level is low! Turning off pumps.");
    
    // Turn both pumps OFF by disabling their channels
    analogWrite(PUMP1_ENABLE_PIN, 0);
    analogWrite(PUMP2_ENABLE_PIN, 0);
    pump1_is_on = false;
    pump2_is_on = false;
    
    // Turn the buzzer ON
    digitalWrite(BUZZER_PIN, HIGH);
    
  } else {
    // Water level is OK
    
    // Set pump 1 direction to forward and turn ON at full speed
    digitalWrite(PUMP1_IN1_PIN, HIGH);
    digitalWrite(PUMP1_IN2_PIN, LOW);
    analogWrite(PUMP1_ENABLE_PIN, 255); 
    pump1_is_on = true;
    
    // Set pump 2 direction to forward and turn ON at full speed
    digitalWrite(PUMP2_IN3_PIN, HIGH);
    digitalWrite(PUMP2_IN4_PIN, LOW);
    analogWrite(PUMP2_ENABLE_PIN, 255);
    pump2_is_on = true;
    
    // Turn the buzzer OFF
    digitalWrite(BUZZER_PIN, LOW);
  }
}

// --- Function to control the lighting schedule (Daytime 8AM - 4PM) ---
void controlLighting() {
  DateTime now = rtc.now();

  // Define on/off times in minutes from midnight for daytime schedule
  const int lightOnTimeMinutes = 8 * 60;   // 8:00 AM = 480 minutes
  const int lightOffTimeMinutes = 16 * 60; // 4:00 PM = 960 minutes

  int currentTimeMinutes = now.hour() * 60 + now.minute();
  
  // Logic for daytime lighting: ON if it's after 8 AM AND before 4 PM
  if (currentTimeMinutes >= lightOnTimeMinutes && currentTimeMinutes < lightOffTimeMinutes) {
    digitalWrite(LED_PIN, HIGH);
    light_is_on = true;
  } else {
    digitalWrite(LED_PIN, LOW);
    light_is_on = false;
  }
}


// --- Function to print system status to Serial Monitor (IMPROVED) ---
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
  Serial.print(rawWaterLevel);
  Serial.print(" | Water Level: ");
  Serial.print(waterLevelPercent);
  Serial.println("%");

  Serial.print("Pump 1 Status: ");
  Serial.println(pump1_is_on ? "ON" : "OFF");

  Serial.print("Pump 2 Status: ");
  Serial.println(pump2_is_on ? "ON" : "OFF");

  Serial.print("Light Status: ");
  Serial.println(light_is_on ? "ON" : "OFF");
  Serial.print("----------------------\n\n");
}
