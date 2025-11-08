#include <WiFiS3.h>  // #include <WiFiNINA.h>  για το REV2  &     για το REV4
#include "arduino_secrets.h"
#include <SoftwareSerial.h>
#include <HUSKYLENS.h>

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

WiFiUDP Udp;
unsigned int localPort = 2392;

IPAddress serverIp(192,168,191,102); // Server
unsigned int serverPort = 2390;

char incomingPacket[256];


// ——— PIN CONFIG ———
const int motorPin1      = 2;   // M1 IN1 → Arduino 2
const int motorPin2      = 3;   // M1 IN2 → Arduino 3
const int motorPin3      = 4;   // M2 IN3 → Arduino 4
const int motorPin4      = 5;   // M2 IN4 → Arduino 5
const int motorSpeedPin1 = 9;   // M1 PWM → Arduino 9
const int motorSpeedPin2 = 10;  // M2 PWM → Arduino 10

HUSKYLENS huskylens;
SoftwareSerial mySerial(A2, A1); // RX, TX
//HUSKYLENS green line >> Pin 10; blue line >> Pin 11
void printResult(HUSKYLENSResult result);

// your trained IDs:
const int fullID   = 1;
const int clearID     = 2;

// ——— PHYSICAL/TUNING ———
const float wheelDiameter      = 6.5;                  // cm
const float wheelCircumference = wheelDiameter * 3.1416; // ≈20.4 cm
const int   travelDistance     = 23;                    // cm straight
const unsigned long turnTime   = 1100;                   // ms for ~90° spin (was 150)
const int   motorSpeed1 = 150;  
const int   motorSpeed2 = 175;  
//
const int trigPin = 13;
const int echoPin = 12;

float duration, distance;

// ——— DECLARATIONS ———
void moveForward(int distanceCm);
void pivot90();
void stopMotors();

void setup() {
  Serial.begin(115200);
  mySerial.begin(9600);
  while (!huskylens.begin(mySerial))
   {
       Serial.println(F("Begin failed!"));
       Serial.println(F("1.Please recheck the \"Protocol Type\" in HUSKYLENS (General Settings>>Protocol Type>>Serial 9600)"));
       Serial.println(F("2.Please recheck the connection."));
       delay(100);
    }
  // Begin serial to HuskyLens
 
  Serial.begin(9600);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    delay(1000);
  }
  Serial.print("Client 1 IP: ");
  Serial.println(WiFi.localIP());

  Udp.begin(localPort);

  // direction pins
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorPin3, OUTPUT);
  pinMode(motorPin4, OUTPUT);
  // speed pins
  pinMode(motorSpeedPin1, OUTPUT);
  pinMode(motorSpeedPin2, OUTPUT);

  // set both motors at the same PWM speed
  analogWrite(motorSpeedPin1, motorSpeed1) ;
  analogWrite(motorSpeedPin2, motorSpeed2 );

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

}

void loop() {
  
  bool seefull = false;
  bool seeclear   = false;

      if (!huskylens.request()) Serial.println(F("Fail to request data from HUSKYLENS, recheck the connection!"));
    else if(!huskylens.isLearned()) Serial.println(F("Nothing learned, press learn button on HUSKYLENS to learn one!"));
    else if(!huskylens.available()) Serial.println(F("No block or arrow appears on the screen!"));
    else

{
    HUSKYLENSResult result;  
    while (huskylens.available()) {
        HUSKYLENSResult result = huskylens.read();
        printResult(result);
    }


int detectedID = -1;

detectedID = result.ID;


if ( detectedID == 1) seefull = true;
if ( detectedID == 2) seeclear = true;

  if (seefull)        
    moveForward(travelDistance);
    stopMotors();
  if (seeclear)
  Serial.println("Still Clear");
  else                 stopMotors();
  delay(50);
}

}

void printResult(HUSKYLENSResult result){
    if (result.command == COMMAND_RETURN_BLOCK){
        Serial.println(String()+result.ID);
    }
    else if (result.command == COMMAND_RETURN_ARROW){
        Serial.println(String()+result.ID);
    }
    else{
        Serial.println("Object unknown!");
    }
}

  // Στείλε μήνυμα στον Server
  String msg = "Hello from R2";
  Serial.println(msg);
  Udp.beginPacket(serverIp, serverPort);
  Udp.write(msg.c_str());
  Udp.endPacket();

  // Έλεγχος για απάντηση
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    int len = Udp.read(incomingPacket, 255);
    if (len > 0) incomingPacket[len] = 0;
    Serial.print("Received from Server: ");
    Serial.println(incomingPacket);
  }

  delay(4000);

    Serial.print("Hi");
    delay(5000);
    moveForward(travelDistance);
    stopMotors();
    delay(5000);

  
 

float dist() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = (duration*.0343)/2;
  
  delay(100);
  return distance;
}


void moveForward(int distanceCm) {
  // Motor 1 forward, Motor 2 backward → physical straight
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, HIGH);
  digitalWrite(motorPin3, HIGH);
  digitalWrite(motorPin4, LOW);

  // run long enough to cover distanceCm
  float rotations = distanceCm / wheelCircumference;
  unsigned long runTime = (unsigned long)(rotations * 1000); 
  delay(runTime);
}

void pivot90() {
  // spin in place: both software-forward
  // → M1 turns physical forward, M2 turns physical backward
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, HIGH);
  digitalWrite(motorPin3, HIGH);
  digitalWrite(motorPin4, LOW);

  delay(turnTime);
}

void stopMotors() {
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, LOW);
  digitalWrite(motorPin3, LOW);
  digitalWrite(motorPin4, LOW);
}


