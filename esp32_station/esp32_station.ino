/*--------------------------------------Notas da versão--------------------------------------
 *                             Este projeto é patrocinado pela JLCPCB
 *                                  https://jlcpcb.com/RAT
 *                                  
 *                           Data de modificação: 25/06/2021 04:57
 * 
 * Compilado para Node32S
 * 
 * 
 */

//---------------------Bibliotecas Auxiliares------------------

#include <esp_now.h>
#include <WiFi.h>

#include <Wire.h>

#include <UnicViewAD.h>

//-------------------------------------------------------------


//---------------------Inicialização do LCM--------------------
LCM Lcm(Serial2);  
const long  lcmBaudrate = 115200;     //taxa de comunicação em 115200 baud rate
  LcmVar lcm_bpm       (0);           // Criação de um objeto da classe LcmVar
  LcmVar lcm_spo2      (1);           // Criação de um objeto da classe LcmVar
  LcmVar lcm_mov       (2);           // Criação de um objeto da classe LcmVar
  LcmVar lcm_battery   (3);           // Criação de um objeto da classe LcmVar
  LcmVar lcm_latencia  (4);           // Criação de um objeto da classe LcmVar
 //LcmVar lcm_emergencia(5);           // Criação de um objeto da classe LcmVar

  LcmVar NavigationButtons(5);          // Criação de um objeto da classe "LcmVar"

//---------------------Mapeamento de hardware------------------




//-------------------------------------------------------------

//---------------------Variáveis globais-----------------------

unsigned long time1;
unsigned long time2;

//-------------------------------------------------------------

//-------------------------------------------------------------


// Endereço do transmissor (Substitua o endereço MAC)
uint8_t broadcastAddress[] = {0xA4, 0xCF, 0x12, 0x88, 0xB8, 0x24};

// Variáveis para armazenar valores a serem enviadas
int     BPM;
int     SPO2;
float   MOV;
float   VOLTAGE_BAT;
int     LATENCY;
boolean EMERGENCY;


// Variáveis para armazenar leituras recebidas
int     incomingBPM;
int     incomingSPO2;
float   incomingMOV;
float   incomingVOLTAGE_BAT;
int     incomingLATENCY;
boolean incomingEMERGENCY;


// Variável para armazenar se o envio de dados foi bem sucedido
String success;

// Deve corresponder à estrutura do receptor
typedef struct struct_message {
    int     bpm;
    int     spo2;
    float   mov;
    float   voltage_bat;
    int     latency;
    boolean emergency;

} struct_message;

// Cria uma struct_message chamada rec_data para manter as leituras do sensor
struct_message rec_data;

// Crie uma struct_message para manter as leituras do sensor de entrada
struct_message incomingReadings;

// Retorno de chamada quando os dados são enviados
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nStatus do envio do ultimo pacote:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Entrega do pacote com sucesso" : "Falha na entrega do pacote");

  time2 = millis();
  LATENCY = (time2 - time1);

  if (status ==0){
    success = "Delivery Success :)";
  }
  else{
    success = "Delivery Fail :(";
  }
}

// Retorno de chamada quando os dados são recebidos
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  Serial.print("Bytes recebidos: ");
  Serial.println(len);
  incomingBPM         	    = incomingReadings.bpm;
  incomingSPO2           	= incomingReadings.spo2;
  incomingMOV         		= incomingReadings.mov;
  incomingVOLTAGE_BAT 		= incomingReadings.voltage_bat;
  incomingLATENCY     		= incomingReadings.latency;
  incomingEMERGENCY         = incomingReadings.emergency;


  


}
 
void setup() {

  // Configura a comunicação serial 1 em 115200
  Serial.begin (115200);
  // Configura a comunicação serial 2 em 115200
  Serial2.begin(115200);
  // Configura a comunicação com o display
  Lcm.begin    (115200); 

  


  //Forca a desconexao da wifi
  WiFi.disconnect(); 

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Inicia ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Erro ao iniciar ESP-NOW");
    return;
  }

   // Depois que o ESPNow for iniciado com sucesso, vamos nos registrar para Send CB para
   // obter o status do pacote transmitido
  esp_now_register_send_cb(OnDataSent);
  
  // Registra o par
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Adiciona o par        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Falha ao adicionar o par");
    return;
  }
  // Registra a função de callback para os dados recebidos
  esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {
 
  // Define os valores a serem lidos
  // Os valores passados são os mesmos valores recebidos
  rec_data.bpm         = incomingBPM;
  rec_data.spo2        = incomingSPO2;
  rec_data.mov         = incomingMOV;
  rec_data.voltage_bat = incomingVOLTAGE_BAT;
  rec_data.latency     = incomingLATENCY;
  rec_data.emergency   = EMERGENCY;


  // Envia a mensagem via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &rec_data, sizeof(rec_data));
  time1 = millis();
   
  if (result == ESP_OK) {
    Serial.println("Envio com sucesso");
  }
  else {
    Serial.println("Erro ao enviar os dados :/");
  }


  lcm_bpm.write        (incomingBPM);
  lcm_spo2.write       (incomingSPO2);
  lcm_mov.write        (incomingMOV);
  lcm_battery.write    (incomingVOLTAGE_BAT);  
  lcm_latencia.write   (LATENCY);
 // lcm_emergencia.write (incomingEMERGENCY); 
  if(incomingEMERGENCY == 1){
      Lcm.changePicId(1); // Muda a tela para a tela com o PicID 1
  } 

  if (NavigationButtons.available()) // Verifica se chegou algo no "LcmVar" "NavigationButtons"
  {
     
    EMERGENCY = NavigationButtons.getData();
    Serial.println("EMERGENCY"); 
  }

  
  Serial.print("Bpm: ");        Serial.println(incomingBPM);
  Serial.print("Spo2: ");       Serial.println(incomingSPO2);
  Serial.print("Mov: ");        Serial.println(incomingMOV);
  Serial.print("Battery: ");    Serial.println(incomingVOLTAGE_BAT);
  Serial.print("Latencia: ");   Serial.println(LATENCY);
  Serial.print("Emergencia: "); Serial.println(incomingEMERGENCY);
  Serial.print("Botão: ");      Serial.println(EMERGENCY);



  delay(500);
}
