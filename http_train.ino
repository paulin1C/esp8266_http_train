#include "WEMOS_Motor.h"
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ESP8266mDNS.h>

const int timeout = 1000; //max time before emergency break; set to 0 to deactivate

uint8_t direction = _CW;
int speed = 0;
long last_update;
String dir_text[3] = {"", "CCW", "CW"};
Motor M1(0x30,_MOTOR_A, 1000);

ESP8266WebServer server(80);

WiFiManager wifiManager;

void handleRoot() {
  if (server.hasArg("d")){ //use d get argument for direction: 1 or 0
    int d = server.arg("d").toInt();
    if(d==1){
      direction = _CCW;
      Serial.println("_CCW ");
    }else{
      direction = _CW;
      Serial.println("_CW  ");
    }
    server.send(200, "text/html", String(dir_text[direction]));
  }  
  else if (server.hasArg("c")){ //use c get argument to change speed about given value: 0-100
    int c = server.arg("c").toInt();
    speed += c;
    if(speed > 100){
      speed = 100;
    }
    else if(speed < 0){
      speed = 0;
    }
    Serial.println(speed);
    last_update = millis();
    M1.setmotor(direction, speed);      
    server.send(200, "text/html", String(speed));
  }  
  else if (server.hasArg("s")){ //use s get argument for speed: 0-100
    int s = server.arg("s").toInt();
    speed = s;
    if(speed > 100){
      speed = 100;
    }
    else if(speed < 0){
      speed = 0;
    }
    Serial.println(speed);
    last_update = millis();
    M1.setmotor(direction, speed);      
    server.send(200, "text/html", String(speed));
  }  
  else if (server.hasArg("t")){ //use t get argument to reset the timeout
    last_update = millis();
    server.send(200, "text/html", String(speed));
  }  
  else {

    server.send(200, "text/html",
      "<html>\
        <head>\
          <meta charset=\"utf-8\">\
          <title>TRAIN (ESP8266)</title>\
          <style>\
            body { background-color: #000000; font-family: Arial, Helvetica, Sans-Serif; Color: #ffffff; text-align: center; user-select: none;}\
            p {margin: 0px;}\
            .slidecontainer {width: 50%; margin: 0 auto; background-color: #000000;}\
            .slider {-webkit-appearance: none; width: 100%; height: 50px; outline: none; background-color: #000000}\
            .slider::-webkit-slider-thumb {-webkit-appearance: none; appearance: none; height: 45px; width: 65px; background: #f4f4f4; border-radius: 0%; outline-style: none; cursor: pointer; -webkit-transition: .1s;}\
            .slider:hover::-webkit-slider-thumb {background: #BFBFBF;}\
            .slider::-moz-range-thumb { height: 45px; width: 65px;  background: #f4f4f4; border-radius: 0%; border: none; cursor: pointer; transition: background .1s; }\
            .slider:hover::-moz-range-thumb {background: #BFBFBF;}\
            a:link.on_Link , a:visited.on_Link {background-color: #43f436; color: black; width: 40px; padding: 14px 14px; text-align: center; text-decoration: none; display: inline-block; -webkit-transition: .1s; transition: background-color .1s;}\
            a:link.off_Link, a:visited.off_Link {background-color: #f44336; color: white; padding: 14px 14px; width: 40px; text-align: center; text-decoration: none; display: inline-block; -webkit-transition: .1s; transition: background-color .1s;}\
            a:link.neut_Link, a:visited.neut_Link {background-color: #f4f4f4; color: black; padding: 14px 14px; width: 40px; text-align: center; text-decoration: none; display: inline-block; -webkit-transition: .1s; transition: background-color .1s;}\
            a:hover.on_Link , a:active.on_Link {background-color: #35BF2A;}\
            a:hover.off_Link, a:active.off_Link {background-color: #BF352A;}\
            a:hover.neut_Link, a:active.neut_Link {background-color: #BFBFBF;}\
          </style>\
        </head>\
        <body>\
          <h1>TRAIN</h1>\
          <p>\
            </br> timeout=   "+ String(timeout) +" speed=<b id=\"speed\">" + String(speed) + "</b>   dir=<b id=\"direction\">"+ dir_text[direction] +"</b>\
            </br></br>\
            <a href=\"#ccw\" class=\"neut_Link\" data-command=\"d\" data-target=\"1\">CCW</a>\
            <a href=\"#cw\" class=\"neut_Link\" data-command=\"d\" data-target=\"0\">CW</a>\
          </p>\
          <div class=\"slidecontainer\">\
            <input type=\"range\" min=\"0\" max=\"100\" value=\"50\" class=\"slider\" id=\"myRange\">\
          </div>\
          <p>\
            <a href=\"#off\" class=\"off_Link\" data-command=\"s\" data-target=\"-100\">off</a>\
            <a href=\"#dm\" class=\"off_Link\" data-command=\"s\" data-target=\"-10\">--</a>\
            <a href=\"#m\" class=\"off_Link\" data-command=\"s\" data-target=\"-1\">-</a>\
            <a href=\"#p\" class=\"on_Link\" data-command=\"s\" data-target=\"+1\">+</a>\
            <a href=\"#dp\" class=\"on_Link\" data-command=\"s\" data-target=\"+10\">++</a>\
            <a href=\"#max\" class=\"on_Link\" data-command=\"s\" data-target=\"+100\">full</a>\
          </p>\
          </br></br>\ 
          <script type=\"text/javascript\">\
            var slider = document.getElementById(\"myRange\");\
            var test;\
            slider.oninput = function() {\
              let sliderspeed = this.value;\
              clearTimeout(test);\
              test = setTimeout(function(){sendrequest(sliderspeed, 0);}, 50);\
            }\
          </script>\
          <script type=\"text/javascript\">\
            let request = new XMLHttpRequest();\
            function sendrequest(value, whichvalue){\
              console.log(value);\
              request.addEventListener('load', function(event) {\
                if (request.status >= 200 && request.status < 300) {\
                  if (whichvalue == 0 || whichvalue == 1){\
                    response = request.responseText;\
                    document.getElementById(\"speed\").innerHTML = response;\
                    slider.value = response;\
                  } else if (whichvalue == 2){\
                    document.getElementById(\"direction\").innerHTML = request.responseText;\
                  }\
                } else {\
                  console.warn(request.statusText, request.responseText);\
                }\
              });\
              switch(whichvalue){\
                case 0:\
                  request.open(\"GET\", \"/?s=\"+String(value));\
                break;\
                case 1:\
                  request.open(\"GET\", \"/?c=\"+String(value));\
                break;\
                case 2:\
                  request.open(\"GET\", \"/?d=\"+String(value));\
                break;\
              }\
              request.send();\
            }\
            window.onload = function() {\
              let links = document.getElementsByTagName(\"a\");\
              for(let index = 0; index < links.length; index++) {\
                links[index].addEventListener(\"click\", (event) => {\
                  if(event.target != null) {\
                    let command = event.target.dataset.command;\
                    if(command = \"s\"){\
                      sendrequest(event.target.dataset.target, 1);\
                    } else if (command = \"d\"){\
                      sendrequest(event.target.dataset.target, 2);\
                    }\
                  }\
                });\
              }\
              "+ String(timeout==0?"":("\
                setInterval(function(){\
                  let request = new XMLHttpRequest();\
                  request.open(\"GET\", \"/?t=1\");\
                  request.send();\
                }," + String(timeout/2.5))) +")\
            }\
          </script>\
        </body>\
      </html>");

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
  if(timeout!=0 && millis() - last_update > timeout){ //check if we hit the timeout and should emrgency break
    Serial.println("timeout");
    speed = 0;
    M1.setmotor(_STOP);
    last_update = millis();
  }
}
