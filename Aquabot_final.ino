#include <SoftwareSerial.h>
#include <HUSKYLENS.h>
#include <WiFiS3.h>      // για το REV4
#include <WiFiUdp.h>
#include "arduino_secrets.h"

// ————— WiFi & UDP CLIENT CONFIG —————
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

WiFiUDP Udp;
unsigned int localPort = 2394;
IPAddress serverIp(192, 168, 191, 102); // Server IP
unsigned int serverPort = 2390;
char incomingPacket[256];

// — motor driver pins —
const uint8_t IN1 = 2;
const uint8_t IN2 = 3;
const uint8_t IN3 = 4;
const uint8_t IN4 = 5;
const uint8_t ENA = 9;  // PWM for Motor A
const uint8_t ENB = 10; // PWM for Motor B

HUSKYLENS huskylens;
SoftwareSerial mySerial(A2, A1); // RX, TX

// your trained IDs:
const int plasticID   = 1;
const int humanID     = 2;
const int motorSpeed  = 255; // 0–255 PWM

// ── Function prototypes ─────────────────────────────────────────
void moveForward(uint8_t speed);
void stopMotors();
void printResult(HUSKYLENSResult result);

// ── setup() ─────────────────────────────────────────────────────
void setup() {
  // Motor pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  Serial.begin(115200);
  mySerial.begin(9600);

  // Initialize HuskyLens
  while (!huskylens.begin(mySerial)) {
    Serial.println(F("Begin failed!"));
    Serial.println(F("1. Check Protocol Type on HUSKYLENS (Serial 9600)"));
    Serial.println(F("2. Check the connection."));
    delay(100);
  }

  // Connect to WiFi
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    delay(1000);
  }
  Serial.print("Client R4 IP: ");
  Serial.println(WiFi.localIP());

  // Start UDP
  Udp.begin(localPort);
}

// ── loop() ──────────────────────────────────────────────────────
void loop() {
  // —— UDP CLIENT COMMUNICATION ——
  // Send message to server
  String msg = "R4;0;0;0;0;0";
  Udp.beginPacket(serverIp, serverPort);
  Udp.write(msg.c_str());
  Udp.endPacket();

  // Check for response
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    int len = Udp.read(incomingPacket, 255);
    if (len > 0) incomingPacket[len] = '\0';
    Serial.print("Received from Server: ");
    Serial.println(incomingPacket);
  }

  // —— HUSKYLENS OBJECT DETECTION ——
  bool seePlastic = false;
  bool seeHuman   = false;

  if (!huskylens.request()) {
    Serial.println(F("Fail to request data from HUSKYLENS"));
  } else if (!huskylens.isLearned()) {
    Serial.println(F("Nothing learned, press learn button on HUSKYLENS!"));
  } else if (!huskylens.available()) {
    Serial.println(F("No block or arrow appears on the screen!"));
  } else {
    HUSKYLENSResult result;
    while (huskylens.available()) {
      result = huskylens.read();
      printResult(result);
    }

    int detectedID = result.ID;
    if (detectedID == plasticID) seePlastic = true;
    if (detectedID == humanID) seeHuman = true;

    if (seeHuman)        stopMotors();
    else if (seePlastic) moveForward(motorSpeed);
    else                 stopMotors();

    delay(50);
  }
}

// ── printResult() ───────────────────────────────────────────────
void printResult(HUSKYLENSResult result) {
  if (result.command == COMMAND_RETURN_BLOCK || result.command == COMMAND_RETURN_ARROW) {
    Serial.println(result.ID);
  } else {
    Serial.println(F("Object unknown!"));
  }
}

// ── moveForward() ──────────────────────────────────────────────
void moveForward(uint8_t speed) {
  // Motor A forward
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  // Motor B forward
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
}

// ── stopMotors() ───────────────────────────────────────────────
void stopMotors() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}
