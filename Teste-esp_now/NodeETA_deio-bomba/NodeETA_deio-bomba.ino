#include <esp_now.h>
#include <WiFi.h>
#include <EmonLib.h>  // biblioteca do sensor de corrente
#define PIN_SENSOR 34

EnergyMonitor emon;  // cria o objeto p/ interagir com o sensor

long lastMsg = 0;

uint8_t broadcastAddress[] = {0x84,0xCC,0xA8,0x7A,0xC4,0x70};

typedef struct struct_message {
  int id; // ID unico para cada esp.
  double irms;
  bool estatus;
} struct_message;

struct_message meuDado;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  emon.current(PIN_SENSOR, 12);   //D34 -> entrada analogica  12-> calibracao

    meuDado.id = 2;
}

void loop() {
  // put your main code here, to run repeatedly:

  long now = millis();
  if (now-lastMsg>5000)
  {
      meuDado.irms = emon.calcIrms(1480);
      meuDado.status = true;
      Serial.println(meuDado.irms);
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &meuDado, sizeof(meuDado));
      result == ESP_OK ? Serial.println("Sent with success"): Serial.println("Error sending the data");
      lastMsg = now;   
  }
}
