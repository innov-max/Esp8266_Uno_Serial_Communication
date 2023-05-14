
#include <ESP8266WiFi.h>
#include <Wire.h>

#include <SPI.h>
#include <RFID.h>
#include <FirebaseESP8266.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#define FIREBASE_HOST "rfid-attendance-4a479-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "BIPUdNgoeJgiv7jF6n2AqiyRCQJTNmXs42fPEl8R"

RFID rfid(D8, D0);
unsigned char str[MAX_LEN];

WiFiUDP ntpUDP;
const long utcOffsetInSeconds = 19800;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

const char ssid[] = "TROJAN";
const char pass[] = "Alex@7402";

String uidPath = "/";
FirebaseJson json;
FirebaseData firebaseData;

unsigned long lastMillis = 0;
const int red = D4;
const int green = D3;
String alertMsg;
String device_id = "Engineering";
boolean checkIn = true;

void connect() {
  Serial1.print("Checking WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial1.println("\nConnected!");
}

void setup() {
 Serial1.begin(115200);
  WiFi.begin(ssid, pass);

  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);

  SPI.begin();
  rfid.init();

  randomSeed(analogRead(A0)); // Seed the random number generator

  timeClient.begin();
  timeClient.setTimeOffset(utcOffsetInSeconds);
  connect();
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
 

}

String generateOTP() {
  String otp = "";
  for (int i = 0; i < 4; i++) {
    otp += String(random(0, 10));
  }
  return otp;
}

void checkAccess(String temp) {
  if (Firebase.getInt(firebaseData, uidPath + "/users/" + temp)) {
    if (firebaseData.intData() == 0) {
      alertMsg = "CHECKING IN";
      delay(1000);

      json.add("time", String(timeClient.getFormattedDate()));
      json.add("id", device_id);
      json.add("uid", temp);
      json.add("status", 1);

      Firebase.setInt(firebaseData, uidPath + "/users/" + temp, 1);

      if (Firebase.pushJSON(firebaseData, uidPath + "/attendance", json)) {
        Serial1.println(firebaseData.dataPath() + firebaseData.pushName());
        // Generate OTP
        String otp = generateOTP();
        // Send OTP to Arduino Uno via Serial
        Serial1.print("OTP:");
        Serial1.println(otp);

      } else {
        Serial1.println(firebaseData.errorReason());
      }
    } else if (firebaseData.intData() == 1) {
      alertMsg = "CHECKING OUT";
      delay(1000);

      Firebase.setInt(firebaseData, uidPath + "/users/" + temp, 0);

      json.add("time", String(timeClient.getFormattedDate()));
      json.add("id", device_id);
      json.add("uid", temp);
      json.add("status", 0);
      if (Firebase.pushJSON(firebaseData, uidPath + "/attendance", json)) {
        Serial1.println(firebaseData.dataPath() + firebaseData.pushName());
      } else {
        Serial1.println(firebaseData.errorReason());
      }
    }
  } else {
    Serial1.println("FAILED");
    Serial1.println("REASON: " + firebaseData.errorReason());
  }
}

void loop()
{
  timeClient.update();
  if (rfid.findCard(PICC_REQIDL, str) == MI_OK) {
    Serial1.println("Card found");
    String temp = "";
    if (rfid.anticoll(str) == MI_OK) {
      Serial1.print("The card's ID number is: ");
      for (int i = 0; i < 4; i++) {
        temp = temp + (0x0F & (str[i] >> 4));
        temp = temp + (0x0F & str[i]);
      }
      Serial1.println(temp);
      checkAccess(temp);
    }
    rfid.selectTag(str);
  }
  rfid.halt();
}
