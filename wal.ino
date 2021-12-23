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

// Medidas del tanque expresada en metros
float tanque_diametro = 1;
float tanque_altura = 1.5;

// inicializo objetos
WiFiClient net;
PubSubClient client(mqtt_broker, mqtt_port, net);
NewPing sensor(PinTrig, PinEcho, 200);

// Declaraciones
float distancia, litros;
unsigned long uptime;
int duracion;
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
 
  // Arduino OTA
  InitOTA("wal-OTA");
}

void loop()
{
  // Arduino OTA
  ArduinoOTA.handle();

  
  // mido la distancia (se puede mas prolijo
  duracion = sensor.ping_median();
  distancia = duracion / US_ROUNDTRIP_CM;

  // calculo los litros aprox
  // desarrollar
  litros = (M_PI * (tanque_diametro / 2) * (tanque_diametro /2) * (tanque_altura - (distancia / 100))) * 1000;

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
