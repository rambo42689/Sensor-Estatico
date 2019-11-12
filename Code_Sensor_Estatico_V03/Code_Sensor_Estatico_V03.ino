/* Programa para Calibração do HX711
 Blog Eletrogate - https://blog.eletrogate.com/balanca-digital-com-arduino-aprenda-a-usar-a-celula-de-carga
 Arduino UNO - IDE 1.8.5 - Modulo HX711 - celulas de Carga 50 Kg
 Gustavo Murta   17/abril/2019
 Biblioteca https://github.com/bogde/HX711
 Baseado em https://www.hackster.io/MOHAN_CHANDALURU/hx711-load-cell-amplifier-interface-with-arduino-fa47f3
*/
 
#include "HX711.h"                    // Biblioteca HX711 
 
#define DOUT 2                      //  HX711 DATA OUT = pino 2 do Arduino (TEM QUE SER DIGITAL)
#define CLK  3                       //  HX711 SCK IN = pino 3 do Arduino  (TEM QUE SER DIGITAL)
 
HX711 balanca;   // define instancia balança HX711
 
float calibration_factor = 47060;   // fator de calibração para teste inicial

// *************** Cartão SD ***************
#include <SD.h>                       // Biblioteca do Cartão SD
#include <SPI.h>                      // Biblioteca da Comunicação SPI
const int chipSelect = 10;             // Seta a saída 4 como CS
File dataFile;
// ************** Controle de Estado Operacional ******************
#define Delay_Time 5000                   //Tempo de funcionamento do Modo Online (usado para auto-desligamento).  OBS: é em milisegundos!
#define YELLOW   7                    //LED amarelo na porta digital 7
#define GREEN   8                     //LED verde na porta digital 8
#define buttonPin 9                   //Botão (estilo pulldown) ligado na porta digital 2
#define Valor_ref 20                  //Define a carga em que o sistema irá acionar o desligamento automatico por tempo.

boolean Trigger_On = false;
float tempo = 0, i = 0,File_open = 0, buttonState = 0, forca_baixa = 0; //Definindo as flags e o "Timer"
//-----------------------------------------------------------------------

void setup()
{
  Serial.begin(9600);   // monitor serial 9600 Bps
  balanca.begin(DOUT, CLK);      // inicializa a balança
  balanca.set_scale();   // configura a escala da Balança
  zeraBalanca ();   // zera a Balança

  //************ Inicialização do SD *************
  if(!SD.begin(chipSelect)){
    Serial.println("Card failed, or not present. CHECK WIRING!");    // Verifica se o cartão iniciou
    while(1);                               // Não permite que o programa continue caso o cartão não eseja presente
  }
  Serial.println("card initialized");
  //************* Configuração dos LEDs **********
  pinMode(YELLOW, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(buttonPin, INPUT);
}
 

void loop()
{
//      -------------------- Gravação dos Dados ----------------------------
// ******************* Trigger ***********************
buttonState = digitalRead(buttonPin);                     //Le o se o botão esta acionado
  if(buttonState == HIGH)                                 //Se o botão foi acionado, ativa o Trigger
  {                                  
    Trigger_On = true;
  }
// *******************  SD Datalogger ****************
  String dataString = "";                                 //Captação de Dados para armazenamento
  float sensor = balanca.get_units();
  sensor = sensor * 9.81;                                 //Transforma os dados de Kg para Newton
  dataString += String(sensor);                           //Armazena a leitura da Célula de Carga, e o tempo que essa leitura foi feita
  timeString = millis();
  
  if(Trigger_On == true)                                  //Inicia o processo de gravação dos dados
  {                                   
    digitalWrite(YELLOW, LOW);
    digitalWrite(GREEN, HIGH);                                    
    File dataFile = SD.open("datalog.txt", FILE_WRITE); // Abre o arquivo que será gravado os dados
   
    if (dataFile)                                        // if the file is available, write to it:
    {                                         
      dataFile.print(dataString);
      dataFile.println(timeString);       
    }
    dataFile.close();
  }
// ****************** Sistema de desligo automatica com o tempo ************
  if((balanca.get_units()) > 20)                        //Se o sensor captar um sinal maior que 20 Newtons, a flag "i" é acionada
  {  
    forca_baixa = 1;
  }
  if( ((balanca.get_units()) < Valor_ref) && (forca_baixa == 1) && (tempo == 0) && (Trigger_On == true))     //Se a a força exercida for menor que a estipulada, e o arquiva já foi aberto, 
  { 
    tempo = millis();                                                           //A contagem para auto-desligamento é iniciada.
  }
  if( ((millis()) - tempo) > Delay_Time)                 //Timer do Sistema de auto-desligamento (Espera 5 segundos para desligar, mudar no define "Delay_Time")
    {                         
      Trigger_On = false;
      tempo = 0;
      digitalWrite(YELLOW, HIGH);
      digitalWrite(GREEN, LOW);
    }
}


void zeraBalanca ()
{
  Serial.println();   // salta uma linha
  balanca.tare();   // zera a Balança
  Serial.println("Balança Zerada ");
}
