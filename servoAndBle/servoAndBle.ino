
#include <Servo.h>
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif


BluetoothSerial SerialBT;
Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

int pos = 0;    // variable to store the servo position
String flag = "";
void setup() {
   Serial.begin(115200);
  myservo.attach(25);  // attaches the servo on pin 9 to the servo object
  
  
  Serial.println(ESP.getSdkVersion());
     String deviceName = "light";

    Serial.println(deviceName);
    SerialBT.begin(deviceName);
    SerialBT.setTimeout(100);
}
void backwardRotation(){
//    for (pos = 100; pos <= 140; pos += 1) { // goes from 0 degrees to 180 degrees
//    // in steps of 1 degree
//    Serial.println(pos);
//   Serial.println("InTop");
//    myservo.write(pos);              // tell servo to go to position in variable 'pos'
//    delay(50);                       // waits 15ms for the servo to reach the position
//  }
 myservo.write(175);  
  }
void forwardRotation(){
//     for (pos = 0; pos <= 90; pos += 1) { // goes from 0 degrees to 180 degrees
//    // in steps of 1 degree
//    Serial.println(pos);
//   Serial.println("InTop");
//    myservo.write(pos);              // tell servo to go to position in variable 'pos'
//    delay(20);                       // waits 15ms for the servo to reach the position
//  }
 myservo.write(0);  
  }
void loop() {

    String code = SerialBT.readString();
    code.trim();
    if(code && code[0]){
    Serial.println(code);
      flag = code;
      }
   if(flag == "f"){
    forwardRotation();
    }else if(flag == "b"){
      backwardRotation();
    }else{
        myservo.write(90);
    }
 
  yield();
}
