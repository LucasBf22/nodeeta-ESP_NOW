/*
Autor: Lucas Batista
Codigo para esp receptor por arquitetura ESP_NOW
*/

#include <WiFi.h>
#include <esp_now.h>
#include <ArduinoJson.h>
#define SAMPLE_TIME 10000 //milisegundos
uint8_t esp_ph_address[] = {0x30,0x83,0x98,0x00,0xA2,0xDC};

unsigned long lastMsg = 0;
int id = 0;
float sp_recv = 0;
//estrutura do pacote de dados recebidos
typedef struct struct_message_temp {
  int id; //id 1
  float temp;
  bool estatus;
}struct_message_temp;

typedef struct struct_message_corrente {
  int id; //id 2
  double irms;
  bool estatus;
}struct_message_corrente;

typedef struct struct_message_ph {
  int id; //id 3
  float ph;
  float sp1;
  bool estatus;
}struct_message_ph;

typedef struct struct_message_or {
  int id; //id 4
  float irms_alta;
  int estado;
  bool estatus;
}struct_message_or;

//variavel do dado recebido
struct_message_temp my_data_temp;
struct_message_corrente my_data_corrente;
struct_message_ph my_data_ph;
struct_message_or my_data_or;


/*--------------------funcao do callback----------------------*/
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {// recebe endereco MAC, dado recebido, tamanho
  memcpy(&id, incomingData, sizeof(id));                                //Copia o numero de bytes do local apontado para a memoria, salvando o pacote de dados na memoria
  switch (id){
    case 1:
      memcpy(&my_data_temp, incomingData, sizeof(my_data_temp));                                //Copia o numero de bytes do local apontado para a memoria, salvando o pacote de dados na memoria
      break;
    case 2:
      memcpy(&my_data_corrente, incomingData, sizeof(my_data_corrente));                                //Copia o numero de bytes do local apontado para a memoria, salvando o pacote de dados na memoria
      break;
    case 3:
      memcpy(&my_data_ph, incomingData, sizeof(my_data_ph));                                //Copia o numero de bytes do local apontado para a memoria, salvando o pacote de dados na memoria
      break;
    case 4:
      memcpy(&my_data_or, incomingData, sizeof(my_data_or));                                //Copia o numero de bytes do local apontado para a memoria, salvando o pacote de dados na memoria
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

   esp_now_register_send_cb(OnDataSent);
   
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, esp_ph_address, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  
  //Chama a funcao callback de acordo com o dado recebido
  esp_now_register_recv_cb(OnDataRecv);
 
}

void loop() {
  if (Serial.available() > 0) {
    sp_recv = Serial.parseFloat();
  }
  StaticJsonDocument<1024> msg;
  unsigned long now = millis();
  if (now-lastMsg>SAMPLE_TIME)
  {
      msg["temp"] = my_data_temp.temp;
      msg["corrente"] = my_data_corrente.irms;
      msg["ph"] = my_data_ph.ph;
      msg["sp1"] = my_data_ph.sp1;
      msg["corrente_alta"] = my_data_or.irms_alta;
      msg["estado"] = my_data_or.estado;
      msg["estatus_esp-temp"] = my_data_temp.estatus;
      msg["estatus_esp-deio"] = my_data_corrente.estatus;
      msg["estatus_esp-ph"] = my_data_ph.estatus;
      msg["estatus_esp-or"] = my_data_or.estatus;
      
      serializeJson(msg, Serial);
      
      my_data_temp.estatus = false;
      my_data_corrente.estatus = false;
      my_data_ph.estatus = false;
      my_data_ph.estatus = false; 
      
      lastMsg = now;   
  }
  
  if (sp_recv != 0)
  {
      esp_err_t result = esp_now_send(esp_ph_address, (uint8_t *) &sp_recv, sizeof(sp_recv));
      sp_recv = 0;
  }
}
