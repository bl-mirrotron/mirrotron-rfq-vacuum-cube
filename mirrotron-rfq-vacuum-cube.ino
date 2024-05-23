const boolean CHATTY_CATHY  = false;
const char*   MQTT_SERVER   = "192.168.4.1";
const char*   MQTT_USERNAME = "mirrotron-controls";
const char*   MQTT_PASSWORD = "areallybadpassword";
const char*   BOX           = "mirrotron-controls";
const char*   TRAY_TYPE     = "rfq-vacuum";
const char*   TRAY_NAME     = "01";
const char*   HUB           = "cube";

#include <Controllino.h> 

union CubeData
{
  struct
  {
    int16_t state;
    int16_t watchdog;
    int16_t newData;
    int16_t gauge1;
    int16_t gauge2;
    int16_t gauge3;
    int16_t gauge4;
    int16_t turboSpeed1;
    int16_t turboNoError1;
    int16_t turboSpeed2;
    int16_t turboNoError2;
    int16_t gate1Open;
    int16_t gate1Closed;
    int16_t gate2Open;
    int16_t gate2Closed;
    int16_t openGate;
    int16_t startTurboOK;
    int16_t scrollGaugeLimit;
  };
  byte buffer[36];
};
CubeData cubeData;

struct
{
    int16_t turboSpeed1 = 0;
    int16_t turboSpeed2 = 0;
    int16_t turboNoError1 = 0;
    int16_t turboNoError2 = 0;
} turboState;

byte mac[] = { 0x42, 0x4C, 0x30, 0x30, 0x30, 0x31 };

#include "BlinkyEtherCube.h"

unsigned long lastPublishTime;
unsigned long publishInterval = 2000;


void setup() 
{
  if (CHATTY_CATHY)
  {
    Serial.begin(9600);
    delay(10000);
    Serial.println("Starting communications...");
  }

  // Optional setup to overide defaults
  BlinkyEtherCube.setChattyCathy(CHATTY_CATHY);
  BlinkyEtherCube.setMqttRetryMs(3000);
  BlinkyEtherCube.setBlMqttKeepAlive(8);
  BlinkyEtherCube.setBlMqttSocketTimeout(4);
  
  // Must be included
  BlinkyEtherCube.setMqttServer(mac, MQTT_SERVER, MQTT_USERNAME, MQTT_PASSWORD);
  BlinkyEtherCube.setMqttTray(BOX,TRAY_TYPE,TRAY_NAME, HUB);
  BlinkyEtherCube.init(&cubeData);

  lastPublishTime = millis();
  cubeData.state = 1;

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

  cubeData.openGate = 0;
  cubeData.watchdog = 0;

  digitalWrite(CONTROLLINO_R4, cubeData.openGate);
  digitalWrite(CONTROLLINO_R5, cubeData.openGate);
  
}

void loop() 
{
  unsigned long nowTime = millis();

  float gauge1 = (float) analogRead(CONTROLLINO_A1);
  float gauge2 = (float) analogRead(CONTROLLINO_A2);
  float gauge3 = (float) analogRead(CONTROLLINO_A3);
  float gauge4 = (float) analogRead(CONTROLLINO_A4);
  gauge1 = (gauge1 * 0.02950830 - 3.4286) * 512;
  gauge2 = (gauge2 * 0.02950830 - 3.4286) * 512;
  gauge3 = (gauge3 * 0.04917969 - 9.2415) * 512;
  gauge4 = (gauge4 * 0.04917969 - 9.2415) * 512;
 
  cubeData.gauge1      = gauge1;
  cubeData.gauge2      = gauge2;
  cubeData.gauge3      = gauge3;
  cubeData.gauge4      = gauge4;
  cubeData.startTurboOK = 0;
  if ((cubeData.gauge1 < 1024) && (cubeData.gauge2 < 1024) && (cubeData.gauge3 < 1024) && (cubeData.gauge4 < 1024) )
  {
      cubeData.startTurboOK = 1;
  }
  if ( (cubeData.scrollGaugeLimit < cubeData.gauge1) || (cubeData.scrollGaugeLimit < cubeData.gauge2) )
  {
    if (cubeData.openGate == 1) cubeData.newData = 1;
    cubeData.openGate = 0;
  }

  cubeData.turboSpeed1   = (int16_t) digitalRead(CONTROLLINO_A5);
  cubeData.turboNoError1 = (int16_t) digitalRead(CONTROLLINO_A6);
  cubeData.turboSpeed2   = (int16_t) digitalRead(CONTROLLINO_A7);
  cubeData.turboNoError2 = (int16_t) digitalRead(CONTROLLINO_A8);
  cubeData.gate1Open     = (int16_t) digitalRead(CONTROLLINO_A9);
  cubeData.gate1Closed   = (int16_t) digitalRead(CONTROLLINO_A10);
  cubeData.gate2Open     = (int16_t) digitalRead(CONTROLLINO_A11);
  cubeData.gate2Closed   = (int16_t) digitalRead(CONTROLLINO_A12);

  boolean downtransition = false;
  if ((cubeData.turboSpeed1 == 0) && (turboState.turboSpeed1 == 1)) downtransition = true;
  if ((cubeData.turboSpeed2 == 0) && (turboState.turboSpeed2 == 1)) downtransition = true;
  if ((cubeData.turboNoError1 == 0) && (turboState.turboNoError1 == 1)) downtransition = true;
  if ((cubeData.turboNoError2 == 0) && (turboState.turboNoError2 == 1)) downtransition = true;
  turboState.turboSpeed1 = cubeData.turboSpeed1;
  turboState.turboSpeed2 = cubeData.turboSpeed2;
  turboState.turboNoError1 = cubeData.turboNoError1;
  turboState.turboNoError2 = cubeData.turboNoError2;
  if (downtransition)
  {
    if (cubeData.openGate == 1) cubeData.newData = 1;
    cubeData.openGate = 0;
  }

  digitalWrite(CONTROLLINO_R4, cubeData.openGate);
  digitalWrite(CONTROLLINO_R5, cubeData.openGate);

  if (cubeData.newData > 0)
  {
    lastPublishTime = nowTime;
    cubeData.watchdog = cubeData.watchdog + 1;
    if (cubeData.watchdog > 32760) cubeData.watchdog= 0 ;
    BlinkyEtherCube.publishToServer();
    BlinkyEtherCube.loop();
    cubeData.newData = 0;
  }

  if ((nowTime - lastPublishTime) > publishInterval)
  {
    lastPublishTime = nowTime;
    cubeData.watchdog = cubeData.watchdog + 1;
    if (cubeData.watchdog > 32760) cubeData.watchdog= 0 ;
    BlinkyEtherCube.publishToServer();
  }  
  BlinkyEtherCube.loop();
}

void handleNewSettingFromServer(uint8_t address)
{
  switch(address)
  {
    case 0:
      break;
    case 1:
      break;
    case 2:
      break;
    case 3:
      break;
    case 4:
      break;
    case 15:
      if (cubeData.state > 0) cubeData.openGate = 0;
      break;
    default:
      break;
  }
}
