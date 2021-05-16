/*--------------------------------------Notas da versão--------------------------------------
 * 
 *                           Data de modificação: 14/05/2021 21:44
 * 
 * Troca do sensor MAX30100 para o MAX30102
 * 
 * 
 */


//---------------------Bibliotecas Auxiliares-------------------------------------------------
#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

//--------------------------------------------------------------------------------------------

//---------------------Mapeamento de Hardware-------------------------------------------------
#define led_funcionamento    2
#define led_falha_de_envio   15
#define bt_power             12
#define bt_emergencia        13
#define bt_3                 14

//--------------------------------------------------------------------------------------------

//---------------------Diretivas Gerais-------------------------------------------------------
MAX30105 particleSensor;

uint8_t broadcastAddress[] = {0x24, 0x0A, 0xC4, 0xC6, 0x36, 0xA0};            //Endereco da estacao
//Estrutura de dados
typedef struct struct_message {
  
  int bpm;
  int spo2;
  int vBat;
  float acx;
  float acy;
  float acz;
  int tmp;
    
} struct_message;
struct_message myData;

//--------------------------------------------------------------------------------------------

//---------------------Variaveis Globais------------------------------------------------------
unsigned long lastTime = 0;  
unsigned long timerDelay = 500;  

boolean bt_power_pressed     = 0;
boolean bt_emergency_pressed = 0;

const byte RATE_SIZE = 4;                                                   //Opções: 1, 2, 4, 8, 16, 32. Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE];                                                      //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0;                                                          //Time at which the last beat occurred

float beatsPerMinute;
int beatAvg;

//--------------------------------------------------------------------------------------------

//---------------------Prototipo das Funcoes--------------------------------------------------

// Funcao de Callback para o envio da estrutura de dados
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
    
}


  //Tarefa para indicar o funcionamento-------------
void func_status_led (void * parameter){
  for(;;){ // loop infinito
    digitalWrite(led_funcionamento, HIGH);
    vTaskDelay(50 / portTICK_PERIOD_MS);
    digitalWrite(led_funcionamento, LOW);
    vTaskDelay(1500 / portTICK_PERIOD_MS);
  }
}

 //------------------------------------------------
 //Leitura dos botoes touch------------------------
void button_read (void * parameter){
  int bt_power_val      = 0;          //Variavel para armazenar o valor lido pelo botao touch
  int bt_emergencia_val = 0;          //Variavel para armazenar o valor lido pelo botao touch

  
  for(;;){ // loop infinito
    bt_power_val      = touchRead(bt_power);
    bt_emergencia_val = touchRead(bt_emergencia);
        
    if (bt_power_val < 30) {
//      bt_power_pressed = 1;
        digitalWrite(led_falha_de_envio, HIGH);
    }

    if (bt_emergencia_val < 30) {
//      bt_emergency_pressed = 1;
        digitalWrite(led_falha_de_envio, LOW);
    }
  }
} 


/*void sensor_read (void * parameter){
   for (;;) {  // loop infinito
      long irValue = particleSensor.getIR();

    if (checkForBeat(irValue) == true)
    {
      //We sensed a beat!
      long delta = millis() - lastBeat;
      lastBeat = millis();

        beatsPerMinute = 60 / (delta / 1000.0);

        if (beatsPerMinute < 255 && beatsPerMinute > 20)
         {
          rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
          rateSpot %= RATE_SIZE; //Wrap variable

          //Take average of readings
          beatAvg = 0;
          for (byte x = 0 ; x < RATE_SIZE ; x++)
          beatAvg += rates[x];
          beatAvg /= RATE_SIZE;
     }
   }

    Serial.print("IR=");
    Serial.print(irValue);
    Serial.print(", BPM=");
    Serial.print(beatsPerMinute);
    Serial.print(", Avg BPM=");
    Serial.println(beatAvg);
    vTaskDelay(10 / portTICK_PERIOD_MS);

       
  }
}

void data_send (void * parameter) {
  if ((millis() - lastTime) > timerDelay) {
    // Seleciona os valores para enviar
     myData.bpm   = pox.getHeartRate();
     myData.spo2  = pox.getSpO2();
     myData.vBat  = 82;
     myData.acx   = random(0,80);
     myData.acy   = random(0,80);
     myData.acz   = random(0,80);
     myData.tmp   = 32;

     // Envia a mensagem via ESP-NOW
     esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

     if (result == ESP_OK) {
        Serial.println("Sent with success");
     }
     else {
        Serial.println("Error sending the data");
     }

      lastTime = millis();
     }
}
*/
 //------------------------------------------------

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

void setup() {

  pinMode(led_funcionamento,  OUTPUT);
  pinMode(led_falha_de_envio, OUTPUT);

  //Registrando as Tarefas-------------------------
    xTaskCreate(
    func_status_led,                    // Função a ser chamada
    "Status de funcionamento",          // Nome da tarefa
    1000,                               // Tamanho (bytes)
    NULL,                               // Parametro a ser passado
    1,                                  // Prioridade da Tarefa
    NULL                                // Task handle
  );

  xTaskCreate(  
    button_read,                        // Função a ser chamada
    "Leitura dos botoes",               // Nome da tarefa
    1000,                               // Tamanho (bytes)
    NULL,                               // Parametro a ser passado
    1,                                  // Prioridade da Tarefa
    NULL                                // Task handle
  );
/*  
  xTaskCreate(  
    sensor_read,                        // Função a ser chamada
    "Leitura dos sensores",             // Nome da tarefa
    1000,                               // Tamanho (bytes)
    NULL,                               // Parametro a ser passado
    2,                                  // Prioridade da Tarefa
    NULL                                // Task handle
  );

  xTaskCreate(  
    data_send,                          // Função a ser chamada
    "Envio dos dados",                  // Nome da tarefa
    1000,                               // Tamanho (bytes)
    NULL,                               // Parametro a ser passado
    3,                                  // Prioridade da Tarefa
    NULL                                // Task handle
  );
 */
 //------------------------------------------------


  
  setCpuFrequencyMhz(80);                            //Define o clock do processador em 80MHz
  

}
 
void loop() {


}


void initial_config (){
  Serial.begin(115200);                              //Inicializa a comunicacao serial em 115200
  WiFi.disconnect();                                 //Forca a desconexao da wifi
  WiFi.mode(WIFI_STA);                               //Coloca o radio WIFI em modo STA

  //Inicia o ESP-NOW------------------------------
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
 //------------------------------------------------
  esp_now_register_send_cb(OnDataSent);               //Registra a funcao de callback para o envio dos dados
  
  //Registra os pares da conexao
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
 //------------------------------------------------
  
  // Adiciona o par        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
 //------------------------------------------------

 
  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println(F("MAX30105 was not found. Please check wiring/power."));
    while (1);
  }

  byte ledBrightness = 60;        //Options: 0=Off to 255=50mA
  byte sampleAverage = RATE_SIZE; //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 2;               //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  byte sampleRate = 800;          //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 215;           //Options: 69, 118, 215, 411
  int adcRange = 4096;            //Options: 2048, 4096, 8192, 16384

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings
}
