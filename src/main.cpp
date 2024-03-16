#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <WiFiCredentials.h>

WebServer server(80);
WebSocketsServer websocket = WebSocketsServer(81);
int interval = 1000;
unsigned long previousMillis = 0;

String webpage = R"(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Random Number Generator</title>
</head>
<body>
<h1>Random Number Generator</h1>
<p>The random number is :<span id = "rand">-</span></p>
<p><button type = "button" id = "BTN_SEND_BACK">Sending stuff back</button></p>

</body>
<script>
  var Socket;
  document.getElementById("BTN_SEND_BACK").addEventListener("click",button_send_back);
  function init(){
      Socket = new WebSocket("ws://" + window.location.hostname + ":81/");
      Socket.onmessage = function(event){
        processCommand(event);
      };
  }
  function button_send_back(){
    Socket.send("Some random stuff hehehehehe");
    console.log("Some random stuff hehehehehe")
  }
  function processCommand(event){
    document.getElementById('rand').innerHTML = event.data;
    console.log(event.data);
  }
  window.onload = function(event){
    init();
  };

</script>
</html>
)";

void onWSevent(byte num ,WStype_t type,uint8_t *payload,size_t length ){
  switch(type){
    case WStype_CONNECTED:
      Serial.println("Client Connected ");
      break;
    case WStype_DISCONNECTED:
      Serial.println("Client Disconnected ");
      break;
    case WStype_TEXT:
      for (int i = 0; i < length ; i++){
        Serial.print((char)payload[i]);
        
      }
      Serial.println("");
      break;
  }

}

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
  websocket.begin();
  websocket.onEvent(onWSevent);
}



void loop() {
  server.handleClient();
  websocket.loop();

  unsigned long now = millis();
  if(now-previousMillis > interval){
    String str = String(random(100));
    int str_len = str.length() + 1;
    char char_array[str_len];
    str.toCharArray(char_array, str_len);
    websocket.broadcastTXT(char_array);
    previousMillis = now;
  }
}

