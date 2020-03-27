
#include "BluetoothSerial.h"
#include <Arduino.h>
#include <string.h>
#include <WiFi.h>
#include <fauxmoESP.h>
#include <string.h>
#include <EEPROM.h>

fauxmoESP fauxmo;
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
hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;
const byte LED_BUILTIN = 26;
const byte INTERRUPT_PIN = 19;

void setup()
{

  Serial.begin(115200);
  // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);

  Serial.println(ESP.getSdkVersion());
  pinMode(LED_BUILTIN, OUTPUT);

  // Starting BLE
  
      setupBle();

  String state = "1";
  String previousState = read_String(stateMemoryAddress);
  if (previousState && previousState[0])
  {
    state = previousState;
  }

  if (state.toInt() == 1)
  {
    handleOnLight();
  }

  // Checking for wifi cred
  String wifiCred = read_String(memoryAddressForWifi);
  if (wifiCred && wifiCred[0])
  {
    getWifiCred(wifiCred);
  }
  // setting up Fauxmo server
  setupFauxmo();
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
void setupFauxmo()
{
  if (ssid && ssid[0] && connectWifi())
  {
    // Setup fauxmo
    fauxmo.setPort(80);
    fauxmo.enable(true);
    fauxmo.addDevice(DEFAULT_NAME.c_str());
    fauxmo.onSetState([](unsigned char device_id, const char *device_name,
                         bool state, unsigned char value) {
      if (state)
      {
        handleLight("1");
      }
      else
      {
        handleLight("0");
      }
      // Here we handle the command received
    });
    Serial.println("ADDING");
  }
}
void getWifiCred(String mainString)
{
  int firstSeperate = mainString.indexOf(SEPERATE_STRING);
  DEFAULT_NAME = mainString.substring(0, firstSeperate);

  String credString = mainString.substring(firstSeperate + SEPERATE_STRING.length(), mainString.length());
  int seperate = credString.indexOf(SEPERATE_STRING);

  ssid = credString.substring(0, seperate);
  password = credString.substring(seperate + SEPERATE_STRING.length(), credString.length());
}
boolean connectWifi()
{
  // Let us connect to WiFi
  int i = 0;

  WiFi.begin(ssid.c_str(), password.c_str());
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    i++;
    if (i > 15)
    {
      Serial.println("Erro");
      SerialBT.println("ERROR: WIFI");
      return false;
    }
  }
  Serial.println("WiFi Connected....IP Address:");
  return true;
}

void handleOnLight()
{
  String previousSpeed = read_String(memoryAddressForSpeed);
  Serial.println(previousSpeed);
  if (previousSpeed && previousSpeed[0] && previousSpeed.toInt() != 1)
  {
    speedLimit = previousSpeed.toInt();
    startTimer();
  }
  else
  {
    digitalWrite(LED_BUILTIN, HIGH);
  }
}
void handleLight(String code)
{

  if (code == "1" || code == "on")
  {
    Serial.println("Came in on");
    handleOnLight();
    writeString(stateMemoryAddress, "1");
  }
  else if (code == "0" || code == "off")
  {
    Serial.println("Came in off");
    digitalWrite(LED_BUILTIN, LOW);
    writeString(stateMemoryAddress, "0");
    writeString(memoryAddressForSpeed, String(speedLimit));
    // endTimer();
  }
  else if (code == "toggle")
  {
    String previousState = read_String(stateMemoryAddress);
    if (previousState == "1")
    {
      handleLight("0");
    }
    else
    {
      handleLight("1");
    }
  }
}

void writeString(int add, String data)
{
  int _size = data.length();
  int i;
  for (i = 0; i < _size; i++)
  {
    EEPROM.write(add + i, data[i]);
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

void setWifiCred(String code)
{

  String newName = code.substring(5, code.length());

  writeString(memoryAddressForWifi, newName);
  if (connectWifi())
  {
    delay(20);
    ESP.restart();
  }
}

void handleReset(String code)
{
  writeString(memoryAddressForWifi, "\0");
  ESP.restart();
}

void sendDetails()
{
  String previousState = read_String(stateMemoryAddress);
  String wifiCred = read_String(memoryAddressForWifi);
  if (!previousState || !previousState[0])
  {
    previousState = "1";
  }
  SerialBT.println("details " + previousState + "--" + wifiCred);
}

void IRAM_ATTR dim_check()
{
  if (zero_cross == true)
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
}

void IRAM_ATTR onTimer()
{
  portENTER_CRITICAL_ISR(&timerMux);
  dim_check();
  portEXIT_CRITICAL_ISR(&timerMux);
}
void startTimer()
{
  if (!isSpeedLoopIsRunning)
  {
    isSpeedLoopIsRunning = true;
    attachInterrupt(INTERRUPT_PIN, zero_cross_detect, RISING);
    timer = timerBegin(0, 80, true);             // timer_id = 0; divider=80; countUp = true;
    timerAttachInterrupt(timer, &onTimer, true); // edge = true
    timerAlarmWrite(timer, 75, true);            //1000 ms
    timerAlarmEnable(timer);
  }
}

void endTimer()
{
  if (isSpeedLoopIsRunning)
  {
    isSpeedLoopIsRunning = false;
    timerEnd(timer);
    timer = NULL;
    detachInterrupt(INTERRUPT_PIN);
  }
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

  if (newSpeed.toInt() == 1)
  {
    endTimer();
    speedLimit = 1;
    handleLight("1");
  }
  else if (newSpeed.toInt() == 128)
  {
    endTimer();
    speedLimit = 128;
    handleLight("0");
  }
  else
  {
    writeString(memoryAddressForSpeed, newSpeed);
    speedLimit = newSpeed.toInt();
    startTimer();
  }
}

void loop()
{

  String code = SerialBT.readString();
  code.trim();
  if (code && code[0])
  {
    Serial.println(code);
  }
  if (code == "retry wifi")
  {
    connectWifi();
  }
  else if (code.indexOf(wifi) >= 0)
  {
    setWifiCred(code);
  }
  else if (code.indexOf(RENAME_COMMAND) >= 0)
  {
    handleRename(code);
  }
  else if (code.indexOf(SPEED_STRING) >= 0)
  {
    handleSpeed(code);
  }
  else if (code == RESET_STRING)
  {
    handleRename(code);
  }
  else if (code == "details")
  {
    sendDetails();
  }
  else
  {
    handleLight(code);
  }

  if (ssid && ssid[0])
  {
    fauxmo.handle();
  }
  yield();
}