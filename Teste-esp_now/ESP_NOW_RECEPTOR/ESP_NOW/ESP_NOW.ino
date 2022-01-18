/*
Autor: Lucas Batista
Codigo para esp receptor por arquitetura ESP_NOW
*/

#include <WiFi.h>
#include <esp_now.h>
#include <ArduinoJson.h>

long lastMsg = 0;

int id;
//estrutura do pacote de dados recebidos
typedef struct struct_message_temp {
  int id;
  float temp;
}struct_message_temp;

typedef struct struct_message_nivel {
  int id;
  float nivel;
}struct_message_nivel;

//variavel do dado recebido
struct_message_temp meuDado_temp;
struct_message_nivel meuDado_nivel;


/*--------------------funcao do callback----------------------*/
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {// recebe endereco MAC, dado recebido, tamanho
  char macStr[18];                                                                // vetor de char que vai receber o MAC
//  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",              // Ponteiro do vetor de char, tamanho do vetor, formato desse vetor
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]); //utilizando o padrao recebido pelo mac_addr
//  Serial.println(macStr);                                                         //Printa o MAC do esp que do qual esta recebendo o dado
  memcpy(&id, incomingData, sizeof(id));                                //Copia o numero de bytes do local apontado para a memoria, salvando o pacote de dados na memoria
  switch (id){
    case 1:
      memcpy(&meuDado_temp, incomingData, sizeof(meuDado_temp));                                //Copia o numero de bytes do local apontado para a memoria, salvando o pacote de dados na memoria
//      Serial.printf("temp value: %f \n", meuDado_temp.temp);                     //printa o dado da posicao do array, de acordo com o ID do emissor
//      Serial.println();
      break;
    case 2:
      memcpy(&meuDado_nivel, incomingData, sizeof(meuDado_nivel));                                //Copia o numero de bytes do local apontado para a memoria, salvando o pacote de dados na memoria
//      Serial.printf("nivel value: %f \n", meuDado_nivel);                     //printa o dado da posicao do array, de acordo com o ID do emissor
//      Serial.println();
      break;
    case 3:
      break;
    default:
      Serial.printf("Nenhum ID cadastrado foi encontrado...");
  }
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

  //Chama a funcao callback de acordo com o dado recebido
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
    Serial.print("ESP8266 Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
  StaticJsonDocument<1024> msg;
  long now = millis();
  if (now-lastMsg>10000)
  {
      msg["temp"] = meuDado_temp.temp;
      msg["corrente"] = meuDado_nivel.nivel;
          
      serializeJson(msg, Serial);
      
      meuDado_temp.temp = 0;
      meuDado_nivel.nivel = 0;
      
      
      lastMsg = now;   
  }
}
