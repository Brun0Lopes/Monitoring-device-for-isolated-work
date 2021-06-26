/*--------------------------------------Notas da versão--------------------------------------
 *                             Este projeto é patrocinado pela JLCPCB
 *                                  https://jlcpcb.com/RAT
 *                                  
 *                           Data de modificação: 22/06/2021 22:20
 * 
 * Compilado para Node32S
 * 
 * 
 */

//---------------------Bibliotecas Auxiliares------------------

#include <esp_now.h>
#include <WiFi.h>

#include <Wire.h>

#include "MAX30100_PulseOximeter.h"


//--------------------------------------------------------------------------------------------


//---------------------Mapeamento de hardware------------------

#define motor_vibracao     15
#define led_funcionamento  2

//-------------------------------------------------------------

//---------------------Configuração de PWM---------------------

// Propriedades do PWM
const int freq = 1;
const int ledChannel = 0;
const int resolution = 8;

//-------------------------------------------------------------


//-------------------Variáveis Globais-------------------------

unsigned long lastTime = 0;   
long tsLastReport = 0;

unsigned long lastTime1 = 0;   
long tsLastReport1 = 0;

//-------------------------------------------------------------


//------------------Constantes auxiliares para o---------------
//------------------filtro de média móvel----------------------

#define      n      5        //número de pontos da média móvel
//long moving_average();       //Função para filtro de média móvel
int       original,          //recebe o valor de AN0
          filtrado;          //recebe o valor original filtrado
int       numbers[n];        //vetor com os valores para média móvel



#define REPORTING_PERIOD_MS     1000
PulseOximeter pox;


// Endereço da estação (Substitua pelo endereço MAC de sua placa)
uint8_t broadcastAddress[] = {0x84, 0xCC, 0xA8, 0x2E, 0xF9, 0x5C};

// Variáveis para armazenar valores a serem enviadas
int   BPM;
int   SPO2;
float MOV;
float VOLTAGE_BAT;
int   LATENCY;
boolean EMERGENCY;

// Variáveis para armazenar leituras recebidas
int   incomingBPM;
int   incomingSPO2;
float incomingMOV;
float incomingVOLTAGE_BAT;
int   incomingLATENCY;
boolean incomingEMERGENCY;

// Variável para armazenar se o envio de dados foi bem sucedido
String success;

// Exemplo de estrutura para enviar dados
// Deve corresponder à estrutura do receptor
typedef struct struct_message {
    int     bpm;
    int     spo2;
    float   mov;
    float   voltage_bat;
    int     latency;
    boolean emergency;
} struct_message;

// Crie uma struct_message chamada rec_data para manter as leituras do sensor
struct_message rec_data;

// Crie uma struct_message para manter as leituras do sensor de entrada
struct_message incomingReadings;

// Retorno de chamada quando os dados são enviados
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nStatus do envio do ultimo pacote:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Entrega com Sucesso" : "Falha na entrega");
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
  incomingBPM          = incomingReadings.bpm;
  incomingSPO2         = incomingReadings.spo2;
  incomingMOV          = incomingReadings.mov;
  incomingVOLTAGE_BAT  = incomingReadings.voltage_bat;
  incomingLATENCY      = incomingReadings.latency;
  incomingEMERGENCY    = incomingReadings.emergency;
  

  


}
 
void setup() {
  // Define a freqência do microcontrolador em 80MHz
  // 80MHz é a freqência mínima para poder utilizar o rádio de wifi
  setCpuFrequencyMhz(80);


  // Inicia o monitor serial em 115200
  Serial.begin(115200);

  pinMode(motor_vibracao,    OUTPUT);
  pinMode(led_funcionamento, OUTPUT);

  ledcSetup(ledChannel, freq, resolution);
  
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(motor_vibracao, ledChannel);


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



  //---------------------------------------------------------------------------------

  //Inicia o sensor MAX30100
  pox.begin();

  //Define a corrente para os leds em 46.8mA
  pox.setIRLedCurrent(MAX30100_LED_CURR_46_8MA);


}
 
void loop() {
  getReadings();

    if (millis() - tsLastReport1 > 2000) {
          send_data ();

          
          tsLastReport1 = millis();
     }

}
void getReadings(){

  pox.update();

      
      if (millis() - tsLastReport > 1000) {


          original = pox.getHeartRate();
          filtrado = moving_average();


          BPM  = filtrado;
          SPO2 = pox.getSpO2();

         // Serial.print("Bpm: ");         Serial.println(BPM);
         // Serial.print("Spo2: ");        Serial.println(SPO2);
         // Serial.print("Mov: ");         Serial.println(MOV);
         // Serial.print("Battery: ");     Serial.println(VOLTAGE_BAT);
         // Serial.print("Latencia: ");    Serial.println(LATENCY);
          Serial.print("Emergência: ");  Serial.println(EMERGENCY);
          Serial.print("incomingEMERGENCY ");  Serial.println(incomingEMERGENCY);
          EMERGENCY = incomingEMERGENCY;
          
          tsLastReport = millis();
     }


  MOV         = 503.1;
  VOLTAGE_BAT = 39.3;

  

  if(BPM < 40 | SPO2 < 90 | incomingEMERGENCY == 1){
      ledcWrite(ledChannel, 100);
      EMERGENCY = 1;
  } else {
      ledcWrite(ledChannel, 0);
      EMERGENCY = 0;
  }

  if(incomingEMERGENCY == 1){
      
      EMERGENCY = 0;
  } 

  
}

void send_data () {

  // Define os valores a serem lidos
  rec_data.bpm         = BPM;
  rec_data.spo2        = SPO2;
  rec_data.mov         = MOV;
  rec_data.voltage_bat = VOLTAGE_BAT;
  rec_data.latency     = LATENCY;
  rec_data.emergency   = EMERGENCY;


  // Envia a mensagem via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &rec_data, sizeof(rec_data));
   
  if (result == ESP_OK) {
    Serial.println("Envio com sucesso");
  }
  else {
    Serial.println("Erro ao enviar os dados :/");
  }


}


long moving_average()
{

   //desloca os elementos do vetor de média móvel
   for(int i= n-1; i>0; i--) numbers[i] = numbers[i-1];

   numbers[0] = original; //posição inicial do vetor recebe a leitura original

   long acc = 0;          //acumulador para somar os pontos da média móvel

   for(int i=0; i<n; i++) acc += numbers[i]; //faz a somatória do número de pontos


   return acc/n;  //retorna a média móvel

 
} //end moving_average

