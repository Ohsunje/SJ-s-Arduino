#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#define MSG_BUFFER_SIZE 50
const char* ssid = "ce404";
const char* password = "ce404lab";
const char* mqtt_server = "10.74.122.37";
//broker.hivemq.com – mqtt 브로커 서비스
const char* outTopic = "201908041/Output";
const char* inTopic = "201908041/Input";
const char* clientName;
const char* LED_status = "LED/status";
int LED_PIN = D4;    //GPIO2
int BUTTON_PIN = 0;  //D3
int isPUSH = 0;
String sChipID;
char cChipID[20];
WiFiClient espClient;
PubSubClient client(espClient);
char msg[MSG_BUFFER_SIZE];

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  IPAddress ip(10, 74, 128, 137);
  IPAddress gateway(10, 74, 0, 1);
  IPAddress subnet(255, 255, 0, 0);
  IPAddress myDns(8, 8, 4, 4);
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

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(clientName)) {
      Serial.println("connected");
      client.publish(outTopic, "Reconnected");
      client.subscribe(inTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
    Serial.print((char)payload[i]);
  Serial.println();
  if (strncmp((const char*)payload, "ON", 2) == 0) {
    Serial.println("LOW");
    digitalWrite(LED_PIN, LOW);
  }
  if (strncmp((const char*)payload, "OFF", 3) == 0) {
    Serial.println("HIGH");
    digitalWrite(LED_PIN, HIGH);
  }
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println();
  setup_wifi();
  sChipID = String(ESP.getChipId(), HEX);  //ESP8266 칩번호 확인
  sChipID.toCharArray(cChipID, sChipID.length() + 1);
  clientName = &cChipID[0];
  Serial.println(clientName);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}
void loop() {
  if (digitalRead(LED_PIN) == 0)
    client.publish(LED_status, "ON");
  if (digitalRead(LED_PIN) == 1)
    client.publish(LED_status, "OFF");
  if (!client.connected()) reconnect();
  client.loop();
  if ((digitalRead(BUTTON_PIN) == LOW) && (!isPUSH)) {
    Serial.println("Switch is pushed");
    client.publish(outTopic, "OFF");
    isPUSH = 1;
  } else if ((digitalRead(BUTTON_PIN) == HIGH) && isPUSH) {
    Serial.println("Switch is not pushed");
    client.publish(outTopic, "ON");
    isPUSH = 0;
  }
  delay(1000);
}