#include "WEMOS_Motor.h"
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>

const int timeout = 1000; //max time before emergency break; set to 0 to deactivate

uint8_t direction = _CW;
uint8_t accel_dir = _CW;
int speed = 0;
int accel = 0;
boolean accel_on = false;
boolean reverse_delay = false;
long reverse_delay_time;
boolean udp_on = true;
long last_update;
String dir_text[3] = {"", "CCW", "CW"};
boolean front = true;
Motor M1(0x30,_MOTOR_A, 1000);

ESP8266WebServer server(80);

WiFiUDP Udp;
unsigned int localPort = 8888;
char packetBuffer[255]; //buffer to hold incoming packet
char ReplyBuffer[] = "acknowledged";       // a string to send back
bool thisPacketIsNotEmpty = false;
bool previousPacketIsLoaded = false;


WiFiManager wifiManager;

void handleRoot() {
  if (server.hasArg("d")){ //use d get argument for direction: 1 or 0
    int d = server.arg("d").toInt();
    if(front){
        direction = d+1;
      }else{
        direction = 2-d;
      }
    Serial.println(dir_text[direction]);
    last_update = millis();
    response();
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
    
    last_update = millis();
    response();
  }
  else if (server.hasArg("udp")){ //use c get argument to change speed about given value: 0-100
    if(udp_on){udp_on = false;}
    else{udp_on = true;}
    server.send(200, "text/html", "u|" + String(udp_on));
    Serial.println("UDP:" + String(udp_on));
  }
  else if (server.hasArg("accel")){ //use c get argument to change speed about given value: 0-100
    if(accel_on){accel_on = false;}
    else{accel_on = true;}
    server.send(200, "text/html", "a|" + String(accel_on));
    Serial.println("ACCEL:" + String(accel_on));
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
    
    last_update = millis();
    response();
  }  
  else if (server.hasArg("t")){ //use t get argument to reset the timeout
    last_update = millis();
    response();
  } 
  else if (server.hasArg("flip")){ //use t get argument to reset the timeout
    if(front){
      front = false;
    }else{
      front = true;
    }
    if(direction == 1){
      direction = 2;
    }else{
      direction = 1;
    }
    if(accel_dir == 1){
      accel_dir = 2;
    }else{
      accel_dir = 1;
    }
    server.send(200, "text/html", "f|" + String(front));
    Serial.println("fliped " + String(front));
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
            a:link.toggle_Link , a:visited.on_Link {background-color: #43f436; color: black; width: 40px; padding: 14px 14px; text-align: center; text-decoration: none; display: inline-block; -webkit-transition: .1s; transition: background-color .1s;}\
            a:hover.toggle_Link , a:active.on_Link {background-color: #35BF2A;}\
          </style>\
        </head>\
        <body>\
          <h1>TRAIN</h1>\n\
          <p>\
            </br> timeout=   "+ String(timeout) +" speed=<b id=\"speed\">" + String(speed) + "</b>   dir=<b id=\"direction\">"+ dir_text[direction] +"</b>\
            </br></br>\
            <a href=\"#ccw\" class=\"neut_Link\" data-command=\"d\" data-target=\"1\">CCW</a>\
            <a href=\"#cw\" class=\"neut_Link\" data-command=\"d\" data-target=\"0\">CW</a>\
          </p>\n\
          <div class=\"slidecontainer\">\
            <input type=\"range\" min=\"0\" max=\"100\" value=\"50\" class=\"slider\" id=\"myRange\">\
          </div>\n\
          <p>\
            <a href=\"#off\" class=\"off_Link\" data-command=\"s\" data-target=\"-100\">off</a>\
            <a href=\"#dm\" class=\"off_Link\" data-command=\"s\" data-target=\"-10\">--</a>\
            <a href=\"#m\" class=\"off_Link\" data-command=\"s\" data-target=\"-1\">-</a>\
            <a href=\"#p\" class=\"on_Link\" data-command=\"s\" data-target=\"+1\">+</a>\
            <a href=\"#dp\" class=\"on_Link\" data-command=\"s\" data-target=\"+10\">++</a>\
            <a href=\"#max\" class=\"on_Link\" data-command=\"s\" data-target=\"+100\">full</a>\
          </p>\
          </br>\n\
          <p>\
            <a href=\"#udp\" id=\"udp\" class=\"toggle_Link\" data-command=\"udp\" data-target=\"0\">UDP</a>\
            <a href=\"#flip\" id=\"flip\" class=\"toggle_Link\" data-command=\"flip\" data-target=\"0\"><-></a>\
            <a href=\"#accel\" id=\"accel\" class=\"toggle_Link\" data-command=\"accel\" data-target=\"0\">ACC</a>\
          </p>\
          </br></br>\n\
          <script type=\"text/javascript\">\n\
            var slider = document.getElementById(\"myRange\");\n\
            var test;\n\
            slider.oninput = function() {\n\
              let sliderspeed = this.value;\n\
              clearTimeout(test);\n\
              test = setTimeout(function(){sendrequest(sliderspeed, 0);}, 50);\n\
            }\n\
          </script>\n\
          <script type=\"text/javascript\">\n\
            var request = new XMLHttpRequest();\n\
            function sendrequest(value, whichvalue){\n\
              console.log(value);\n\
              switch(whichvalue){\n\
                case 0:\n\
                  request.open(\"GET\", \"/?s=\"+String(value));\n\
                break;\n\
                case 1:\n\
                  request.open(\"GET\", \"/?c=\"+String(value));\n\
                break;\n\
                case 2:\n\
                  request.open(\"GET\", \"/?d=\"+String(value));\n\
                break;\n\
                case 3:\n\
                  request.open(\"GET\", \"/?udp=0\");\n\
                break;\n\
                case 4:\n\
                  request.open(\"GET\", \"/?accel=0\");\n\
                break;\n\
                case 5:\n\
                  request.open(\"GET\", \"/?flip=0\");\n\
                break;\n\
              }\n\
              request.send();\n\
            }\n\
            window.onload = function() {\n\
              document.getElementById(\"accel\").style.backgroundColor = \"#f44336\";\n\
              request.addEventListener('load', function(event) {\n\
                if (request.status >= 200 && request.status < 300) {\n\
                  response = request.responseText;\n\
                  let segment = 0;\n\
                  let rspeed = \"\";\n\
                  let rdir = \"\";\n\
                  if (response[0] == \"s\"){\n\
                    for (i in response){\n\
                      if (response[i] == \"|\"){\n\
                        segment ++;\n\
                      }\n\
                      else if(segment == 1){\n\
                        rspeed += response[i];\n\
                      }\n\
                      else if(segment == 2){\n\
                        rdir += response[i];\n\
                      }\n\
                    }\n\
                    document.getElementById(\"speed\").innerHTML = rspeed;\n\
                    document.getElementById(\"direction\").innerHTML = rdir;\n\
                    slider.value = rspeed;\n\
                  }else if(response[0]==\"u\"){\n\
                    if(response[2] == \"0\"){\n\
                      document.getElementById(\"udp\").style.backgroundColor = \"#f44336\";\n\
                    }else{\n\
                      document.getElementById(\"udp\").style.backgroundColor = \"#43f436\";\n\
                    }\n\
                  }else if(response[0]==\"a\"){\n\
                    if(response[2] == \"0\"){\n\
                      document.getElementById(\"accel\").style.backgroundColor = \"#f44336\";\n\
                    }else{\n\
                      document.getElementById(\"accel\").style.backgroundColor = \"#43f436\";\n\
                    }\n\
                  }\n\
                }\n\
              });\n\
              let links = document.getElementsByTagName(\"a\");\n\
              for(let index = 0; index < links.length; index++) {\n\
                links[index].addEventListener(\"click\", (event) => {\n\
                  if(event.target != null) {\n\
                    let command = event.target.dataset.command;\n\
                    if(command == \"s\"){\n\
                      sendrequest(event.target.dataset.target, 1);\n\
                    } else if (command == \"d\"){\n\
                      sendrequest(event.target.dataset.target, 2);\n\
                    } else if (command == \"udp\"){\n\
                      sendrequest(event.target.dataset.target, 3);\n\
                    } else if (command == \"accel\"){\n\
                      sendrequest(event.target.dataset.target, 4);\n\
                    } else if (command == \"flip\"){\n\
                      sendrequest(event.target.dataset.target, 5);\n\
                    }\n\
                  }\n\
                });\n\
              }\n\
            }\n\
          </script>\n\
        </body>\n\
      </html>");

  }
}

void response(){
  server.send(200, "text/html", "s|" + String(speed)+ "|" + String(dir_text[direction]));
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
  Udp.begin(localPort);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  if(udp_on){
    while (true) {
      thisPacketIsNotEmpty = (Udp.parsePacket() > 0);
  
      if (thisPacketIsNotEmpty) { // raise flag that a packet is loaded and read it in the buffer
        previousPacketIsLoaded = true;
        Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
      }
      else if (!thisPacketIsNotEmpty && previousPacketIsLoaded) { // if the current packet is empty, but a loaded packet exists, break out of the loop
        previousPacketIsLoaded = false;
        accel = int(packetBuffer[0]);
        switch(packetBuffer[1]){
          case 0:
            accel_dir = _CCW;
          break;
          case 1:
            accel_dir = _CW;
          break;
        }
        if(!front){
          accel_dir = 3-accel_dir;
        }
        last_update = millis();     
        break;
      }
      else { // no packet received
        break;
      }
    }
    thisPacketIsNotEmpty = false;
    previousPacketIsLoaded = false;
  
    if (accel_on){
      if(reverse_delay){
        if(reverse_delay_time + 500 < millis()){
          reverse_delay = false;
        }
        speed = 0;
      }else{
        speed = speed-0.08*speed;
        if(accel_dir == direction){
          speed += accel / 10;
        } else{
          speed -= accel / 5;
        }
      
        if (speed < 0){
          switch (direction){
            case 1:
              direction = 2;
            break;
            case 2:
              direction = 1;
            break;
          }
          speed = 0;
          reverse_delay = true;
          reverse_delay_time = millis();
        } else if (speed > 100){
          speed = 100;
        }
      }
    }else{
      speed = accel;
      direction = accel_dir;
    }
  }
  
  if(speed > 1){M1.setmotor(direction, speed*0.7+30);}
  else{M1.setmotor(direction, 0);}
  analogWrite(LED_BUILTIN, speed);
  for(int i = 0; i < 50 + speed/2 - (direction-1)*speed; i++){
    Serial.print(" ");
  }
  Serial.println(String(speed) + " | " + dir_text[direction]);
  //Serial.println("O");
  
  server.handleClient();
  if(timeout!=0 && millis() - last_update > timeout){ //check if we hit the timeout and should emrgency break
    Serial.println("timeout");
    speed = 0;
    M1.setmotor(_STOP);
  }
}
