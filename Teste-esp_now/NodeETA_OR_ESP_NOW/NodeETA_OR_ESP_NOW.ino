#include <espnow.h>
#include <EmonLib.h>
#include <ESP8266WiFi.h>
//#include <WiFi.h>
#define PIN_SENSOR A0
int CONTATO_ENXAGUE = D2;
int CONTATO_AUTO = D1;
int CONTATO_BOMBA = D0;
EnergyMonitor emon;
long lastMsg = 0;

uint8_t broadcastAddress[] = {0x%02,0x%02,0x%02,0x%02,0x%02,0x%02};

typedef struct struct_message {
  int id; // ID unico para cada esp.
  float corrente_alta;
  int estado;
} struct_message;

struct_message meuDado;

void OnSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(sendStatus);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != 0) {
    Serial.println("Problem during ESP-NOW init");
    return;
  }


  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  // Register the peer
  Serial.println("Registering a peer");
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 0, NULL, 0);
  Serial.println("Registering send callback function");
  esp_now_register_send_cb(OnSent);
  

  pinMode(CONTATO_ENXAGUE, INPUT);
  pinMode(CONTATO_AUTO, INPUT);
  pinMode(CONTATO_BOMBA, INPUT);
  emon.current(PIN_SENSOR, 42);
  meuDado.id = 4;
  meuDado.estado = 1;
  meuDado.corrente_alta = 1;
}

void loop() {
  long now = millis();
  float irms = emon.calcIrms(1480);
  if (now-lastMsg>5000)
  {
      meuDado.corrente_alta = irms;

      if (digitalRead(CONTATO_BOMBA)) {
      if (digitalRead(CONTATO_ENXAGUE) == HIGH)
      {
        meuDado.estado = 3;
      }
      if (digitalRead(CONTATO_AUTO) == HIGH)
      {
        meuDado.estado = 1;
      }
      if (!digitalRead(CONTATO_ENXAGUE) && !digitalRead(CONTATO_AUTO)) {
        meuDado.estado = 2;
      }
    }
    if (digitalRead(CONTATO_BOMBA) == LOW) {
      meuDado.estado = 4;
    }
    Serial.println(meuDado.corrente_alta);
    Serial.println(meuDado.estado);
      int result = esp_now_send(NULL, (uint8_t *) &meuDado, sizeof(meuDado));
      result == 0 ? Serial.println("Sent with success"): Serial.println("Error sending the data");
      lastMsg = now;   
  }
  
}
