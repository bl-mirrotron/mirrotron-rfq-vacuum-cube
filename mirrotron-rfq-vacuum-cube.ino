#include "Controllino.h" 
#include "BlinkyBus.h"
#define BAUD_RATE  19200
#define commLEDPin    80

#define BLINKYBUSBUFSIZE  15
union BlinkyBusUnion
{
  struct
  {
    int16_t state;
    int16_t gauge1;
    int16_t gauge2;
    int16_t gauge3;
    int16_t gauge4;
    int16_t turboSpeed1;
    int16_t turboError1;
    int16_t turboSpeed2;
    int16_t turboError2;
    int16_t gate1Open;
    int16_t gate1Closed;
    int16_t gate2Open;
    int16_t gate2Closed;
    int16_t openGate;
    int16_t watchdog;
  };
  int16_t buffer[BLINKYBUSBUFSIZE];
} bb;
BlinkyBus blinkyBus(bb.buffer, BLINKYBUSBUFSIZE, Serial, commLEDPin);
int pollType = 0;
void setup() 
{
  pinMode(CONTROLLINO_A1,  INPUT); // gauge1
  pinMode(CONTROLLINO_A2,  INPUT); // gauge2
  pinMode(CONTROLLINO_A3,  INPUT); // gauge3
  pinMode(CONTROLLINO_A4,  INPUT); // gauge4
  pinMode(CONTROLLINO_A5,  INPUT); // turboSpeed1
  pinMode(CONTROLLINO_A6,  INPUT); // turboError1
  pinMode(CONTROLLINO_A7,  INPUT); // turboSpeed2
  pinMode(CONTROLLINO_A8,  INPUT); // turboError2
  pinMode(CONTROLLINO_A9,  INPUT); // gate1Open
  pinMode(CONTROLLINO_A10, INPUT); // gate1Closed
  pinMode(CONTROLLINO_A11, INPUT); // gate2Open
  pinMode(CONTROLLINO_A12, INPUT); // gate2Closed
  pinMode(CONTROLLINO_R4, OUTPUT); // openGate1
  pinMode(CONTROLLINO_R5, OUTPUT); // openGate2

  bb.openGate = 0;
  bb.watchdog = 0;

  digitalWrite(CONTROLLINO_R4, bb.openGate);
  digitalWrite(CONTROLLINO_R5, bb.openGate);
  
  Serial.begin(BAUD_RATE);
  blinkyBus.start();
  delay(1000);
}

void loop() 
{
  
  bb.gauge1      = (int16_t) analogRead(CONTROLLINO_A1);
  bb.gauge2      = (int16_t) analogRead(CONTROLLINO_A2);
  bb.gauge3      = (int16_t) analogRead(CONTROLLINO_A3);
  bb.gauge4      = (int16_t) analogRead(CONTROLLINO_A4);
  bb.turboSpeed1 = (int16_t) digitalRead(CONTROLLINO_A5);
  bb.turboError1 = (int16_t) digitalRead(CONTROLLINO_A6);
  bb.turboSpeed2 = (int16_t) digitalRead(CONTROLLINO_A7);
  bb.turboError2 = (int16_t) digitalRead(CONTROLLINO_A8);
  bb.gate1Open   = (int16_t) digitalRead(CONTROLLINO_A9);
  bb.gate1Closed = (int16_t) digitalRead(CONTROLLINO_A10);
  bb.gate2Open   = (int16_t) digitalRead(CONTROLLINO_A11);
  bb.gate2Closed = (int16_t) digitalRead(CONTROLLINO_A12);

  digitalWrite(CONTROLLINO_R4, bb.openGate);
  digitalWrite(CONTROLLINO_R5, bb.openGate);

  pollType = blinkyBus.poll();
  if (pollType > 0)
  {
    bb.watchdog = bb.watchdog + 1;
    if (bb.watchdog > 32700) bb.watchdog = 0;
  }
  

  delay(10); 
}
