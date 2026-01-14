#include <Servo.h>
#include <WiFiS3.h>  // #include <WiFiNINA.h>  για το REV2  &     για το REV4
#include "arduino_secrets.h"

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

WiFiUDP Udp;
unsigned int localPort = 2394;

IPAddress serverIp(192,168,191,102); // Server
unsigned int serverPort = 2390;

char incomingPacket[256];

int sensorId = 110;

// ——— PIN CONFIG ———
const int motorPin1      = 2;   // M1 IN1 → Arduino 2
const int motorPin2      = 3;   // M1 IN2 → Arduino 3
const int motorPin3      = 4;   // M2 IN3 → Arduino 4
const int motorPin4      = 5;   // M2 IN4 → Arduino 5
const int motorSpeedPin1 = 9;   // M1 PWM → Arduino 9
const int motorSpeedPin2 = 10;  // M2 PWM → Arduino 10
Servo myservo;  // create servo object to control a servo
int pos = 0;    // variable to store the servo position
int lastHandled = 100;   // so we don’t re‐run the same route

// ——— PHYSICAL/TUNING ———
const float wheelDiameter      = 6.5;                  // cm
const float wheelCircumference = wheelDiameter * 3.1416; // ≈20.4 cm
const unsigned long turnTime   = 500;                   // ms for ~90° spin (was 150)
const int   motorSpeed1 = 145;  
const int   motorSpeed2 = 145;  
//
const int trigPin = 13;
const int echoPin = 12;

float duration, distance;

// ——— DECLARATIONS ———
void moveForward(int distanceCm);
void pivot90();
void stopMotors();

void setup() {
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
  Serial.begin(9600);
  myservo.attach(13);  // attaches the servo on pin 11 to the servo object

  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    delay(1000);
  }
  Serial.print("Client R4: ");
  Serial.println(WiFi.localIP());

  Udp.begin(localPort);

}

void loop() {  
  Serial.println(WiFi.localIP());
   // Στείλε μήνυμα στον Server
  String msg = "R4;0;0;0;0;0";
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

  delay(5000);

char* robot;
char* local;
char* statusR2;
char* statusR3;
char* statusR4;
char* statusR5;

    robot = strtok (incomingPacket , ";");
    local = strtok (NULL , ";");
    statusR2 = strtok (NULL, ";");
    statusR3 = strtok (NULL, ";");
    statusR4 = strtok (NULL, ";");
    statusR5 = strtok (NULL, ";");


  
  sensorId =  atoi(statusR4);



  Serial.println(sensorId);
  // Sensor 1 pin3
  if (sensorId == 3 && sensorId != lastHandled) {
    const int   travelDistance     = 20 ;                    // cm straight
    lastHandled = 3;
    delay(5000);
    moveForward(travelDistance);
    stopMotors();
    pivot90();
    stopMotors();
    moveForward(travelDistance);
    stopMotors();
    servosweep();
   // while (true);  // done
   Serial.println(sensorId);
  }

  // Sensor 2 pin5
  if (sensorId == 5 && sensorId != lastHandled) {
    const int   travelDistance     = 44 ;                    // cm straight
    lastHandled = 5;
    moveForward(travelDistance);
    stopMotors();
    servosweep();
  }

  // Sensor 3 pin 4
  if (sensorId == 4 && sensorId != lastHandled) {
    lastHandled = 4;
    const int   travelDistance     = 88 ;                    // cm straight
    lastHandled = 2;
    moveForward(travelDistance);
    stopMotors();
    servosweep();
  }
}

void moveForward(int distanceCm) {
  // FIXED MOTOR DIRECTIONS FOR FORWARD
  digitalWrite(motorPin1, HIGH);
  digitalWrite(motorPin2, LOW);
  digitalWrite(motorPin3, LOW);
  digitalWrite(motorPin4, HIGH);

  float rotations = distanceCm / wheelCircumference;
  unsigned long runTime = (unsigned long)(rotations * 1000); 
  delay(runTime);
}

void pivot90() {
  // FIXED: One motor forward, other backward to pivot
  digitalWrite(motorPin1, HIGH);   // M1 forward
  digitalWrite(motorPin2, LOW);
  digitalWrite(motorPin3, HIGH);    // M2 backward
  digitalWrite(motorPin4, LOW);

  delay(turnTime);
}

void stopMotors() {
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, LOW);
  digitalWrite(motorPin3, LOW);
  digitalWrite(motorPin4, LOW);
}

void servosweep() {
  for (pos = 0; pos <= 180; pos += 1) {
    myservo.write(pos);
    delay(5);
  }
  for (pos = 180; pos >= 0; pos -= 1) {
    myservo.write(pos);
    delay(5);
  }
}


