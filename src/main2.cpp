#include <Arduino.h>
#include <WiFiMulti.h>
#include <ArduinoJson.h>
#include <Webserver.h>
#include <WebSocketsClient.h>
#include <WiFiCredentials.h>
#include <SPIFFS.h>

WebServer webserver(80);
WiFiMulti wifiMulti;
WebSocketsClient wsClient;

String webpage = "";

#define WS_HOST ""
#define WS_PORT 443
#define WS_URL ""
#define JSON_DOC_SIZE 2048
#define MSG_SIZE 128


void setup() {
  Serial.begin(921600);
  pinMode( LED_BUILTIN , OUTPUT);

  wifiMulti.addAP( SSID , Password );

while(wifiMulti.run()!= WL_CONNECTED){
    sprintf("Establishing connection with %s",SSID);
    Serial.print(".");
    delay(100);
}
Serial.print("Connected");
wsClient.beginSSL(WS_HOST,WS_PORT,WS_URL,"","wss");
// wsClient.onEvent(onWSevent);
webserver.on("/", []()
             { webserver.send(200, "text\html", webpage); });
webserver.begin();


 // Initialize SPIFFS (optional, comment out if already initialized)
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  // Open the file for reading
  File htmlFile = SPIFFS.open("/index.html", "r");

  // Check if the file was opened successfully
  if (!htmlFile) {
    Serial.println("Failed to open index.html");
    return;
  }

  // Read the entire file content into the string variable
  webpage = htmlFile.readStringUntil('\n');

  // Read remaining content (optional, for larger files)
  while (htmlFile.available()) {
    webpage += htmlFile.readStringUntil('\n');
  }

  // Close the file
  htmlFile.close();

  // Start your web server here (assuming you have a web server library)
}



void loop() {
    webserver.handleClient();
    digitalWrite(LED_BUILTIN, WiFi.status() == WL_CONNECTED);
    wsClient.loop();
}

