/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com  
*********/
#include "Arduino.h"
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

// Assign output variables to GPIO pins
const int RED_PIN = 13;
const int GREEN_PIN = 12;
const int BLUE_PIN = 15;

const int RED_CHENNEL = 0;
const int GREEN_CHENNEL = 1;
const int BLUE_CHENNEL = 2;
void setup() {
  Serial.begin(115200);
  // Initialize the output variables as outputs
  ledcAttachPin(RED_PIN, RED_CHENNEL);
  ledcAttachPin(GREEN_PIN, GREEN_CHENNEL);
  ledcAttachPin(BLUE_PIN, BLUE_CHENNEL);

  ledcSetup(RED_CHENNEL, 5000, 8);
  ledcSetup(GREEN_CHENNEL, 5000, 8);
  ledcSetup(BLUE_CHENNEL, 5000, 8);

   Serial.println(ESP.getSdkVersion());
   String deviceName = "light";

    Serial.println(deviceName);
    SerialBT.begin(deviceName);
    SerialBT.setTimeout(100);
}

void RGB_color(int red_light_value, int green_light_value, int blue_light_value)
 {
  ledcWrite(RED_CHENNEL,  255 - red_light_value);
  ledcWrite(GREEN_CHENNEL, 255 - green_light_value);
  ledcWrite(BLUE_CHENNEL,255 -  blue_light_value);
}

void controlRGB(String code){
  String colors  = code.substring(7,code.length());
  Serial.println(colors);
  int posR = colors.indexOf("r");
  int posG = colors.indexOf("g");
  int posB = colors.indexOf("b");
  int posA = colors.indexOf("&");
  Serial.println(colors.substring(posR + 1,posG).toInt());
  Serial.println(colors.substring(posG + 1 ,posB).toInt());
  Serial.println(colors.substring(posB + 1,posA).toInt());
  
  RGB_color(colors.substring(posR + 1,posG).toInt(),colors.substring(posG + 1,posB).toInt(),colors.substring(posB + 1,posA).toInt());
  }

void loop() {


   String code = SerialBT.readString();
   code.trim();
   if(code && code[0]){
   controlRGB(code);
    }
 yield();
}
