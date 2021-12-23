/* mgs 22-12-2-21 */
#include <NewPing.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include "OTA.h"

// Tiempo de actualizacion en segundos
#define LOOP_TIME 60

// Pines
const int PinTrig = 5;
const int PinEcho = 4;

// Mqtt
char mqtt_broker[] = "broker.emqx.io";
int mqtt_port = 1883;
char mqtt_client[] = "wal";
char mqtt_user[] = "";
char mqtt_pwd[] = "";

// Medidas del tanque expresada en centimetros
float tanque_diametro = 100;
float tanque_altura = 150;
float tanque_radio = tanque_diametro / 2;

// inicializo objetos
WiFiClient net;
PubSubClient client(mqtt_broker, mqtt_port, net);
NewPing sensor(PinTrig, PinEcho, 200);

// Declaraciones
float distancia, litros,capacidad_total, espacio_vacio, porcentaje_agua;
unsigned long uptime;
int duracion;
char payload[110];

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
  
  // mido la distancia
  duracion = sensor.ping_median(20);
  distancia = duracion / US_ROUNDTRIP_CM;

  // calculo los litros aprox
  // Calculo la capacidad total del tanque con las medidas iniciales en cm del tanque.
  capacidad_total = M_PI * tanque_radio * tanque_radio * tanque_altura;
  
  // Calculo el espacio vacío medido por el sensor usando las medidas iniciales en cm del tanque y 
  // la distancia en cm calculada con el sensor.
  espacio_vacio =  M_PI * tanque_radio * tanque_radio * distancia;
  
  // La resta de la capacidad total y el espacio vacío da en cm3 la cantidad de agua que tiene el tanque, 
  // para pasar de cm3 a litros multiplico por 0.001 o divido por 1000, es lo mismo.
  litros = (capacidad_total - espacio_vacio) * 0.001;

  // porcentaje de agua en el tanque
  porcentaje_agua =  (capacidad_total - espacio_vacio) * 100 / capacidad_total;
  
  //uptime
  uptime = (millis() / 60000);

  // armo un mensaje json con todos los datos
  sprintf(payload, "{\"uptime\":%u,\"distancia\":%g,\"litros\":%g,\"intervalo_actualizacion\":%i,\"porcentaje\":%g}", uptime, distancia, litros, LOOP_TIME, porcentaje_agua);

  // muestro en el monitor
  Serial.print(payload);

  // publico en el broker
  if (client.connect(mqtt_client, mqtt_user, mqtt_pwd)) {
    client.publish("wal", payload);
    Serial.println(" *");
  } else {
    Serial.println(" [mqtt push error]");
  }
  delay(LOOP_TIME * 1000);
}
