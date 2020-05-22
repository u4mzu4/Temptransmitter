#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <credentials.h>

#define ONE_WIRE_BUS 12
#define LEDPIN 13
#define DS18B20_RESOLUTION 11
#define TIMEOUT 5000
#define REFRESHTIME 50000
#define LEDON 500
#define SERVER_PORT 80
#define NR_OF_SENSORS 2

char tempStr[5];
unsigned long lastRefresh;
bool ledIsOn;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensor(&oneWire);
DeviceAddress sensorDeviceAddress[NR_OF_SENSORS];
ESP8266WebServer server(SERVER_PORT);

void handleRoot()
{
  server.send(200, "text/plain", tempStr);
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  digitalWrite(LEDPIN, LOW);
  ledIsOn = 1;
  lastRefresh = millis();
}

void ReadDS18B20()
{
  unsigned short tempRaw[NR_OF_SENSORS];
  static unsigned long timer;
  float tempC;
  unsigned long DS18B20timeout;

  //read sensors
  for (int j = 0; j < NR_OF_SENSORS; j++)
  {
    sensor.requestTemperaturesByAddress(sensorDeviceAddress[j]);
    tempRaw[j] = sensor.getTemp(sensorDeviceAddress[j]);
    DS18B20timeout = millis();
    while (tempRaw[j] == 0x2A80)
    {
      sensor.requestTemperaturesByAddress(sensorDeviceAddress[j]);
      tempRaw[j] = sensor.getTemp(sensorDeviceAddress[j]);
      if (millis() - DS18B20timeout > TIMEOUT)
      {
        break;
      }
    }
  }
  tempC = min(tempRaw[0], tempRaw[1]) / 128.0;
  dtostrf(tempC, 5, 1, tempStr);
  lastRefresh = millis();
}

void setup()
{
  unsigned long wifitimeout;

  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, HIGH);
  sensor.begin();
  for (int i = 0; i < NR_OF_SENSORS; i++)
  {
    sensor.getAddress(sensorDeviceAddress[i], i);
    sensor.setResolution(sensorDeviceAddress[i], DS18B20_RESOLUTION);
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin (ssid, password);
  // Wait for connection
  wifitimeout = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    if (millis() - wifitimeout > TIMEOUT)
    {
      break;
    }
  }
  ReadDS18B20();
  server.on("/", handleRoot);
  server.begin(SERVER_PORT);
}

void loop()
{
  static unsigned long timer;

  server.handleClient();
  if ((millis() - lastRefresh > LEDON) && ledIsOn)
  {
    digitalWrite(LEDPIN, HIGH);
    ledIsOn = 0;
  }
  if (millis() - lastRefresh > REFRESHTIME)
  {
    ReadDS18B20();
  }
}
