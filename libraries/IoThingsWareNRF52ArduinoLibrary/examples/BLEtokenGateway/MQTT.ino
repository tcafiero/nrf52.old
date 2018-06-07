#include <WiFi.h>
// include library  https://github.com/knolleary/pubsubclient/releases/tag/v2.6
#include <PubSubClient.h>

// Update these with values suitable for your network.
//const char* ssid = "NW001";
//const char* password = "07041957";
const char* ssid = "IoThingsWareBus";
const char* password = "07B04U1957S";
const char* mqttServer = "m23.cloudmqtt.com";
#define mqttPort 16947
#define MQTT_USER "tester"
#define MQTT_PASSWORD "tester"
WiFiClient wifiClient;

PubSubClient client(wifiClient);

void WiFiEvent(WiFiEvent_t event)
{
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch (event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println("WiFi connected");
      Serial.printf("IP address: %s\n", WiFi.localIP());
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      ESP.restart();
      break;
  }
}


void setup_wifi() {
    // delete old config
  WiFi.disconnect(true);
  delay(1000);
  WiFi.onEvent(WiFiEvent);
  delay(10);
  // We start by connecting to a WiFi network
  Serial.printf("\nConnecting to SSID:  %s\n", ssid);
  WiFi.begin(ssid, password);
  int i = 10;
  while (WiFi.status() != WL_CONNECTED) {
    i -= 1;
    if (i < 0) ESP.restart();
    delay(500);
    Serial.print(".");
  }
  Serial.println("Set Server MQTT");
  client.setServer(mqttServer, mqttPort);
}

void MQTTpublish(const char* topic, char* value)
{
  client.publish(topic, value);
}

void MQTTconnect() {
  // Create a unique client ID
  String clientId = "ESP32Client-";
  clientId += String((unsigned int)chipid & 0xffff, HEX);
  client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD);
  // Loop until we're connected
  int i = 10;
  while (!client.connected()) {
    i -= 1;
    // Attempt to reconnect
    if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
      //Serial.println("connected");
      client.setCallback(callback);
    } else {
      Serial.print("failed, rc=");
      Serial.println(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
      if (i < 0) ESP.restart();
    }
  }
}


void callback(char* topic, byte *payload, unsigned int length) {
}

void MQTTloop() {
  client.loop();
}

void MQTTdisconnect(){
  client.disconnect();
}

