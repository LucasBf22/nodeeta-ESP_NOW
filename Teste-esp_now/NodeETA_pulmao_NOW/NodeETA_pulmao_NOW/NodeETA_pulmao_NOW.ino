#include <esp_now.h>
#include <WiFi.h>
#include <FastCRC.h>
#define SSerialTxControl   33   //RS485 Direction control
#define RS485Transmit    HIGH
#define RS485Receive     LOW
long lastMsg = 0;
long lastMsg2 = 0;
int change_poll = 0;

uint8_t broadcastAddress[] = {0x84,0xCC,0xA8,0x7A,0xC4,0x70};
byte msg2[10];
int i=0;
typedef struct struct_message {
  int id; // ID unico para cada esp.
  float ph;
  float sp1;
} struct_message;

union {
   byte b[4];
   float fval;
}u;

//void FloatToHex(float f, byte* hex){
//  byte* f_byte = reinterpret_cast<byte*>(&f);
//  memcpy(hex, f_byte, 4);
//}

struct_message meuDado;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void consulta_ph()
{  
   digitalWrite(SSerialTxControl, RS485Transmit);  //LER PH ENDEREÇO 0550
   Serial2.write((byte)0x03);
   Serial2.write((byte)0x03);
   Serial2.write((byte)0x05);
   Serial2.write((byte)0x50);
   Serial2.write((byte)0x00);
   Serial2.write((byte)0x02);
   Serial2.write((byte)0xC5);
   Serial2.write((byte)0x34);
   delay(30);
   digitalWrite(SSerialTxControl, RS485Receive);
   resposta(false);
} 

//void escrita_sp1_pH()
//{
//  FastCRC16 CRC16;
//   digitalWrite(SSerialTxControl, RS485Transmit);  //ESCREVER SP1 ENDEREÇO 0200
//   uint8_t packet [11] = {0x03, 0x10, 0x02, 0x00, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00};
//   packet[7] = b_sp[3];
//   packet[8] = b_sp[2];
//   unsigned int crcXmodem = CRC16.modbus(packet, 9);
//   packet[10] = highByte(crcXmodem);
//   packet[9] = lowByte(crcXmodem);
//   Serial2.write(packet[0]);
//   Serial2.write(packet[1]);
//   Serial2.write(packet[2]);
//   Serial2.write(packet[3]);
//   Serial2.write(packet[4]);
//   Serial2.write(packet[5]);
//   Serial2.write(packet[6]);
//   Serial2.write(packet[7]);
//   Serial2.write(packet[8]);
//   Serial2.write(packet[9]);
//   Serial2.write(packet[10]);
//   delay(30);
//   digitalWrite(SSerialTxControl, RS485Receive);
//}

//void escrita_sp1_mA()
//{
//   FastCRC16 CRC16;
//   digitalWrite(SSerialTxControl, RS485Transmit);  //ESCREVER SP1 ENDEREÇO 0200
//   uint8_t packet [11] = {0x03, 0x10, 0x04, 0x30, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00};
//   packet[7] = b_sp[3];
//   packet[8] = b_sp[2];
//   unsigned int crcXmodem = CRC16.modbus(packet, 9);
//   packet[9] = highByte(crcXmodem);
//   packet[10] = lowByte(crcXmodem);
//   Serial2.write(packet[0]);
//   Serial2.write(packet[1]);
//   Serial2.write(packet[2]);
//   Serial2.write(packet[3]);
//   Serial2.write(packet[4]);
//   Serial2.write(packet[5]);
//   Serial2.write(packet[6]);
//   Serial2.write(packet[7]);
//   Serial2.write(packet[8]);
//   Serial2.write(packet[9]);
//   Serial2.write(packet[10]);
//   delay(30);
//   digitalWrite(SSerialTxControl, RS485Receive);
//}

void consulta_sp1()
{  
   digitalWrite(SSerialTxControl, RS485Transmit);  //LER SETPOINT1 ENDEREÇO 0200
   Serial2.write((byte)0x03);
   Serial2.write((byte)0x03);
   Serial2.write((byte)0x02);
   Serial2.write((byte)0x00);
   Serial2.write((byte)0x00);
   Serial2.write((byte)0x02);
   Serial2.write((byte)0xC4);
   Serial2.write((byte)0x51);
   //Serial.println("Consulta SP1");
   delay(30);
   digitalWrite(SSerialTxControl, RS485Receive);
   resposta(true);
} 

void resposta(bool ph){
  i=0;
  if(Serial2.available()){
      while (Serial2.available())
      {
          byte msg = Serial2.read();
          msg2[i] = msg;
          i++;
       }
       if(msg2[0] == 0x03 && msg2[1] == 0x03 && ph){ // ID=03 | FC=03 | PUB no pH
        u.b[0] = msg2[6];
        u.b[1] = msg2[5];
        u.b[2] = msg2[4];
        u.b[3] = msg2[3];
        meuDado.ph = u.fval;
        Serial.print("SP1: ");
        Serial.println(u.fval, 4);

       }else if(msg2[0] == 0x03 && msg2[1] == 0x03 && !ph){ // ID=03 | FC=03 | PUB no SP
        u.b[0] = msg2[6];
        u.b[1] = msg2[5];
        u.b[2] = msg2[4];
        u.b[3] = msg2[3];
        meuDado.sp1 = u.fval;
        Serial.print("pH: ");
        Serial.println(u.fval, 4);

       }
    }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial2.begin(9600);
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

  pinMode(SSerialTxControl, OUTPUT);  
  digitalWrite(SSerialTxControl, RS485Receive);  // Init Transceiver

    meuDado.id = 3;

}

void loop() {
  // put your main code here, to run repeatedly:
  long now = millis();

   if (now - lastMsg > 2500) {
    if(change_poll == 0)
      consulta_ph();
    else if(change_poll == 1)
      consulta_sp1();
      lastMsg = now;
      
      change_poll++;
    if(change_poll > 1) change_poll = 0;
  }

  if (now-lastMsg2>5000)
  {
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &meuDado, sizeof(meuDado));
      result == ESP_OK ? Serial.println("Sent with success"): Serial.println("Error sending the data");
      lastMsg2 = now;   
  }
}
