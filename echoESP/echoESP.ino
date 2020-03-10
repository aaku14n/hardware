#include <WiFi.h>
#include <fauxmoESP.h>

fauxmoESP fauxmo;
const char *ssid = "akku";
const char *password = "nosecurity";
void setup() { 
  Serial.begin(115200);
  
  if (connectWifi()) {
    // Setup fauxmo
     Serial.println("Adding LED device");
     fauxmo.setPort(80);  
     fauxmo.enable(true);
     fauxmo.addDevice("Led"); 
     fauxmo.onSetState([](unsigned char device_id, const char * device_name, 
                    bool state, unsigned char value) {
    Serial.print("Device name:");
    Serial.println(device_name);
    // Here we handle the command received
});
  
}
  
}

void loop() {
  fauxmo.handle();
}

boolean connectWifi() {
  // Let us connect to WiFi
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println(".......");
  Serial.println("WiFi Connected....IP Address:");
  Serial.println(WiFi.localIP());

  return true;
}
