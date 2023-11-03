#include "DHTesp.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#define DHTpin D5
DHTesp dht;
const char* ssid = "ce404";
const char* password = "ce404lab";
const char* mqtt_server = "10.74.122.37";
const char* outTopicS = "201908041/Status";
const char* outTopicT = "201908041/temperature";
const char* outTopicH = "201908041/humidity";
const char* clientName;
String sChipID;
char cChipID[20];
WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  IPAddress ip(10, 74, 122, 137);
  IPAddress gateway(10, 74, 0, 1);
  IPAddress subnet(255, 255, 0, 0);
  IPAddress myDns(203, 246, 96, 254);
  WiFi.config(ip, myDns, gateway, subnet);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);
  } else {
    digitalWrite(BUILTIN_LED, HIGH);
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(clientName)) {
      Serial.println("connected");
      client.publish(outTopicS, "START");
      //client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  String thisBoard = ARDUINO_BOARD;
  Serial.println(thisBoard);
  dht.setup(DHTpin, DHTesp::DHT11);
  setup_wifi();
  sChipID = String(ESP.getChipId(), HEX);
  sChipID.toCharArray(cChipID, sChipID.length() + 1);
  clientName = &cChipID[0];
  Serial.println(clientName);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  delay(5000);
  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();
  Serial.print("humidity : ");
  Serial.println(humidity, 1);
  Serial.print("temperature : ");
  Serial.println(temperature, 1);
  char tmp[30];
  sprintf(tmp, "%f", temperature);
  client.publish(outTopicT, tmp);
  sprintf(tmp, "%f", humidity);
  client.publish(outTopicH, tmp);
}