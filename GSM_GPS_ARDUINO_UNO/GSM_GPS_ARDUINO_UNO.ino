#include <SoftwareSerial.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <TinyGPS++.h>

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
// The TinyGPS++ object
TinyGPSPlus gps;

// Define the pins for SoftwareSerial
#define SIM800L_RX 2
#define SIM800L_TX 3

// Create a SoftwareSerial object
SoftwareSerial sim800l(SIM800L_RX, SIM800L_TX);

// Replace with the recipient's phone number
const String PHONE_NUMBER = "+60123456789"; // Use international format


void setup()
{
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  Serial.begin(9600);

  sim800l.begin(9600);
  delay(1000);

  lcd.init();
  lcd.backlight(); // Turn on the backlight
  lcd.print("Initializing...");
  delay(2000);
  lcd.clear();

  // Initialize GSM communication using hardware serial
  Serial.begin(9600); // Use Serial for GSM (TX=1, RX=0 on Arduino Uno)
  delay(1000);

  lcd.setCursor(0, 0);
  lcd.print("Initializing GSM");
  delay(2000);
  lcd.clear();

  // Check if the GSM module is ready
  Serial.println("AT");
  delay(1000);
  lcd.setCursor(0, 0);
  lcd.print("Sending AT...");
  delay(3000);

  lcd.clear();

  // Configure the GSM module to text mode
  Serial.println("AT+CMGF=1");
  delay(1000);
  lcd.setCursor(0, 0);
  lcd.print("Setting Text Mode");
  delay(3000);
  
  lcd.clear();

  // Set the GSM module to show incoming SMS directly
  Serial.println("AT+CNMI=2,2,0,0,0");
  delay(1000);
  lcd.setCursor(0, 0);
  lcd.print("Setting SMS Mode");
  delay(3000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("GSM Ready!");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waiting for GPS!");
}


void loop()
{
   while (Serial.available() > 0){
    gps.encode(Serial.read());
    if (gps.location.isUpdated()){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Lat:" + String(gps.location.lat(), 6)); 
      lcd.setCursor(0,1);
      lcd.print("Lng:" + String(gps.location.lng(), 6)); 
      String latitude = String(gps.location.lat(), 4);
      String longitude = String(gps.location.lng(), 4);

          if (sim800l.available()) {
      String response = sim800l.readString();
    // Check if the response contains an SMS
        
        if (response.indexOf("+CMT:") != -1) {
      // Extract and print the SMS content
          int index = response.indexOf("\r\n");
          String smsContent = response.substring(index + 2);
            if (smsContent.indexOf("LOCATION") != -1) {
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Sending location!");
              delay(3000);
              sim800l.println("AT+CMGF=1");
              delay(1000);
              sim800l.print("AT+CMGS=\"");
              sim800l.print(PHONE_NUMBER);
              sim800l.println("\"");
              delay(1000);
              sim800l.print("The current location is: " + latitude + "," + longitude); // Send formatted message
              delay(1000);
              sim800l.write(26); // Send Ctrl+Z to indicate the end of the message
              delay(1000);
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Location sent!");
      }
    }
        }
              delay(3000);
     //end of gps 
    }
  }
}
