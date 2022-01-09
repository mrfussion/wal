/* mgs 22-12-21 */
#include <NewPing.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include "OTA.h"

// Dispositivo
#define DISP "wal01"

// Tiempo de actualizacion en segundos
#define LOOP_TIME 60

// Pines
const int PinTrig = 5;
const int PinEcho = 4;

// Mqtt
char mqtt_broker[] = "broker.emqx.io";
int mqtt_port = 1883;
char mqtt_client[] = DISP;
char mqtt_user[] = "";
char mqtt_pwd[] = "";

// Medidas del tanque expresada en centimetros (cm)
float tanque_diametro = 111;
float tanque_altura = 113.6728;
float tanque_radio = tanque_diametro / 2;

// inicializo objetos
WiFiClient net;
PubSubClient client(mqtt_broker, mqtt_port, net);
NewPing sensor(PinTrig, PinEcho, 200);

// Declaraciones
float distancia, litros,capacidad_total, espacio_vacio, porcentaje_agua;
unsigned long uptime;
unsigned long previousMillis = 0;
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
  res = wm.autoConnect(DISP);
  if(!res) {
    Serial.println("Fallo la conexion");
    ESP.restart();
  } else {
    Serial.print(DISP);
    Serial.println(" conectado");
  }
 
  // Arduino OTA
  InitOTA(DISP);
}

void loop()
{
  // Arduino OTA
  ArduinoOTA.handle();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= (LOOP_TIME * 1000) || previousMillis == 0)
  {
    // despierto el dispositivo
    Serial.print("[up] ");
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
    // espero porque espero, por las dudas
    delay(1000);
    
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
      client.publish(DISP, payload);
      Serial.print(" [mqtt push ok]");
    } else {
      Serial.print(" [mqtt push error]");
    }

    // guardo el valor de tiempo
    previousMillis = millis();

    // Duermo el dispositivo
    // WIFI_LIGHT_SLEEP WIFI_MODEM_SLEEP
    Serial.println(" [sleep]");
    WiFi.setSleepMode(WIFI_LIGHT_SLEEP);
  }
}
