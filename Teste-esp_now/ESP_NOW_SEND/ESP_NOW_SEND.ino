#include <esp_now.h>
#include <WiFi.h>
long lastMsg = 0;

uint8_t broadcastAddress[] = {0x%02,0x%02,0x%02,0x%02,0x%02,0x%02};

typedef struct struct_message {
  int id; // ID unico para cada esp.
  int x;
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

}

void loop() {
  // put your main code here, to run repeatedly:

  meuDado.id = random(1,2);
  meuDado.x = random(1,50);

  long now = millis();
  if (now-lastMsg>5000)
  {
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &meuDado, sizeof(meuDado));
      result == ESP_OK ? Serial.println("Sent with success"): Serial.println("Error sending the data");
      lastMsg = now;   
  }
  
}
