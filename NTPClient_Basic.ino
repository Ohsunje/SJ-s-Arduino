#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char *ssid = "SK_WiFiGIGADE73";
const char *password = "1801025581";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  /*
  IPAddress ip(10, 74, 122, 137);
  IPAddress gateway(10, 74, 0, 1);
  IPAddress mask(255, 255, 0, 0);
  IPAddress DNS1(8, 8, 8 ,8);
  IPAddress DNS2(4, 4, 4, 4);
  WiFi.config(ip, gateway, mask, DNS1, DNS2);
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

void setup() {
  setup_wifi();
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  timeClient.begin();
}

void loop() {
  timeClient.update();
  Serial.println(timeClient.getFormattedTime());
  delay(1000);
}