#include <Servo.h>
#include <WiFiS3.h>  // #include <WiFiNINA.h>  για το REV2  &     για το REV4
#include "arduino_secrets.h"


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
Servo myservo;  // create servo object to control a servo
int pos = 0;    // variable to store the servo position




// ——— PHYSICAL/TUNING ———
const float wheelDiameter      = 6.5;                  // cm
const float wheelCircumference = wheelDiameter * 3.1416; // ≈20.4 cm
const int   travelDistance     = 5;                    // cm straight
const unsigned long turnTime   = 1100;                   // ms for ~90° spin (was 150)
const int   motorSpeed1 = 130;  
const int   motorSpeed2 = 120;  
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
  myservo.attach(11);  // attaches the servo on pin 11 to the servo object

Serial.begin(9600);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    delay(1000);
  }
  Serial.print("Client 1 IP: ");
  Serial.println(WiFi.localIP());

  Udp.begin(localPort);


}

void loop() {
  String statusR5 = "0";
  Serial.println(dist());
  if ((dist() < 5) && (dist() > 0) && ("statusR4" != "1" )) {
  for (int i = 0; i <= 3; i++) {
  statusR5 = "1" ; 
    moveForward(travelDistance);
    stopMotors();
    servosweep();
  }
   pivot90();
   stopMotors();
   
  //while (true);  // done
  }
 
  // Στείλε μήνυμα στον Server
  String msg = "R5;" + statusR5 + ";0;0;0;" + statusR5 ;
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
  }

  delay(5000);


}

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
  digitalWrite(motorPin3, LOW);
  digitalWrite(motorPin4, HIGH);

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

void servosweep() {
  for (pos = 0; pos <= 90; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(5);                       // waits 15ms for the servo to reach the position
  }
  for (pos = 90; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(5);                       // waits 15ms for the servo to reach the position
  }
}



