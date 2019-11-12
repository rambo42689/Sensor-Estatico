 
#include "HX711.h"                 // Biblioteca HX711 
#define DOUT 2                     //  HX711 DATA OUT = pino 2 do Arduino (TEM QUE SER DIGITAL)
#define CLK  3                     //  HX711 SCK IN = pino 3 do Arduino  (TEM QUE SER DIGITAL)
#define vcc_hx 4
 
HX711 balanca;   // define instancia balança HX711
 
float calibration_factor = 47060/2;                        // fator de calibração para teste inicial
float sensor_time = 0, sensor_data = 0;
// *************** Cartão SD ***************
#include <SdFat.h>                                          // Biblioteca do Cartão SD
#include <SPI.h>                                         // Biblioteca da Comunicação SPI
const int chipSelect = 10;                               // Seta a saída 10 como CS
SdFile dataFile;
SdFat sd;
// ************** Controle de Estado Operacional ******************
#define Delay_Time 10000                                  //Tempo de funcionamento do Modo Online (usado para auto-desligamento).  OBS: é em milisegundos!
#define YELLOW   7                                       //LED amarelo na porta digital 7
#define GREEN   8                                        //LED verde na porta digital 8
#define buttonPin 6                                      //Botão (estilo pulldown) ligado na porta digital 9
#define Valor_ref 20                                     //Define a carga em que o sistema irá acionar o desligamento automatico por tempo.

boolean Trigger_On = false, File_open = false, forca_alta = false, buttonState = false;
float tempo_inicial = 0, falha_sd = 1; //Definindo as flags e o tempo inicial do "Timer" para desligamento automático
//-----------------------------------------------------------------------

void setup()
{
  Serial.begin(9600);
  balanca.begin(DOUT, CLK);                               // inicializa a balança
  balanca.set_scale(calibration_factor);                  // configura a escala da Balança
  balanca.tare();                                         // zera a Balança
  pinMode(vcc_hx,OUTPUT);
  digitalWrite(vcc_hx,HIGH);
  pinMode(YELLOW, OUTPUT);
  pinMode(GREEN, OUTPUT);  
  pinMode(9,OUTPUT);                        // Por conta do Layout da PCB foi necessário usar a porta digital 9 como GND,  
  digitalWrite(9, LOW);                     // ou seja, setar ela como LOW (CUIDADO, a corrente máxima é 40mA !!)
  pinMode(6,OUTPUT);
  digitalWrite(6,HIGH);                     // A porta digital 6 age como POWER do HX711
  //************ Inicialização do SD *************
  if (!sd.begin(chipSelect, SPI_HALF_SPEED)){      //Testar futaramente para SPI_FULL_SPEED
    while(falha_sd == 1){
      digitalWrite(YELLOW, LOW);               // Caso o módulo/cartão falhem em iniciar, ou não estejam presente, o LED amarelo começara a piscar
      delay(500);
      digitalWrite(YELLOW, HIGH);
      delay(500);    
      }
    }
  if (!dataFile.open("dataFile.txt", O_RDWR | O_CREAT | O_AT_END)) { //Testa a abertura do arquivo de teste (o arquivo permanece aberto para a gravação de dados)
    while(falha_sd == 1){                     // Caso o arquivo falhe em abrir, o LED verde começará a piscar
      digitalWrite(GREEN, LOW);
      delay(500);
      digitalWrite(GREEN, HIGH);
      delay(500);    
      }
    }
  //************* Configuração dos LEDs **********
  digitalWrite(YELLOW, HIGH);               //Caso todos os sistemas tiverem inicializados com sucesso, os LED piscarão 1 vez, juntos, rapidamente
  digitalWrite(GREEN, HIGH);
  delay(100);
  digitalWrite(YELLOW, LOW);
  digitalWrite(GREEN, LOW);
  delay(100);
  digitalWrite(GREEN, HIGH);
  balanca.tare();
  delay(30000);
  tempo_inicial = millis();
}
 

void loop()
{
//      -------------------- Gravação dos Dados ----------------------------
// *******************  SD Datalogger ****************
  String SensorString = "", TimeString = "", SpaceString = "   "; // Definindo as Strings que serão usadas para escrever no SD
  sensor_data = balanca.get_units();
  sensor_data *= 9.81;                                    // Transforma os dados de Kg para Newton (revisar isso)
  SensorString += String(sensor_data);                    // Armazena a leitura da Célula de Carga em uma String que será escrita no SD
  sensor_time = millis() - tempo_inicial;                 // Marca quando que os dados da dataString foram medidos (usado para fazer um gráfico posteriormente)
  TimeString += String(sensor_time);                      // Armazena esse tempo em uma String que será escrita no SD
                                                          // Inicia o processo de gravação dos dados se estiver no modo Online
                                                                              
      dataFile.print(sensor_data);                       // Escreve as medições do Sensor no arquivo
      dataFile.print(SpaceString);                        // O SpaceString é usada para separar os números do sensor_data e sensor_time 
      dataFile.println(sensor_time);                       // Escreve o tempo que as medições foram feitas, em forma de float, e pula uma linha

    if((millis() - tempo_inicial) >= 60000){
      dataFile.close();
      while(millis()> 0){                                 // Quando a operação terminar, os LED acenderão intercalando, rapidamente
      digitalWrite(GREEN, HIGH);
      digitalWrite(YELLOW, LOW);
      delay(200);
      digitalWrite(YELLOW, HIGH);                         
      digitalWrite(GREEN, LOW);
      delay(200);
      }
    }
}
 
    
