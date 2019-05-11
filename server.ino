/************************************************
 * Project: Home Lighting Automation System
 * Date: 2019-May-04
 * Author: William Becerra Gonzalez
 * 
 * Description: This project allows a user to connect via the internet and connect to a NodeMCU ESP8266
 * based circuit that controlls the lighting system via a web interface shows the time and allows indicates if there are users 
 * in the building
 * 
 ************************************************/
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <NTPClient.h>
#include <WifiUdp.h>

const char* ssid = "Schrodinger"; // Wifi name
const char* password = "1993Will"; // wifi password
WiFiUDP ntpUDP;

ESP8266WebServer server(80); // The default HTTP port
NTPClient timeClient(ntpUDP, "ntp1.meraka.csir.co.za");


uint8_t lightControlPin = D1;
uint8_t lightStatus = A0;
uint8_t occupiedPin = D2;
bool LEDStatus = LOW;
bool occupied = false;
bool forceLightsOn  = false;
const int darknessThreshold = 350;

void setup() {
  Serial.begin(115200); // Initializing serial monitor
  pinMode(lightControlPin, OUTPUT);
  digitalWrite(lightControlPin, LOW);
  pinMode(A0, INPUT);
  pinMode(occupiedPin, INPUT);
  // Connecting to WIFI
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", root);
  server.on("/on", lightsOn);
  server.on("/off", lightsOff);
  server.onNotFound(URLNotFound);
  server.begin();
  attachInterrupt(digitalPinToInterrupt(occupiedPin), IntCallback, CHANGE);
}

void IntCallback(){
  if(digitalRead(D2) == HIGH && analogRead(A0) < darknessThreshold){
    onSwitch();
  } else if ((analogRead(A0) >= darknessThreshold || digitalRead(D2) == HIGH)){
    offSwitch();
  } else if (forceLightsOn == false){
    offSwitch();
  }else{
    //nop
  }
}
void loop() {
  server.handleClient();

}

void lightsOn(){
  onSwitch();
  server.send(200, "text/html", root_html());
  forceLightsOn == true;
//  Serial.println("Lights Switched On");
}

void lightsOff(){
  offSwitch();
  server.send(200, "text/html", root_html());
  forceLightsOn = false;
//  Serial.println("Lights Switched OFF");
}

void root(){
//This one must set all states by reading and application logic as well as display the home.html
// if occupied and lights are off they must come on
//  if occupied and lights on they stay on
//  else lights off
//  Must also get time and update it
//    Serial.println("Client Connected");
    server.send(200, "text/html", root_html());
}

void URLNotFound(){
  Serial.println("Client tried a URL not found");
  server.send(404, "text/plain", "URL Not Found");
}

String root_html(){
    timeClient.update();
    String ptr = "<!DOCTYPE html><html> \n";
    ptr       +=  "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"> \n";
    ptr       +=  "<link rel=\"icon\" href=\"data:,\"> \n";
    ptr       +=  "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;} \n";
    ptr       +=  ".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px; \n";
    ptr       +=  "text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;} \n";
    ptr       +=  ".button2 {background-color: #77878A;}</style></head> \n";
    ptr       +=  "<body><h1>ESP8266 Web Server</h1> \n";
    ptr       +=  "<h2> ZA Time: " + timeClient.getFormattedTime() + "</h2> \n";
    String outputState = (LEDStatus == LOW) ? "OFF": "ON";
    Serial.println("Lights are " + outputState); 
    ptr       +=  "<p>Light Control - State " + outputState + "</p> \n";
    ptr       +=  "<p><a href=\"/on\"><button class=\"button\"> ON </button></a></p> \n";
    ptr       +=  "<p><a href=\"/off\"><button class=\"button button2\"> OFF </button></a></p> \n";
    ptr       +=  "<h2> The office is currently: " + isItOccupide() + " and it is " + isDark() + "</h2> \n";
    Serial.println("The office is currently " + isItOccupide() + " and it is " + isDark());
    return ptr;
}

void onSwitch(){
  LEDStatus = HIGH;
  digitalWrite(lightControlPin, HIGH);
//  Serial.println("Lights Turned On");
}

void offSwitch(){
  LEDStatus = LOW;
  digitalWrite(lightControlPin, LOW);
//  Serial.println("Lights Turned Off");
  
}

String isDark(){
  return (analogRead(A0) < darknessThreshold) ? "dark" : "bright" ;
}

String isItOccupide(){
  return (digitalRead(D2) == HIGH) ? "occupied" : "unoccupied" ;
}
