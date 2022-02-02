#include <DS18B20.h>
#include <esp_now.h>
#include <WiFi.h>
#define pin_led_v 33  //LED ESP LIGADO
#define pin_led_a 26  //LED ESP NOW SENDING/ BLINK IF NOT RECEPTED
long lastMsg = 0;
DS18B20 ds(19);

uint8_t broadcastAddress[] = {0x84,0xCC,0xA8,0x7A,0xC4,0x70};

typedef struct struct_message {
  int id; // ID unico para cada esp.
  float Temp;
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
  pinMode(pin_led_v, OUTPUT);
  pinMode(pin_led_a, OUTPUT);
  digitalWrite(pin_led_a, HIGH);
  digitalWrite(pin_led_v, HIGH);
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

}

void loop() {
  // put your main code here, to run repeatedly:
  long now = millis();
  meuDado.id = 1;
  if (now-lastMsg>5000)
  {
      meuDado.Temp = ds.getTempC();
      meuDado.estatus = true;
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &meuDado, sizeof(meuDado));
      result == ESP_OK ? Serial.println("Sent with success"): Serial.println("Error sending the data");
      lastMsg = now;   
  }
}
