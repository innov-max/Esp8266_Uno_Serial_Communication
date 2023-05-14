#include <Keypad.h>

const byte ROWS = 4;  // Number of keypad rows
const byte COLS = 3;  // Number of keypad columns

char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

byte rowPins[ROWS] = {8, 7, 6, 5};     // Connect to the row pinouts of the keypad
byte colPins[COLS] = {11, 10, 9};  // Connect to the column pinouts of the keypad

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

const int relayPin = 4;  // Connect the relay to digital pin 4

void setup() {
  Serial.begin(115200);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);  // Initially turn off the relay
}

void loop() {
  char key = keypad.getKey();
  
  if (Serial.available()) {
    String serialData = Serial.readStringUntil('\n');
    if (serialData.startsWith("OTP:")) {
      String otp = serialData.substring(4);
      Serial.println("Received OTP: " + otp);
      // Process the received OTP as needed
    }
  }

  if (key != NO_KEY) {
    if (key == '#') {
      String otp = readOTP();
      if (otp != "") {
        if (validateOTP(otp)) {
          authorizeAccess();
        } else {
          Serial.println("Invalid OTP");
        }
      } else {
        Serial.println("Failed to read OTP");
      }
    }
  }
}

String readOTP() {
  String otp = "";
  while (Serial.available()) {
    char c = Serial.read();
    if (c != '\n') {
      otp += c;
    }
  }
  return otp;
}

bool validateOTP(String otp) {
  // Implement your OTP validation logic here
  return true;
}

void authorizeAccess() {
  digitalWrite(relayPin, HIGH);  // Turn on the relay
  delay(5000);                   // Keep the relay on for 5 seconds
  digitalWrite(relayPin, LOW);   // Turn off the relay
}
