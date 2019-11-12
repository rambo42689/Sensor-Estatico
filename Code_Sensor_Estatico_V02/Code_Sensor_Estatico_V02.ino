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
  Serial.println();   // salta uma linha
  Serial.println("CÉLULA DE CARGA KOSMOS");   // imprime no monitor serial
  Serial.println("HX711 - Calibracao da Balança");   // imprime no monitor serial
  Serial.println("Remova o peso da balanca");
  Serial.println("Depois que as leituras começarem, coloque um peso conhecido sobre a Balança");
  Serial.println("Pressione a,s,d,f para aumentar Fator de Calibração por 10,100,1000,10000 respectivamente");
  Serial.println("Pressione z,x,c,v para diminuir Fator de Calibração por 10,100,1000,10000 respectivamente");
  Serial.println("Após leitura correta do peso, pressione t para TARA(zerar) ");
 
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
/*  --------------  DESCOMENTE CASO DESEJE VER OS DADOS DA BALANÇA
  balanca.set_scale(calibration_factor);   // ajusta fator de calibração
  Serial.print("FORÇA: ");   // imprime no monitor serial
  Serial.print(balanca.get_units()*9.81, 1);   // imprime peso da balança com 3 casas decimais
  Serial.print(" N");
  Serial.print("      Fator de Calibração: ");   // imprime no monitor serial
  Serial.println(calibration_factor);   // imprime fator de calibração
  delay(10) ;   // atraso de 0,1 segundo
*/ //----------------------------- Calibração do Sensor ----------------------------------------
  if (Serial.available())   // reconhece letra para ajuste do fator de calibração
  {
   char temp = Serial.read();
   if (temp == '+' || temp == 'a')   // a = aumenta 10
   calibration_factor += 10;
   else if (temp == '-' || temp == 'z')   // z = diminui 10
   calibration_factor -= 10;
   else if (temp == 's')   // s = aumenta 100
   calibration_factor += 100;
   else if (temp == 'x')   // x = diminui 100
   calibration_factor -= 100;
   else if (temp == 'd')   // d = aumenta 1000
   calibration_factor += 1000;
   else if (temp == 'c')   // c = diminui 1000
   calibration_factor -= 1000;
   else if (temp == 'f')   // f = aumenta 10000
   calibration_factor += 10000;
   else if (temp == 'v')   // v = dimuni 10000
   calibration_factor -= 10000;
   else if (temp == 't') zeraBalanca ();   // t = zera a Balança
  }
//      -------------------- Gravação dos Dados ----------------------------
// ******************* Trigger ***********************
buttonState = digitalRead(buttonPin);                     //Le o se o botão esta acionado
  if(buttonState == HIGH)                                 //Se o botão foi acionado, ativa o Trigger
  {                                  
    Trigger_On = true;
  }
// *******************  SD Datalogger ****************
  String dataString = "";                                 //Captação de Dados para armazenamento
  int sensor = balanca.get_units();
    dataString += String(sensor + "  " + millis());              //Armazena a leitura da Célula de Carga, e o tempo que essa leitura foi feita
  
  if(Trigger_On == true)                                  //Inicia o processo de gravação dos dados
  {                                   
    digitalWrite(YELLOW, LOW);
    digitalWrite(GREEN, HIGH);
    if( File_open == 0)                                      //Grava o tempo inicial (usado para o desligamento automatico por tempo)
    {                                          
      File dataFile = SD.open("datalog.txt", FILE_WRITE); // Abre o arquivo que será gravado os dados. Está dentro deste "if" para garantir que irá abrir somente 1 vez (no inicio do trigger)
      File_open += 1;
    }
    if (dataFile)                                        // if the file is available, write to it:
    {                                         
      dataFile.println(dataString);
    }
  }
// ****************** Sistema de desligo automatica com o tempo ************
  if((balanca.get_units()) > 20)                        //Se o sensor captar um sinal maior que 20 Newtons, a flag "i" é acionada
  {  
    forca_baixa = 1;
  }
  if( ((balanca.get_units()) < Valor_ref) && (forca_baixa == 1) && (File_open == 1) && (tempo == 0))     //Se a a força exercida for menor que a estipulada, e o arquiva já foi aberto, 
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
