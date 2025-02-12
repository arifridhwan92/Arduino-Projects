#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Define pins for SoftwareSerial (SIM800L and GPS)
#define SIM800L_RX 10
#define SIM800L_TX 11
#define GPS_RX 2
#define GPS_TX 3

// Create SoftwareSerial objects for SIM800L and GPS
SoftwareSerial sim800l(SIM800L_RX, SIM800L_TX);
SoftwareSerial gpsSerial(GPS_RX, GPS_TX);

// Create a TinyGPS++ object for parsing GPS data
TinyGPSPlus gps;

// Initialize I2C LCD (16x2)
LiquidCrystal_I2C lcd(0x27, 16, 2); // Change the address (0x27) if necessary

// Replace with the recipient's phone number
const String PHONE_NUMBER = "+60163053106"; // Use international format

void setup() {
  // Initialize Serial Monitor for debugging
  Serial.begin(9600);
  while (!Serial);

  // Initialize SIM800L communication
  sim800l.begin(9600);
  delay(1000);

  // Initialize GPS communication
  gpsSerial.begin(9600);
  delay(1000);

  // Initialize I2C LCD
  lcd.init();
  lcd.backlight(); // Turn on the backlight
  lcd.print("Initializing...");
  delay(2000);
  lcd.clear();

  Serial.println("Initializing SIM800L and GPS...");

  // Check if the SIM800L module is ready
  sim800l.println("AT");
  delay(1000);
  while (sim800l.available()) {
    Serial.write(sim800l.read());
  }

  // Configure the SIM800L module to text mode
  sim800l.println("AT+CMGF=1");
  delay(1000);
  while (sim800l.available()) {
    Serial.write(sim800l.read());
  }

  // Set the SIM800L module to show incoming SMS directly
  sim800l.println("AT+CNMI=2,2,0,0,0");
  delay(1000);
  while (sim800l.available()) {
    Serial.write(sim800l.read());
  }

  Serial.println("SIM800L and GPS are ready...");
  lcd.setCursor(0, 0);
  lcd.print("Ready!");
  delay(2000);
  lcd.clear();
}

void loop() {
  // Process GPS data
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  // Continuously display GPS location on LCD
  if (gps.location.isValid()) {
    displayGPSOnLCD();
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("No GPS Signal!");
    delay(3000);
  }

  // Check for incoming SMS
  if (sim800l.available()) {
    String response = sim800l.readString();
    Serial.println(response);  // Print the response to the Serial Monitor

    // Check if the response contains an SMS
    if (response.indexOf("+CMT:") != -1) {
      Serial.println("New SMS Received!");
      // Extract and print the SMS content
      int index = response.indexOf("\r\n");
      String smsContent = response.substring(index + 2);
      Serial.println("SMS Content: " + smsContent);

      // If the SMS contains the keyword "LOCATION", send current GPS location
      if (smsContent.indexOf("LOCATION") != -1) {
        Serial.println("Fetching GPS Location...");
        if (gps.location.isValid()) {
          String gpsLocation = "Lat: " + String(gps.location.lat(), 6) + "\n";
          gpsLocation += "Lng: " + String(gps.location.lng(), 6);
          Serial.println("GPS Location: " + gpsLocation);
          // Send the GPS location as an SMS reply
          sendSMS(PHONE_NUMBER, gpsLocation);
        } else {
          Serial.println("GPS Location not available.");
          sendSMS(PHONE_NUMBER, "GPS Location not available.");
        }
      }
    }
  }
}

// Function to send SMS
void sendSMS(String phoneNumber, String message) {
  sim800l.println("AT+CMGS=\"" + phoneNumber + "\"");  // Set the recipient's phone number
  delay(1000);
  sim800l.print(message);  // Send the message
  delay(1000);
  sim800l.write(26);  // Send Ctrl+Z to indicate the end of the message
  delay(1000);
  Serial.println("SMS Sent to " + phoneNumber + ": " + message);
}

// Function to display GPS location on LCD
void displayGPSOnLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Lat: " + String(gps.location.lat(), 6));
  lcd.setCursor(0, 1);
  lcd.print("Lng: " + String(gps.location.lng(), 6));
}