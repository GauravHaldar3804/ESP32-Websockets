#include <Arduino.h>
#include <WiFiMulti.h>
#include <WebSocketsClient.h>

#define WIFI_SSID "Billu"
#define WIFI_PASSWORD "haldarfamily4321"
#define WS_HOST ""
#define WS_PORT 443
#define WS_URL ""



WiFiMulti wifiMulti;
WebSocketsClient wsClient;

void onWSevent(WStype_t type, uint8_t *payload, size_t length){

switch(type){
  case WStype_CONNECTED:
    Serial.println("WS Connected");
    break;
    case WStype_DISCONNECTED:
    Serial.println("WS Disconnected");
    break;
    case WStype_TEXT:
    Serial.printf("WS Message %s\n",payload);
    break;
}
}

void setup() {
  Serial.begin(921600);
  pinMode( LED_BUILTIN , OUTPUT);

  wifiMulti.addAP( WIFI_SSID , WIFI_PASSWORD );

while(wifiMulti.run()!= WL_CONNECTED){
  Serial.print(".");
  delay(100);
}
Serial.print("Connected");
wsClient.beginSSL(WS_HOST,WS_PORT,WS_URL,"","wss");
wsClient.onEvent(onWSevent);
}

void loop() {
  digitalWrite(LED_BUILTIN, WiFi.status() == WL_CONNECTED);
  wsClient.loop();
}

