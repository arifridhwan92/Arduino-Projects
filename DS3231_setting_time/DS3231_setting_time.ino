/*
  Set DS3231 Time

  This is a simple sketch to manually set the date and time on a DS3231
  Real-Time Clock (RTC) module.

  Instructions:
  1. Make sure your DS3231 RTC is wired correctly to your Arduino
     (VCC->5V, GND->GND, SDA->A4, SCL->A5).
  2. Change the values in the rtc.adjust() line below to the desired date and time.
  3. Upload this sketch to your Arduino.
  4. Open the Serial Monitor to see the confirmation.
  5. Once the time is set, you can re-upload your main project sketch. The RTC
     will remember the time thanks to its backup battery.
*/

// Include necessary libraries
#include <Wire.h>
#include "RTClib.h"

// Create an RTC object
RTC_DS3231 rtc;

void setup() {
  // Start serial communication
  Serial.begin(9600);

  // A small delay to ensure the serial monitor can connect in time
  delay(1000); 

  // Initialize the RTC module
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC! Check wiring or battery.");
    while (1); // Halt if the RTC is not found
  }

  // --- SET THE TIME HERE ---
  // The line below sets the RTC to a specific date and time.
  // DateTime format: (Year, Month, Day, Hour, Minute, Second)
  // Hour is in 24-hour format.
  
  // Set to: July 23, 2025, 2:34 PM (14:34)
  rtc.adjust(DateTime(2025, 7, 23, 14, 41, 0));
  
  Serial.println("RTC time has been set!");
  Serial.println("You can now upload your main project sketch.");
}

void loop() {
  // Get the current time from the RTC
  DateTime now = rtc.now();
  
  // Print the current time to the Serial Monitor every 3 seconds
  // to confirm it was set correctly and is running.
  Serial.print("Current Time: ");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  if(now.minute() < 10) Serial.print('0'); // Add leading zero for minutes
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  if(now.second() < 10) Serial.print('0'); // Add leading zero for seconds
  Serial.print(now.second(), DEC);
  Serial.println();

  delay(3000);
}
