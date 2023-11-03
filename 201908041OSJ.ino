#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <DHTesp.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <TM1637Display.h>

const char* ssid = "ce404";
const char* password = "ce404lab";
const char* mqtt_server = "10.74.122.19";
const char* DayTopic = "DayTopic";
const char* outTopicS = "201908041/Status";
const char* outTopicT = "201908041/temperature";
const char* outTopicH = "201908041/humidity";
const char* DateTopic = "DateTopic";
const char* lightsensor = "lightsensor";
const char* clientName;
unsigned long lastMsg = 0;

int Dayhour, Dayminute, Datevalue, timeoffset;
int hour, minute, displayValue;

DHTesp dht;
String sChipID;
char cChipID[20];
WiFiUDP ntpUDP;
WiFiClient espClient;
PubSubClient client(espClient);
NTPClient timeClient(ntpUDP, "pool.ntp.org", 32400, 60000);

#define CLK D2
#define DIO D3
#define DHTpin D5
#define buzzer D6
TM1637Display display(CLK, DIO);  // TM1637 연결 핀 설정

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);

  IPAddress ip(10, 74, 122, 119);
  IPAddress gateway(10, 74, 0, 1);
  IPAddress mask(255, 255, 0, 0);
  IPAddress DNS1(8, 8, 8, 8);
  IPAddress DNS2(4, 4, 4, 4);
  WiFi.config(ip, gateway, mask, DNS1, DNS2);

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
      client.publish(DayTopic, "Reconnected");
      client.subscribe(DayTopic);
      client.subscribe(DateTopic);
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
  char str[30] = "";
  for (int i = 0; i < length; i++)  {
    Serial.print((char)payload[i]);
    str[i] = (char)payload[i];
  }
  Serial.println();
  if (strcmp(topic, DayTopic) == 0) {
    long payloadValue = atoi(str);
    Dayhour = (payloadValue / 3600000);
    Dayminute = (payloadValue % 3600000) / 60000;
    Serial.print("Day Hour : ");
    Serial.println(Dayhour);
    Serial.print("Day Minute : ");
    Serial.println(Dayminute);
  }
}

void setup() {
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  setup_wifi();
  sChipID = String(ESP.getChipId(), HEX);  //ESP8266 칩번호 확인
  sChipID.toCharArray(cChipID, sChipID.length() + 1);
  clientName = &cChipID[0];
  Serial.println(clientName);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  Serial.begin(115200);
  dht.setup(DHTpin, DHTesp::DHT11);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  timeClient.begin();
  timeClient.update();
  display.setBrightness(7);  // 디스플레이 밝기 설정
}

void displayTime() {
  hour = timeClient.getHours();
  minute = timeClient.getMinutes();
  displayValue = hour * 100 + minute;
  display.showNumberDecEx(displayValue, true);
  Serial.print("Hour : ");
  Serial.println(hour);
  Serial.print("Minute : ");
  Serial.println(minute);
  if (Dayhour == hour && Dayminute == minute)
    digitalWrite(buzzer, 1);
  else
    digitalWrite(buzzer, 0);
}

void temp() {
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

void light() {
  int light = analogRead(A0);
  Serial.print("Light : ");
  Serial.println(light);
  char tmp[30];
  sprintf(tmp, "%d", light);
  client.publish(lightsensor, tmp);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();
  temp();
  light();
  displayTime();
  delay(1000);
}