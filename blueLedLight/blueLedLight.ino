
#include "BluetoothSerial.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif


// use first channel of 16 channels (started from zero)
#define LEDC_CHANNEL_0     0

// use 13 bit precission for LEDC timer
#define LEDC_TIMER_13_BIT  13

// use 5000 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ     5000

// fade LED PIN (replace with LED_BUILTIN constant for built-in LED)
#define LED_PIN            26

BluetoothSerial SerialBT;
int received;// received value will be stored in this variable
char receivedChar;// received value will be stored as CHAR in this variable

const char turnON ='a';
const char turnOFF ='b';
const int LEDpin = 26;


// Arduino like analogWrite
// value has to be between 0 and valueMax
void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255) {
  // calculate duty, 8191 from 2 ^ 13 - 1
  Serial.print(value);
  uint32_t duty = (8191 / valueMax) * min(value, valueMax);
Serial.println(duty);
  // write duty to LEDC
  ledcWrite(channel, duty);
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(115200);
  SerialBT.begin("ESP32_Robojax"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
  Serial.println("To turn ON send: a");//print on serial monitor  
  Serial.println("To turn OFF send: b"); //print on serial monitor 
  // Setup timer and attach timer to a led pin
  ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(LED_PIN, LEDC_CHANNEL_0);
  ledcAnalogWrite(LEDC_CHANNEL_0, 200);
}

void loop() {
    //receivedChar =(char)SerialBT.read();
    String x = SerialBT.readString();
    x.trim();
    
  

//    if(x == "a"){
//      digitalWrite(LEDpin, HIGH);
//    }else if(x == "b"){
//      digitalWrite(LEDpin, LOW);
//    }
if(x && x[0]){

 ledcAnalogWrite(LEDC_CHANNEL_0, x.toInt());
  }
 
Serial.println(x);
//  if (Serial.available()) {
//    SerialBT.write(Serial.read());
//  
//  }
//  if (SerialBT.available()) {
//    
//    SerialBT.print("Received:");// write on BT app
//    SerialBT.println(receivedChar);// write on BT app      
//    Serial.print ("Received:");//print on serial monitor
//    Serial.println(receivedChar);//print on serial monitor    
//    //SerialBT.println(receivedChar);//print on the app    
//    //SerialBT.write(receivedChar); //print on serial monitor
//    if(receivedChar == turnON)
//    {
//     SerialBT.println("LED ON:");// write on BT app
//     Serial.println("LED ON:");//write on serial monitor
//     digitalWrite(LEDpin, HIGH);// turn the LED ON
//       
//    }else
//    {
//     SerialBT.println("LED OFF:");// write on BT app
//     Serial.println("LED OFF:");//write on serial monitor
//      digitalWrite(LEDpin, LOW);// turn the LED off 
//    }    
//     
//  
//
//  }
  delay(20);
}
