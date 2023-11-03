#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
const char *ssid = "SK_WiFiGIGADE73";
const char *password = "1801025581";
const char *mqtt_server = "broker.hivemq.com";
const char *DayTopic = "Day";
const char *clientName;
String sChipID;
char cChipID[20];
WiFiUDP ntpUDP;
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 32400, 60000);
//timeClient(NTP 클라이언트 이름, NTP 서버 주소, GMT조절, 클라이언트 업데이트 주기)
//GMT조절: 대한민국 GMT+9이므로 9*60*60 = 32400초
//클라이언트 업데이트 주기는 ms단위(1시간 주기:1시간*3600초*1000=3600000ms)
const int led = 2;
int hour;
int minute;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  /*
  IPAddress ip(10, 74, 127, 137);
  IPAddress gateway(10, 74, 0, 1);
  IPAddress subnet(255, 255, 0, 0);
  IPAddress myDns(203, 246, 96, 254);
  WiFi.config(ip, myDns, gateway, subnet);
  */
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
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
    Serial.print((char)payload[i]);
  Serial.println();
  long payloadValue = atol((char*)payload);
  hour = (payloadValue / 3600000);
  minute = (payloadValue % 3600000) / 60000;
  client.publish(DayTopic, reinterpret_cast<char *>(payload));
  Serial.print("Hour : ");
  Serial.println(hour);
  Serial.print("Minute : ");
  Serial.println(minute);
}

void setup() {
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
  pinMode(led, OUTPUT);
  digitalWrite(led, 1);
  //timeClient.setTimeOffset(32400);
  timeClient.begin();
  Serial.println("");
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();
  time_t nowTime;
  struct tm *ts;
  char buf[80];
  timeClient.update();
  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    nowTime = timeClient.getEpochTime();
    ts = localtime(&nowTime);
    Serial.print(ts->tm_year + 1900);
    Serial.print("년 ");
    Serial.print(ts->tm_mon + 1);
    Serial.print("월 ");
    Serial.print(ts->tm_mday);
    Serial.print("일 ");
    Serial.print(ts->tm_hour);
    Serial.print("시 ");
    Serial.print(ts->tm_min);
    Serial.print("분 ");
    Serial.print(ts->tm_sec);
    Serial.println("초 ");
    Serial.print("Hour : ");
    Serial.println(hour);
    strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);
    Serial.println(buf);
    Serial.println(timeClient.getFormattedTime());
    Serial.println();
  }
  if (timeClient.isTimeSet()) {
    if (hour == timeClient.getHours() && minute == timeClient.getMinutes())
      digitalWrite(led, 0);
  
}