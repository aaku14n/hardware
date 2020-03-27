
#include "BluetoothSerial.h"
#include <Arduino.h>
#include <string.h>
//#include <WiFi.h>
//#include <fauxmoESP.h>
#include <string.h>
#include <EEPROM.h>

//fauxmoESP fauxmo;
byte EEPROM_SIZE = 128;
byte memoryAddress = 4;
byte stateMemoryAddress = 0;
String DEFAULT_NAME = "LITTRA SMART";
const String wifi = "wifi ";
String ssid = "";
String password = "";
byte memoryAddressForWifi = 30;
byte memoryAddressForSpeed = 90;
String SEPERATE_STRING = "--";
const String RENAME_COMMAND = "rename";
const String RESET_STRING = "reset";
String SPEED_STRING = "speed ";
int isSpeedLoopIsRunning = false;
volatile byte i = 0;             // Variable to use as a counter of dimming steps. It is volatile since it is passed between interrupts
volatile boolean zero_cross = 0; // Flag to indicate we have crossed zero
byte speedLimit = 1;
String code = "";
byte currentState = 0;
hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;
const byte LED_BUILTIN = 26;
const byte INTERRUPT_PIN = 19;

void IRAM_ATTR onTimer()
{
  portENTER_CRITICAL_ISR(&timerMux);
   if (zero_cross == true && currentState == 1)
  {
    if (i >= speedLimit)
    {
      digitalWrite(LED_BUILTIN, HIGH); // turn on light
      i = 0;                           // reset time step counter
      zero_cross = false;              // reset zero cross detection flag
    }
    else
    {
      i++; // increment time step counter
    }
  }
  portEXIT_CRITICAL_ISR(&timerMux);
}
void startTimer()
{
 
 
    timer = timerBegin(0, 80, true);             // timer_id = 0; divider=80; countUp = true;
    timerAttachInterrupt(timer, &onTimer, true); // edge = true
    timerAlarmWrite(timer, 75, true);            //1000 ms
    timerAlarmEnable(timer);
    attachInterrupt(INTERRUPT_PIN, zero_cross_detect, RISING);
}

void endTimer()
{
  if (isSpeedLoopIsRunning)
  {
     Serial.println("came in end timer");
    isSpeedLoopIsRunning = false;
    timerEnd(timer);
    timer = NULL;
    detachInterrupt(INTERRUPT_PIN);
  }
}


void setup()
{

  Serial.begin(115200);
  // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);

  Serial.println(ESP.getSdkVersion());
  pinMode(LED_BUILTIN, OUTPUT);

  // Starting BLE
  
  setupBle();

  startTimer();
  String lastState = read_String(stateMemoryAddress);
 
  if (lastState && lastState[0])
  {
   
    currentState = lastState.toInt();
  }

  if (currentState == 1)
  {
     String previousSpeed = read_String(memoryAddressForSpeed);
    
  if (previousSpeed && previousSpeed[0] && previousSpeed.toInt() != 1)
  {
    speedLimit = previousSpeed.toInt();
   
  }else{
    speedLimit = 1;
    }
 
  }
  
}


void setupBle()
{
  String deviceName = read_String(memoryAddress);
  if (!deviceName || !deviceName[0])
  {
    deviceName = DEFAULT_NAME;
  }
  Serial.println(deviceName);
  SerialBT.begin(deviceName);
  SerialBT.setTimeout(100);
}


void handleLight(String code)
{
  if (code == "1" || code == "on")
  {
    currentState = 1;
//    writeString(stateMemoryAddress, "1");
  } else if (code == "0" || code == "off")
  {
    currentState = 0;
//    writeString(stateMemoryAddress, "0");
  }
  else if (code == "toggle")
  {
    if (currentState == 1)
    {
      handleLight("off");
    }else
    {
      handleLight("on");
    }
  }
      writeString(stateMemoryAddress, String(currentState));
}

void writeString(int add, String data)
{
  byte _size = data.length();
  byte dataCount;
  for (dataCount = 0; dataCount < _size;dataCount++)
  {
    EEPROM.write(add + dataCount, data[dataCount]);
  }
  EEPROM.write(add + _size, '\0'); //Add termination null character for String Data
  EEPROM.commit();
}

String read_String(int add)
{

  char data[128]; //Max 100 Bytes
  int len = 0;
  unsigned char k;
  k = EEPROM.read(add);
  while (k != '\0' && len < 128) //Read until null character
  {
    k = EEPROM.read(add + len);
    if (k != 255)
    {
      data[len] = k;
    }
    else
    {
      break;
    }
    len++;
  }
  data[len] = '\0';

  return String(data);
}
void handleRename(String code)
{
  String newName = code.substring(7, code.length());
  writeString(memoryAddress, newName);
  ESP.restart();
}
void sendDetails()
{
  String previousState = read_String(stateMemoryAddress);
  String wifiCred = read_String(memoryAddressForWifi);
  String previousSpeed = read_String(memoryAddressForSpeed);
  if (!previousState || !previousState[0])
  {
    previousState = "1";
  }
  SerialBT.println("details " + previousState + "--" + wifiCred + "--" + previousSpeed);
}



void zero_cross_detect()
{
  
  zero_cross = true; // set flag for dim_check function that a zero cross has occured
  i = 0;             // stepcounter to 0.... as we start a new cycle
  digitalWrite(LED_BUILTIN, LOW);
  
}

void handleSpeed(String code)
{
  String newSpeed = code.substring(6, code.length());
    speedLimit = newSpeed.toInt();
   
    handleLight("on");
    writeString(memoryAddressForSpeed, String(speedLimit));
}

void loop()
{

  code = SerialBT.readString();
  code.trim();
  if (code && code[0])
  {
    Serial.println(code);
     SerialBT.println(code);
  }
//  if (code == "retry wifi")
//  {
//    connectWifi();
//  }
//  else if (code.indexOf(wifi) >= 0)
//  {
//    setWifiCred(code);
//  }
//  else
  if (code.indexOf(RENAME_COMMAND) >= 0)
  {
    handleRename(code);
  }
  else if (code.indexOf(SPEED_STRING) >= 0)
  {
    handleSpeed(code);
  }
//  else if (code == RESET_STRING)
//  {
//    handleRename(code);
//  }
  else if (code == "details")
  {
    sendDetails();
  }
  else if(code == "on" || code == "off" || code == "toggle")
  {
    handleLight(code);
  }


  delay(100);
}
