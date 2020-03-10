#include "BluetoothSerial.h"
#include <Arduino.h>
#include <string.h>
#include <EEPROM.h>



#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif


BluetoothSerial SerialBT;

volatile int i=0;               // Variable to use as a counter of dimming steps. It is volatile since it is passed between interrupts
volatile boolean zero_cross=0;  // Flag to indicate we have crossed zero
int AC_pin = 26;                 // Output to Opto Triac                // second button at pin 5
int dim = 100;                  // Dimming level (0-10)  0 = on, 10 = 0ff
const uint8_t PIN_IR_TX = 32;
const String ON = "on";
const String OFF = "off";
const String IR_STRING = "ir";
const String RENAME_COMMAND = "rename";
const int memoryAddress = 0;
const String DEFAULT_NAME = "LITTRA SMART";
const int EEPROM_SIZE = 128;


hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void dim_check() {  
  
  if(zero_cross == true ) {  
              
    if(i>=dim) {                     
      digitalWrite(AC_pin, HIGH);  // turn on light       
      i=0;  // reset time step counter                         
      zero_cross=false;    // reset zero cross detection flag
    } 
    else {
      i++;  // increment time step counter                     
    }                                
  }    
}  

void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  dim_check();
  portEXIT_CRITICAL_ISR(&timerMux);
}
void startTimer() {
  timer = timerBegin(0, 80, true); // timer_id = 0; divider=80; countUp = true;
  timerAttachInterrupt(timer, &onTimer, true); // edge = true
  timerAlarmWrite(timer, 75, true);  //1000 ms
  timerAlarmEnable(timer);
}

void endTimer() {
  timerEnd(timer);
  timer = NULL; 
  startTimer();
}

 
void setup() {  // Begin setup
  Serial.begin(115200);   
 Serial.setDebugOutput(true);
  pinMode(AC_pin, OUTPUT);                          // Set the Triac pin as output

      // initialize EEPROM with predefined size

//    EEPROM.begin(EEPROM_SIZE);

    
    Serial.println(ESP.getSdkVersion());
//    String deviceName = read_String(memoryAddress);
        String deviceName = "fan";
    if(!deviceName || !deviceName[0] ){
      deviceName = DEFAULT_NAME;
      }
    Serial.println(deviceName);
    SerialBT.begin(deviceName);
    SerialBT.setTimeout(100);
    
    Serial.println("Press the button to change the device's name");
     
   
  attachInterrupt(21, zero_cross_detect, RISING);    // Attach an Interupt to Pin 2 (interupt 0) for Zero Cross Detection
  startTimer();

}

void zero_cross_detect() {  
Serial.println("Zero");
  zero_cross = true;               // set flag for dim_check function that a zero cross has occured
  i=0;                             // stepcounter to 0.... as we start a new cycle
  digitalWrite(AC_pin, LOW);
}                                 


void loop() {  
 
   String code = SerialBT.readString();
    code.trim();
    if(code && code[0])
    {
      SerialBT.println(dim);
      dim = code.toInt();

    }
   
     
      
  delay (100);
}
