#include <SoftwareSerial.h> 


// Define the pins for SoftwareSerial
#define SIM800L_RX 10
#define SIM800L_TX 11

// Create a SoftwareSerial object
SoftwareSerial sim800l(SIM800L_RX, SIM800L_TX);

void setup() {
  // Initialize Serial Monitor for debugging
  Serial.begin(9600);
  while (!Serial);

  // Initialize SIM800L communication
  sim800l.begin(9600);
  delay(1000);

  Serial.println("Initializing SIM800L...");

  // Check if the module is ready
  sim800l.println("AT");
  delay(1000);
  while (sim800l.available()) {
    Serial.write(sim800l.read());
  }

  // Configure the module to text mode
  sim800l.println("AT+CMGF=1");
  delay(1000);
  while (sim800l.available()) {
    Serial.write(sim800l.read());
  }

  // Set the module to show incoming SMS directly
  sim800l.println("AT+CNMI=2,2,0,0,0");
  delay(1000);
  while (sim800l.available()) {
    Serial.write(sim800l.read());
  }

  Serial.println("SIM800L is ready to receive SMS...");
}

void loop() {

  // Check if there is any data available from the SIM800L module
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
      if (smsContent.indexOf("LOCATION") != -1) {
        Serial.println("Fetching GPS Location...");
      }
    }
  }
}