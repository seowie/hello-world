/*
* Car driving code created by Stephen Baker
* Version_test
* 
* Functions:
*   IR
*   Basic movements: Forward (2), Backward (8), Stop (5), Left (4), Right (6).
*   Ultrasonic: Used in 'self-stopping'(CH) & 'follow me' (CH+). Press (CH-) to turn off.
*   Speed counter (+) & (-): Changes speed for both Forward & Reverse; increments of 10.
*       ** Include default speed = 100 (EQ)???
*   Self-stopping (CH ): If < 10cm, car will stop, reverse and turn right before continuing forward.
*   Follow me (CH+): Following range: 10-40cm. Stop if > 40cm; Reverse if < 10cm.
*/

//code added to decode buttons from Duinotech remote
// only slightly modified from Library example

#include <IRremote.h>
int RECV_PIN = 11;
IRrecv irrecv(RECV_PIN);
decode_results results;
// Setup motor controller
// motor one
int enA = 10;
int in1 = 9;
int in2 = 8;
// motor two
int enB = 5;
int in3 = 7;
int in4 = 6;
// Ultrasonic
#define trigPin 12
#define echoPin 13
// Display
#define MAX7219_DIN 4
#define MAX7219_CS  3
#define MAX7219_CLK 2
long duration, distance;
int ultra = 0;      // ultrasonic is not on initially
int counter = 100;  // inital speed = 100
int remainder, quotient;

enum {  MAX7219_REG_DECODE    = 0x09,
        MAX7219_REG_INTENSITY = 0x0A,
        MAX7219_REG_SCANLIMIT = 0x0B,
        MAX7219_REG_SHUTDOWN  = 0x0C,
        MAX7219_REG_DISPTEST  = 0x0F
     };
     

void setup()
{
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
  // set all the motor control pins to outputs
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  // Ultrasonic Pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  // Display
  digitalWrite(MAX7219_CS, HIGH);
  pinMode(MAX7219_DIN, OUTPUT);
  pinMode(MAX7219_CS, OUTPUT);
  pinMode(MAX7219_CLK, OUTPUT);
}


/* ---------------------------- // ---------------------------- */

// -----------------------------------------------
// Motor Movements: Forward, Reverse, Halt
// -----------------------------------------------
void Forward()
{
  // this function will run the motors forward
  // turn on motor A
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  // set speed to 100 out of possible range 0~255
  analogWrite(enA, counter);
  // turn on motor B
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  // set speed to 100 out of possible range 0~255
  analogWrite(enB, counter);
  //Serial.println("Forward");
}
void Reverse()
{
  // this function will run the motors Reverse
  // turn on motor A
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  // set speed to 100 out of possible range 0~255
  analogWrite(enA, counter);
  // turn on motor B
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  // set speed to 100 out of possible range 0~255
  analogWrite(enB, counter);
  Serial.println("Reverse");
}
void Halt()
{
  // this function will stop the motors
  // stop motor A
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  // stop motor B
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  Serial.println("Halt");
}


// -----------------------------------------------
// Motor Movements: Right, Left
// -----------------------------------------------
void Right()
{
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  analogWrite(enA, 100);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  analogWrite(enB, 125);
  // For 90 degree turn, uncomment the following lines.
  // Adjust delay time if speed is changed.
  delay(850);
  Halt();
  delayMicroseconds(100);
  Forward();
  //Serial.println("Right");
}
void Left()
{
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  analogWrite(enA, 125);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  analogWrite(enB, 100);
  // For 90 degree turn, uncomment the following lines.
  // Adjust delay time if speed is changed.
  delay(850);
  Halt();
  delayMicroseconds(100);
  Forward();
  //Serial.println("Left");
}


// -----------------------------------------------
// Speed counter
// -----------------------------------------------
void counterplus() {
  if (counter < 220)
  {
    counter = counter + 10;
  }
  Serial.print(counter);
}
void counterminus() {
  if (counter > 50) 
  {
    counter = counter - 10;
  }
  Serial.print(counter);
}


// -----------------------------------------------
// Ultrasonic
// -----------------------------------------------
void Ultrasonic()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration / 2) / 29.1;
  //Serial.println("Ultrasonic");
}

// -----------------------------------------------
// Self Stopping
// -----------------------------------------------
void Selfstop()
{
  Ultrasonic();
  if (distance <= 0 || distance > 150) {
    Serial.println("Far Far Away");
  }
  else if (distance >= 10) {
    Serial.print(distance);
    Serial.println("cm");
    //Forward();
  }
  else {
    Halt();
    delay(1000);
    Reverse();
    delay(500);
    Right();
  }
}

// -----------------------------------------------
// Follow Me
// -----------------------------------------------
void FollowMe() {
  Ultrasonic();
  //Serial.print(distance);
  //Serial.println("cm");
  if (distance > 40) {
    Halt();
  }
  else if (distance > 10) {
    Serial.println("Fooollooow~~~~");
    counter = 100;
    Forward();
  }
  else {
    Serial.println("Tooooo clooosssee!!!!");
    counter = 100;
    Reverse();
  }
}


// -----------------------------------------------
// 7 Segment Display
// -----------------------------------------------
void output(byte address, byte data)
{
  digitalWrite(MAX7219_CS, LOW);
  shiftOut(MAX7219_DIN, MAX7219_CLK, MSBFIRST, address);
  shiftOut(MAX7219_DIN, MAX7219_CLK, MSBFIRST, data);
  digitalWrite(MAX7219_CS, HIGH);
}
void DisplayDistance()
{
  Ultrasonic();
  remainder = distance % 10;
  quotient = distance / 10;
  output(MAX7219_REG_DISPTEST, 0x00); //display test register - test mode off
  output(MAX7219_REG_SHUTDOWN, 0x01); //shutdown register - normal operation
  output(MAX7219_REG_INTENSITY, 0x06); //intensity register
  output(MAX7219_REG_DECODE, 0xff); //decode mode register - CodeB decode all digits
  output(MAX7219_REG_SCANLIMIT, 0x01); //scan limit register - display digits 0 & 1 only
  output(0x02, quotient);
  output(0x01, remainder); //digit 0 (rightmost digit) data
  Serial.print(quotient);
  Serial.print(remainder);
  Serial.println("\n");
  delay(100);
  output(MAX7219_REG_SHUTDOWN, 0x00);   //shutdown
}


/* ---------------------------- // ---------------------------- */

// -----------------------------------------------
// Main Loop
// -----------------------------------------------
void loop() {
  if (irrecv.decode(&results)) {
    //Serial.println(results.value, HEX);
    switch (results.value) {
      // Car mp3 remote
      case 0xFFA25D: ultra = 0; Serial.println("CH-: Normal Driving"); break; // CH-
      case 0xFF629D: ultra = 1; Serial.println("CH : Self-stopping mode"); break; // CH
      case 0xFFE21D: ultra = 2; Serial.println("CH+: Follow me mode"); break; // CH+
      case 0xFF22DD: Serial.println("Prev"); break; // |<<
      case 0xFF02FD: Serial.println("Next"); break; // >>|
      case 0xFFC23D: Serial.println("Play / Pause"); break; // |>||
      case 0xFFE01F: counterminus(); Forward(); Serial.println("\t-: Decrease Speed"); break; // -
      case 0xFFA857: counterplus(); Forward(); Serial.println("\t+: Increase Speed"); break; // +
      case 0xFF906F: Serial.println("EQ"); break; // EQ
      case 0xFF9867: Serial.println("100+"); break; // 100+
      case 0xFFB04F: Serial.println("200+"); break; // 200+

      case 0xFF6897: Serial.println("0"); break; // 0
      case 0xFF30CF: Serial.println("1"); break; // 1
      case 0xFF18E7: Forward(); Serial.print(counter); Serial.println("\t2: Forward"); break; // 2
      case 0xFF7A85: Serial.println("3"); break; // 3
      case 0xFF10EF: Left(); Serial.println("4: Left"); break; // 4
      case 0xFF38C7: Halt(); Serial.println("5: Stop"); break; // 5
      case 0xFF5AA5: Right(); Serial.println("6: Right"); break; // 6
      case 0xFF42BD: Serial.println("7"); break; // 7
      case 0xFF4AB5: Reverse(); Serial.print(counter); Serial.println("\t8: Reverse"); break; // 8
      case 0xFF52AD: Serial.println("9"); break; // 5
        //case 0xFFFFFFFF: Serial.println("Repeat"); break;
    }
    irrecv.resume(); // Receive the next value
  }
  DisplayDistance(); 
  if (ultra == 1) {
    Selfstop();
  }
  if (ultra == 2) {
    FollowMe();
  }
}




