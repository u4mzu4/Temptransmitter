#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS D7
#define DS18B20_RESOLUTION 11
#define SENSORPIN A0
#define TIMEOUT 5000
#define REFRESHTIME 60000
#define SERVER_PORT 80

const char* ssid     = "";
const char* password = "";

char tempstr[5];

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensor(&oneWire);
DeviceAddress sensorDeviceAddress0;
DeviceAddress sensorDeviceAddress1;
ESP8266WebServer server(SERVER_PORT);

void handleRoot() {
  server.send(200, "text/plain", tempstr);
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
}

void setup() {
  unsigned long wifitimeout;
  // put your setup code here, to run once:
  sensor.begin();
  sensor.getAddress(sensorDeviceAddress0, 0);
  sensor.getAddress(sensorDeviceAddress1, 1);
  sensor.setResolution(sensorDeviceAddress0, DS18B20_RESOLUTION);
  sensor.setResolution(sensorDeviceAddress1, DS18B20_RESOLUTION);

  WiFi.mode(WIFI_STA);
  WiFi.begin (ssid, password);
  // Wait for connection
  wifitimeout = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    if (millis()-wifitimeout > TIMEOUT)
    {
      break;
    }
  }
  server.on("/", handleRoot);
  server.begin(SERVER_PORT);
}

void loop() {
  unsigned short tempRaw0;
  unsigned short tempRaw1;
  static unsigned long timer;
  int sensorValue;
  float tempC;
  unsigned long DS18B20timeout;
  //read sensor

  server.handleClient();
  if (millis()-timer>REFRESHTIME)
  {
    sensor.requestTemperaturesByAddress(sensorDeviceAddress0);
    tempRaw0 = sensor.getTemp(sensorDeviceAddress0);
    DS18B20timeout = millis();
    while (tempRaw0 == 0x2A80)
    {
      sensor.requestTemperaturesByAddress(sensorDeviceAddress0);
      tempRaw0 = sensor.getTemp(sensorDeviceAddress0);
      if (millis()-DS18B20timeout > TIMEOUT)
        {
        break;
        }
    }
    sensor.requestTemperaturesByAddress(sensorDeviceAddress1);
    tempRaw1 = sensor.getTemp(sensorDeviceAddress1);
    DS18B20timeout = millis();
    while (tempRaw1 == 0x2A80)
    {
      sensor.requestTemperaturesByAddress(sensorDeviceAddress1);
      tempRaw1 = sensor.getTemp(sensorDeviceAddress1);
      if (millis()-DS18B20timeout > TIMEOUT)
        {
        break;
        }
    }
    tempC=min(tempRaw0,tempRaw1)/128.0;
    sensorValue = analogRead(SENSORPIN);
    dtostrf(tempC, 5, 1, tempstr);
    timer=millis();
  }
}
