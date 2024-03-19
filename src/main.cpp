#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <WiFiCredentials.h>

WebServer server(80);
WebSocketsServer websocket = WebSocketsServer(81);
JsonDocument doc_tx;
JsonDocument doc_rx;
int LED_intensity = 128;
int LED_selected = 1;

int led_1 = 15;
int led_2 = 4;
int led_3 = 5;

const int freq = 5000;
const int led_channels[] = {1, 2, 3};
const int resolution = 8;

String webpage = R"(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>LED Controller</title>
  <style>
    body {
      font-family: sans-serif;
      text-align: center;
    }

    h1 {
      margin-bottom: 20px;
    }

    .led-controls, .intensity-control {
      display: inline-block;
      margin: 0 10px;
    }

    .intensity-control {
      vertical-align: top;
    }

    .slider {
      width: 200px;
    }

    #intensity-value {
      margin-top: 10px;
      font-weight: bold;
    }
  </style>
</head>
<body>
  <h1>LED Controller</h1>
  <div class="led-controls">
    <h2>Select LED:</h2>
    <input type="radio" id="led1" name="led-choice" value="1" checked>
    <label for="led1">LED 1</label><br>
    <input type="radio" id="led2" name="led-choice" value="2">
    <label for="led2">LED 2</label><br>
    <input type="radio" id="led3" name="led-choice" value="3">
    <label for="led3">LED 3</label><br>
  </div>
  <div class="intensity-control">
    <h2>Intensity:</h2>
    <input type="range" min="0" max="255" value="128" id="intensity-slider" class="slider">
    <p id="intensity-value">128</p>
  </div>
  <script>

    var Socket;
 
  function init(){
      Socket = new WebSocket("ws://" + window.location.hostname + ":81/");
      Socket.onmessage = function(event){
        processCommand(event);
      };
  }


    var led1_selected = document.getElementById('led1');
    led1_selected.addEventListener('click',led_changed);
   var led2_selected = document.getElementById('led2');
    led2_selected.addEventListener('click',led_changed);
    var led3_selected = document.getElementById('led3');
    led3_selected.addEventListener('click',led_changed);

    function led_changed(){
      var led_selected = 1;
    if(led2_selected.checked == true){
        led_selected = 2;
    }
    else if(led3_selected.checked == true){
        led_selected = 3;
    }
    console.log(led_selected);

    var msg = {
      type : "LED Selected",
      value : led_selected
    }
    Socket.send(JSON.stringify(msg));
    }

   

    var intensitySlider = document.getElementById('intensity-slider');
    var intensityValue = document.getElementById('intensity-value');
    intensitySlider.addEventListener('click',intensity_changed);

    function intensity_changed(){
        var intensity = intensitySlider.value;
        console.log(intensity);
        var msg = {
      type : "LED Intensity",
      value : intensity
    }
    Socket.send(JSON.stringify(msg));
      }

    intensitySlider.oninput = function() {
      intensityValue.textContent = this.value;
    };

     function processCommand(event){
    var obj = JSON.parse(event.data);
    var type = obj.type;
    if(type.localeCompare("LED Intensity") == 0){
      var LED_intensity = parseInt(obj.value);
      console.log(LED_intensity);
      intensitySlider.value = LED_intensity;
    }
    else if(type.localeCompare("LED Selected") == 0){
      var LED_selected = parseInt(obj.value);
      console.log(LED_selected);
      if(LED_selected == 1){
        led1_selected.checked = true;
      }
      else if(LED_selected == 2){
        led2_selected.checked = true;
      }
      else if(LED_selected == 3){
        led3_selected.checked = true;
      }

    }

  }
  window.onload = function(event){
    init();
  };
  </script>
</body>
</html>



)";

void sendJSON(String type,String value ){
  String jsonstring = "";
  JsonObject object = doc_tx.to<JsonObject>();
  object["type"] = type;
  object["value"] = value;
  serializeJson(doc_tx, jsonstring);
  websocket.broadcastTXT(jsonstring);
}

void onWSevent(byte num ,WStype_t type,uint8_t *payload,size_t length ){
  switch(type){
    case WStype_CONNECTED:
      Serial.println("Client Connected ");
      break;
    case WStype_DISCONNECTED:
      Serial.println("Client Disconnected ");
      break;
    case WStype_TEXT:
      DeserializationError error = deserializeJson(doc_rx, payload);
      if(error){
        Serial.print("deserialization() error");
        return;
      }
      else{
        const char *type = doc_rx["type"];
        const int value = doc_rx["value"];
        
        Serial.println("Recived info");
        Serial.println("Type:"+String(type));
        Serial.println("Value :"+String(value));

        if(String(type)=="LED Intensity"){
          LED_intensity = int(value);
          sendJSON("LED Intensity", String(value));
          ledcWrite(led_channels[LED_selected - 1], LED_intensity);
        }
        if(String(type)=="LED Selected"){
          LED_selected = int(value);
          sendJSON("LED Selected", String(value));
          for (int i = 0; i < 3;i++){
            if(i == LED_selected-1){
              ledcWrite(led_channels[i], LED_intensity);
            }
            else{
              ledcWrite(led_channels[i], 0);

            }
          }
        }
        

        

      }
      break;
  }

}

void setup() {
  Serial.begin(115200);
  ledcSetup(led_channels[0], freq, resolution);
  ledcSetup(led_channels[1], freq, resolution);
  ledcSetup(led_channels[2], freq, resolution);

  ledcAttachPin(led_1, led_channels[0]);
  ledcAttachPin(led_2, led_channels[1]);
  ledcAttachPin(led_3, led_channels[2]);


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
  sendJSON("LED Intensity", String(LED_intensity));
  sendJSON("LED Selected", String(LED_selected));
  ledcWrite(led_channels[LED_selected - 1], LED_intensity);
}



void loop() {
  server.handleClient();
  websocket.loop();

}

