#include <EEPROM.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>


const char *ssid = "akku";
const char *password = "nosecurity";
const char *mqtt_server = "192.168.1.4";
const char *device_id = "connect";

WiFiClient espClient;
PubSubClient client(espClient);

const byte ledPin5 = 16;
char message_buff[100];

void callback(char *led_control, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(led_control);
    Serial.println("] ");
    Serial.println(length);
    
   
    int i;
    for (i = 0; i < length; i++)
    {
      
        message_buff[i] = payload[i];
        Serial.println(payload[i]);
        Serial.println(i);
        Serial.println(message_buff[i]);
        
    }
    message_buff[i] = '\0';
Serial.println(message_buff);
    String msgString = String(message_buff);
    Serial.println(msgString);
    if (strcmp(led_control, "message") == 0)
    {
        if (msgString == "1")
        {
            digitalWrite(ledPin5, LOW); // PIN HIGH will switch OFF the relay
        }
        if (msgString == "0")
        {
            digitalWrite(ledPin5, HIGH); // PIN LOW will switch ON the relay
        }
    }
}

void reconnect()
{
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        if (client.connect(device_id))
        {
            Serial.println("connected");
            client.subscribe("myTopic"); // write your unique ID here
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void setup()
{
    Serial.begin(115200);

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    client.setServer(mqtt_server, 3001); // change port number as mentioned in your cloudmqtt console
    client.setCallback(callback);

    pinMode(ledPin5, OUTPUT);
}

void loop()
{
    if (!client.connected())
    {
        reconnect();
    }
    client.loop();
}
