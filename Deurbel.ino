/*
  ESP Spacestate switch for bitlair
*/
#include <limits.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

const char* projectName = "Deurbel";

// WiFi settings
const char ssid[] = "Bitlair-things";                  //  your network SSID (name)
const char pass[] = "";                      // your network password
const char* mqtt_server = "mqtt.bitlair.nl";

const uint8_t inputPin = D2; // active high
const unsigned int waitDuration = 6000; // Milliseconds to wait after press

const int BAUD_RATE   = 115200;                       // serial baud rate

// MQTT stuff
char ID[9] = {0};
WiFiClient espClient;
PubSubClient client(espClient);
const char *message = "IK MOEST BELLEN WANT DE KLOP DOET HET NIET";
const char* mqttTopic = "bitlair/doorbell"; // post message
const char* mqttDebugTopic = "bitlair/debug";
long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {
  uint32_t chipid = ESP.getChipId();
  snprintf(ID, sizeof(ID), "%x", chipid);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  pinMode(inputPin, INPUT);

  Serial.begin(BAUD_RATE);
  Serial.println();
  Serial.println(projectName);

  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");

  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(50);
    Serial.print(".");
  }
  Serial.println("");

  Serial.print("WiFi connected to: ");
  Serial.println(ssid);

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void callback(char* topic, byte* payload, unsigned int length) {
  /*Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();*/
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(ID)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      snprintf (msg, 75, "%s (re)connect #%ld", projectName, value);
      Serial.print("Publish message: ");
      Serial.println(msg);
      client.publish(mqttDebugTopic, msg);
      ++value;
      // ... and resubscribe
      client.subscribe(mqttTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void handleSwitches() {
  boolean reading = digitalRead(inputPin);

  if (reading) {
    Serial.print(mqttTopic);
    Serial.print(": ");
    Serial.println(message);

    if (client.publish(mqttTopic, message, false)) {
        Serial.println("MQTT publish succesful!");
        delay(waitDuration);
    } else {
        Serial.println("MQTT publish unsuccesful! Retrying later.");
        delay(100);
    }
    client.subscribe(mqttTopic);
  }
  
  delay(100);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  handleSwitches();
  client.loop();
  yield();
}
