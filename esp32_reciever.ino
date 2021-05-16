
#include <esp_now.h>
#include <WiFi.h>
#include <UnicViewAD.h> 


// --- Inicialização do LCM ---
LCM Lcm(Serial2);  
const long  lcmBaudrate = 115200;                  //taxa de comunicação em 115200 baud rate
  LcmVar lcm_bpm(0);                               // Criação de um objeto da classe LcmVar
  LcmVar lcm_temp(1);                              // Criação de um objeto da classe LcmVar
  LcmVar lcm_spo2(2);                              // Criação de um objeto da classe LcmVar
  LcmVar lcm_battery(3);                           // Criação de um objeto da classe LcmVar
  LcmVar lcm_emergencia(4);                        // Criação de um objeto da classe LcmVar

//----------------------------------------------------------------------------------------------------------------------


//------Estrutura para o recebimento de dados----------------------------------------------
typedef struct struct_message {
  
  int bpm;
  int spo2;
  int vBat;
  float acx;
  float acy;
  float acz;
  int tmp;
  
} struct_message;

//----------------------------------------------------------------------------------------------------------------------

// --- Constantes Auxiliares ---
#define      n      12        //número de pontos da média móvel
#define      n1     60        //número de pontos da média móvel
 


// ===============================================================================
// --- Protótipo da Função ---
long moving_average();       //Função para filtro de média móvel
long moving_average1();       //Função para filtro de média móvel


// ===============================================================================
// --- Variáveis Globais ---
int       original,          //recebe o valor de AN0
          filtrado;          //recebe o valor original filtrado

int       numbers[n];        //vetor com os valores para média móvel

int       original1,          //recebe o valor de AN0
          filtrado1;          //recebe o valor original filtrado

int       numbers1[n1];        //vetor com os valores para média móvel

uint32_t tsLastReport = 0;



// ===============================================================================

//--------Cria uma struct_message chamada myData
struct_message myData;

//----------------------------------------------------------------------------------------------------------------------

//--------Função de callback que será executada quando os dados forem recebidos-----------------------------------------
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));

  original = (myData.bpm);
  filtrado = moving_average();
  original1 = (myData.spo2);
  filtrado1 = moving_average1();
  Serial.print("ACX:");
  Serial.print(myData.acx / 16384.0, 4);
  Serial.print(" ");
  Serial.print("ACY:");
  Serial.print(myData.acy / 16384.0, 4);
  Serial.print(" ");
  Serial.print("ACZ:");
  Serial.print(myData.acz / 16384.0, 4);
  Serial.println(" ");
}




void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);                 //configura comunicação serial
  Serial2.begin(115200);                //configura comunicação serial2
  Lcm.begin(115200);                    //configura a comunicação com o display            
  WiFi.disconnect();                    //desconecta do wifi
  WiFi.mode(WIFI_STA);                  //define o wifi como Station

  pinMode(2, OUTPUT);


  // Inicia o ESP NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
//------------------------------------------------------------------------------------------------------------
  esp_now_register_recv_cb(OnDataRecv); // obtem as informações do nrecebmento dos dados
}
 
void loop() {
  if (millis() - tsLastReport > 2000) {
       lcm_bpm.write(filtrado);
       lcm_temp.write(myData.tmp - 20);           // Escreve no LcmVar Led1Indicator o valor da variável value
       lcm_spo2.write(filtrado1);          // Escreve no LcmVar Led1Indicator o valor da variável value
       lcm_battery.write(myData.vBat);       // Escreve no LcmVar Led1Indicator o valor da variável value
       Lcm.writeTrendCurve0(filtrado);     // Escreve na trend Curve 0 o valor da variável bpm

        tsLastReport = millis();
        
  }
  

}



// ===============================================================================
// --- Desenvolvimento da Função ---
long moving_average()
{

   //desloca os elementos do vetor de média móvel
   for(int i= n-1; i>0; i--) numbers[i] = numbers[i-1];

   numbers[0] = original; //posição inicial do vetor recebe a leitura original

   long acc = 0;          //acumulador para somar os pontos da média móvel

   for(int i=0; i<n; i++) acc += numbers[i]; //faz a somatória do número de pontos


   return acc/n;  //retorna a média móvel

 
} //end moving_average

long moving_average1()
{

   //desloca os elementos do vetor de média móvel
   for(int i= n1-1; i>0; i--) numbers1[i] = numbers1[i-1];

   numbers1[0] = original1; //posição inicial do vetor recebe a leitura original

   long acc = 0;          //acumulador para somar os pontos da média móvel

   for(int i=0; i<n1; i++) acc += numbers1[i]; //faz a somatória do número de pontos


   return acc/n1;  //retorna a média móvel

 
} //end moving_average
