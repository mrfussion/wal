/* mgs 22-12-2-21 */
#include <NewPing.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include "OTA.h"

// Pines
const int PinTrig = 5;
const int PinEcho = 4;

// Mqtt
char mqtt_broker[] = "192.168.1.8";
int mqtt_port = 1883;
char mqtt_client[] = "wal";
char mqtt_user[] = "wal";
char mqtt_pwd[] = "wal2021";

// Wifi
WiFiClient net;

// MQTT
PubSubClient client(mqtt_broker, mqtt_port, net);

// Sensor ultrasonico
NewPing sensor(PinTrig, PinEcho, 200);

// Declaraciones
float distancia, litros;
unsigned long duracion, uptime;
char payload[100];

void setup()
{
  // Monitor
  Serial.begin(115200);

  // Wifi
  WiFi.mode(WIFI_STA);
  WiFiManager wm;
  // esto se usa para resetear la config de wifi
  // descomentar, subir, volver a comentar y subir
  //wm.resetSettings();
  bool res;
  res = wm.autoConnect("wal");
  if(!res) {
    Serial.println("Fallo la conexion");
    ESP.restart();
  } else {
    Serial.println("conectado! :)");
  }
  
  // Pines
  //pinMode(PinTrig, OUTPUT);
  //pinMode(PinEcho, INPUT);

  // Arduino OTA
  InitOTA("wal-OTA");
}

void loop()
{
  // Arduino OTA
  ArduinoOTA.handle();

  
  // mido la distancia (se puede mas prolijo
  int duracion = sensor.ping_median();
  //iniciarTrigger();
  //duracion = pulseIn(PinEcho, HIGH);
  //distancia = duracion * 0.034 /2;
  distancia = duracion / US_ROUNDTRIP_CM;

  // calculo los litros aprox
  // desarrollar
  litros = 10;

  //uptime
  uptime = (millis() / 60000);

  // armo un mensaje json con todos los datos
  sprintf(payload, "{\"uptime\":%u,\"distancia\":%g,\"litros\":%g}", uptime, distancia, litros);

  // muestro en el monitor
  Serial.println(payload);

  // publico en el broker
  if (client.connect(mqtt_client, mqtt_user, mqtt_pwd)) {
    client.publish("wal", payload);
    Serial.println("MQTT publicado");
  }
  delay(10 * 1000);
}

void iniciarTrigger()
{
  digitalWrite(PinTrig, LOW);
  delayMicroseconds(2);

  digitalWrite(PinTrig, HIGH);
  delayMicroseconds(10);

  digitalWrite(PinTrig, LOW);
}
