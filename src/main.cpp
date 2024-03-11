#include <Arduino.h>
#include <WiFiMulti.h>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>
#include <WifiCredentials.h>


#define WS_HOST ""
#define WS_PORT 443
#define WS_URL ""
#define JSON_DOC_SIZE 2048
#define MSG_SIZE 128



WiFiMulti wifiMulti;
WebSocketsClient wsClient;

uint8_t   toMode(const char *val){

  if (strcmp(val,"output") == 0)
    return OUTPUT;

    if (strcmp(val,"input_pullup") == 0)
    return INPUT_PULLUP;

    if (strcmp(val,"input") == 0)
    return INPUT;
}

void sendErrorMessage(const char *error){
  char msg[MSG_SIZE];

  sprintf(msg, "{\"action\":\"msg\",\"type\":\"error\",\"body\":%s}", error);
  wsClient.sendTXT(msg);
}

void handleMessage( uint8_t *payload){
  JsonDocument doc;

   DeserializationError error = deserializeJson(doc, payload);

  // Test if parsing succeeds
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    sendErrorMessage(error.c_str());
    return;
  }
  if(!doc["type"].is<const char *>()){
    sendErrorMessage("Invalid Message Type");
    return;
  }
  if(strcmp(doc["type"],"cmd") == 0){
    if(!doc["body"].is<JsonObject>()){
      sendErrorMessage("Invalid Body Command");
      return;
    }
  }

  if(strcmp(doc["type"]["body"],"pinMode") == 0){
    pinMode(doc["body"]["pin"], toMode(doc["body"]["mode"]));
    return;
  }

    if(strcmp(doc["type"]["body"],"digitalWrite") == 0){
    digitalWrite(doc["body"]["pin"], doc["body"]["mode"]);
    return;
    }

    if(strcmp(doc["type"]["body"],"digitalRead") == 0){
    auto value = digitalRead(doc["body"]["pin"]);
    char msg[MSG_SIZE];

    sprintf(msg, "{\"action\":\"msg\",\"type\":\"output\",\"body\":%d}", value);
    wsClient.sendTXT(msg);
    return;
  }
}
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

  wifiMulti.addAP( SSID , Password );

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

