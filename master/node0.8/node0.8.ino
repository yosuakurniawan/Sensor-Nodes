/*
GIPE 2021 Smart Farming IoT Code
==================================
Last Updated 24 May 2021

Parts :
- ESP 32
- DHT 11
- LDR
- Soil Moisture Sensor v1.2 (https://makersportal.com/blog/2020/5/26/capacitive-soil-moisture-calibration-with-arduino)

Lib:
- dht11 lib by adafruit
- https://thingsboard.io/docs/samples/esp32/gpio-control-pico-kit-dht22-sensor/

IMPORTANT IF YOU WANNA TEST USING ESP8266::
- DO NOT UPDATE YOUR ESP8266 BOARD DRIVER(bellow board manager) TO 3.0.0!!!!! USE 2.7.4 INSTEAD OR YOU WILL GET AN ERROR FROM THINGSBOARD!!!
- YOU WILL GET AN ERROR WITH ANOTHER ANALOG INPUT SINCE ESP8266 HAS ONLY 1 ANALOG INPUT AND WE USE 2 ANALOG INPUT; THEREFORE ANOTHER ANALOG DOES NOT EXIST

IMPORTANT IF YOU USE ESP 32:
- IF YOURE DUMMY LIKE ME AND HAVE PROBLEM UPLOADING CODE HERE IS THE SOLUTION https://randomnerdtutorials.com/getting-started-with-esp32/

==================================
Made by:
GIPE 2021 Scholars

Tested on DOIT ESP 32 DEV Kit V1 board 24 May 2021
Note : Failed to connect to thingsboard server
*/

#include "ThingsBoard.h"
#include <WiFi.h>           //Setup for ESP32 change to <ESP8266WiFi.h> if you want to use ESP8266(WARNING ESP 8266 has only 1 Analog input!)
//setup ISP
#define WIFI_AP             "SSID"     //PUSPITA AP
#define WIFI_PASSWORD       "Password" //Password
//setup token
#define TOKEN               "tr33crxcEKPGpPpAMAz8"  // node 1
#define THINGSBOARD_SERVER  "http://[2400:6180:0:d0::10cb:8001]:8080/" //our thingsboardserver or use http://178.128.112.223:8080/
// Baud rate for debug serial
#define SERIAL_DEBUG_BAUD   115200

// Initialize ThingsBoard client
WiFiClient espClient;
// Initialize ThingsBoard instance
ThingsBoard tb(espClient);
// the Wifi radio's status
int status = WL_IDLE_STATUS;

//Setup Sensors and Pins

//DHT11
#include "DHT.h"
#define DHTPIN 32            //DHT Pin
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

//LDR
#define LDR 34              //LDR Pin

//Soil Moisture
#define SoilMoisturePin 35  //Soil Moisture Pin (MUST BE ANALOG PIN)
const int AirValue = 620;   //you need to replace this value through calibration, read it when the sensor is placed in open air
const int WaterValue = 310; //you need to replace this value through calibration, read it when the sensor is drowned in water
int soilMoistureValue = 0;
int soilmoisturepercent=0;

void setup() {
  // initialize serial for debugging
  Serial.begin(SERIAL_DEBUG_BAUD);
  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  InitWiFi();
  //DHT11
  dht.begin();
  //Soil Moisture
  //.....
  //LDR
  pinMode(LDR, INPUT);
}

void loop() {
  delay(1000);

  if (WiFi.status() != WL_CONNECTED) {
    reconnect();
  }

  if (!tb.connected()) {
    // Connect to the ThingsBoard
    Serial.print("Connecting to: ");
    Serial.print(THINGSBOARD_SERVER);
    Serial.print(" with token ");
    Serial.println(TOKEN);
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN)) {
      Serial.println("Failed to connect");
      return;
    }
  }

  Serial.println("Sending data...");
  
  dht_readsend();
  delay(100);
  soilmoisture_readsend();
  delay(100);
  LDR_readsend();
  delay(100);
  
  tb.loop();
}


void dht_readsend(){
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  tb.sendTelemetryInt("temperature", t); 
  tb.sendTelemetryFloat("humidity", h);
}

void soilmoisture_readsend(){
  soilMoistureValue = analogRead(SoilMoisturePin);  //put Sensor insert into soil
  Serial.println(soilMoistureValue);
  soilmoisturepercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);
  if(soilmoisturepercent >= 100)
  {
    Serial.println("100 %");
    tb.sendTelemetryInt("Soil Moisture", 100);
  }
  else if(soilmoisturepercent <=0)
  {
    Serial.println("0 %");
    tb.sendTelemetryInt("Soil Moisture", 0);
  }
  else if(soilmoisturepercent >0 && soilmoisturepercent < 100)
  {
    Serial.print(soilmoisturepercent);
    Serial.println("%"); 
    tb.sendTelemetryInt("Soil Moisture", soilmoisturepercent); 
  }
}

void LDR_readsend(){
  int light = analogRead(LDR);
  
  tb.sendTelemetryInt("Light", light); 
}

void InitWiFi()
{
  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network

  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}

void reconnect() {
  // Loop until we're reconnected
  status = WiFi.status();
  if ( status != WL_CONNECTED) {
    WiFi.begin(WIFI_AP, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("Connected to AP");
  }
}
