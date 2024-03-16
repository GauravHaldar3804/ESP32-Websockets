#include <WiFi.h>
#include <WebServer.h>
#include <WiFiCredentials.h>

WebServer server(80);
String webpage = R"(<!DOCTYPE html>
<html lang='en'>
<head>
  <meta charset='UTF-8'>
  <meta name='viewport' content='width=device-width,initial-scale=1'>
  <title>Simple Webserver</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      margin: 0;
      padding: 20px;
      text-align: center;
    }

    h1 {
      font-size: 2em;
      margin-bottom: 15px;
    }
  </style>
</head>
<body>
  <h1>Simple Webserver</h1>
</body>)";
void setup() {
  Serial.begin(115200);
  WiFi.begin(SSID, Password);
  Serial.println("Establishing connection with " + String(SSID));

  while(WiFi.status() != WL_CONNECTED){
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi network with IP address :");
  Serial.print(WiFi.localIP());
  server.on("/", []
            { server.send(200,"text/html",webpage); });
  server.begin();
}



void loop() {
  server.handleClient();
}

