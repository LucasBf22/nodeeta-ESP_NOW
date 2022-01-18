/*
Autor: Lucas Batista
Codigo para esp receptor por arquitetura ESP_NOW
*/

#include <WiFi.h>
#include <esp_now.h>
#include <ArduinoJson.h>

uint8_t broadcastAddress[] = {0xE0, 0xE2, 0xE6, 0xD0, 0xF9, 0x08};

long lastMsg = 0;
int id = 0;
float sp_recv = 0;
//estrutura do pacote de dados recebidos
typedef struct struct_message_temp {
  int id; //id 1
  float temp;
}struct_message_temp;

typedef struct struct_message_corrente {
  int id; //id 2
  double irms;
}struct_message_corrente;

typedef struct struct_message_ph {
  int id; //id 3
  float ph;
  float sp1;
}struct_message_ph;

typedef struct struct_message_or {
  int id; //id 4
  float irms_alta;
  int estado;
}struct_message_or;

//variavel do dado recebido
struct_message_temp meuDado_temp;
struct_message_corrente meuDado_corrente;
struct_message_ph meuDado_ph;
struct_message_or meuDado_or;


/*--------------------funcao do callback----------------------*/
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {// recebe endereco MAC, dado recebido, tamanho
  char macStr[18];                                                                // vetor de char que vai receber o MAC
  //Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",              // Ponteiro do vetor de char, tamanho do vetor, formato desse vetor
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]); //utilizando o padrao recebido pelo mac_addr
  //Serial.println(macStr);                                                         //Printa o MAC do esp que do qual esta recebendo o dado
  memcpy(&id, incomingData, sizeof(id));                                //Copia o numero de bytes do local apontado para a memoria, salvando o pacote de dados na memoria
  switch (id){
    case 1:
      memcpy(&meuDado_temp, incomingData, sizeof(meuDado_temp));                                //Copia o numero de bytes do local apontado para a memoria, salvando o pacote de dados na memoria
      break;
    case 2:
      memcpy(&meuDado_corrente, incomingData, sizeof(meuDado_corrente));                                //Copia o numero de bytes do local apontado para a memoria, salvando o pacote de dados na memoria
      break;
    case 3:
      memcpy(&meuDado_ph, incomingData, sizeof(meuDado_ph));                                //Copia o numero de bytes do local apontado para a memoria, salvando o pacote de dados na memoria
      break;
    case 4:
      memcpy(&meuDado_or, incomingData, sizeof(meuDado_or));                                //Copia o numero de bytes do local apontado para a memoria, salvando o pacote de dados na memoria
      break;
    default:
      Serial.printf("Nenhum ID cadastrado foi encontrado...");
  }
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
}

void setup() {
  //Initialize Serial Monitor
  Serial.begin(115200);
  
  //Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  
  //Chama a funcao callback de acordo com o dado recebido
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);
}

void loop() {
  StaticJsonDocument<1024> msg;
  long now = millis();
  if (now-lastMsg>10000)
  {
      msg["temp"] = meuDado_temp.temp;
      msg["corrente"] = meuDado_corrente.irms;
      msg["ph"] = meuDado_ph.ph;
      msg["sp1"] = meuDado_ph.sp1;
      msg["corrente_alta"] = meuDado_or.irms_alta;
      msg["estado"] = meuDado_or.estado;
      
      serializeJson(msg, Serial);
      
      meuDado_temp.temp = 0;
      meuDado_corrente.irms = 0;
      meuDado_ph.ph = 0;
      meuDado_ph.sp1 = 0; 
      
      lastMsg = now;   
  }
  if (sp_recv != 0)
  {
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &sp_recv, sizeof(sp_recv));
      result == ESP_OK ? Serial.println("Sent with success"): Serial.println("Error sending the data");
      sp_recv = 0;
  }
}
