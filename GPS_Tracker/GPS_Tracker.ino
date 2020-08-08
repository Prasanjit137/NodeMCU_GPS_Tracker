#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

TinyGPSPlus gps;  
SoftwareSerial ss(4, 5);

float latitude , longitude;
String lat_str , lng_str;

const char* ssid = "ssid";
const char* password =  "Password";
const char* mqttServer = "mqtt.pndsn.com";
const int mqttPort = 1883;
const char* clientID = "pub_key/sub_key/secret_key";
const char* channelName = "nodemcu-tracker";

WiFiClient MQTTclient;
PubSubClient client(MQTTclient);

void callback(char* topic, byte* payload, unsigned int length) {
  String payload_buff;
  for (int i=0;i<length;i++) {
    payload_buff = payload_buff+String((char)payload[i]);
  }
  Serial.println(payload_buff); 
}

long lastReconnectAttempt = 0;

boolean reconnect() {
  if (client.connect(clientID)) {
    client.subscribe(channelName); 
  }
  return client.connected();
}

void setup() {
  Serial.begin(9600);
  ss.begin(9600);
  Serial.println("Attempting to connect...");
  WiFi.begin(ssid, password); // Connect to WiFi.
  if(WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.println("Couldn't connect to WiFi.");
      while(1) delay(100);
  }
  client.setServer(mqttServer, mqttPort); // Connect to PubNub.
  client.setCallback(callback);
  lastReconnectAttempt = 0;
}

void loop() {   
  if (!client.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      if (reconnect()) { 
        lastReconnectAttempt = 0;
      }
    }
  }else {
    client.loop();

    while (ss.available() > 0)
      if (gps.encode(ss.read()))
      {
        if (gps.location.isValid())
        {
          latitude = gps.location.lat();
          lat_str = String(latitude , 6);
          longitude = gps.location.lng();
          lng_str = String(longitude , 6);
        }
    
        char latBuf[20];
        char longBuf[20];
        char fullBuf[100];

        dtostrf(latitude, 0, 6, latBuf);
        dtostrf(longitude, 0, 6, longBuf);

        sprintf_P(fullBuf, PSTR("{\"lat\": %s,\"lng\": %s}"), latBuf, longBuf);
    
        //client.subscribe(channelName);
        client.publish(channelName,fullBuf); // Publish message.
        //delay(1000);
      }
  }
}
  
