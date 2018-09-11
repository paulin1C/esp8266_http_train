#include "WEMOS_Motor.h"
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ESP8266mDNS.h>

const int timeout = 1000; //max time before emergency break

uint8_t direction = _CW;
int speed = 0;
long last_update;
Motor M1(0x30,_MOTOR_A, 1000);

ESP8266WebServer server(80);

WiFiManager wifiManager;

void handleRoot() {
	server.send(200, "text/html", "okay"); //the http payload isn't important
  if (server.hasArg("d")){ //use d get argument for direction: 1 or 0
    int d = server.arg("d").toInt();
    if(d==1){
      direction = _CCW;
      Serial.print("_CCW ");
    }else{
      direction = _CW;
      Serial.print("_CW  ");
    }
  }
  if (server.hasArg("s")){ //use s get argument for speed: 0-100
    int s = server.arg("s").toInt();
    if(s >= 0 && s <= 100){
      speed = s;
      Serial.println(speed);
      last_update = millis();
      M1.setmotor(direction, speed);      
    }
  }
}

void setup() {
	delay(1000);
	Serial.begin(115200);
	Serial.println("trying to connect to wifi");
  wifiManager.autoConnect("train");
	Serial.print("connected to wifi");
  if (!MDNS.begin("train")) {
    Serial.println("Error setting up MDNS responder!");
    while(1) { 
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
	server.on("/", handleRoot);
	server.begin();
	Serial.println("HTTP server started");
  MDNS.addService("esp", "tcp", 80); //add mdms to be found by controller
  last_update = millis();
}

void loop() {
	server.handleClient();
  if(millis() - last_update > timeout){ //check if we hit the timeout and should emrgency break
    Serial.println("timeout");
    speed = 0;
    M1.setmotor(_STOP);
    last_update = millis();
  }
}



















