/* Programa para Calibração do HX711
 Blog Eletrogate - https://blog.eletrogate.com/balanca-digital-com-arduino-aprenda-a-usar-a-celula-de-carga
 Arduino UNO - IDE 1.8.5 - Modulo HX711 - celulas de Carga 50 Kg
 Gustavo Murta   17/abril/2019
 Biblioteca https://github.com/bogde/HX711
 Baseado em https://www.hackster.io/MOHAN_CHANDALURU/hx711-load-cell-amplifier-interface-with-arduino-fa47f3
*/
 
#include "HX711.h"                    // Biblioteca HX711 
 
#define DOUT  A0                      //  HX711 DATA OUT = pino A0 do Arduino 
#define CLK  A1                       //  HX711 SCK IN = pino A1 do Arduino
#define RED  13                       //  Seta a saída do LED vermelho no pino 13
#define GREEN  12                     //  Seta a saída do LED verde no pino 12
 
HX711 balanca;   // define instancia balança HX711
 
float calibration_factor = 47060;   // fator de calibração para teste inicial

// *************** Cartão SD ***************
#include <SD.h>                       // Biblioteca do Cartão SD
#include <SPI.h>                      // Biblioteca da Comunicação SPI
const int chipSelect = 4;             // Seta a saída 4 como CS
// ************** Controle de Estado Operacional ******************
boolean Trigger_On = false ;
#define Valor_ref 20 //Define a carga em que o sistema irá acionar o desligamento automatico por tempo.
float tempo = 0, i = 0;
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
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
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

// *******************  SD Datalogger ****************
   if(Trigger_On == true){
    String dataString = "";                            //Cria uma String para guardar os dados que serão gravados
    dataString = "balanca.get_units() + "     " + millis()";     //Armazena na String os dados de interesse (peso e tempo)

    File dataFile = SD.open("datalog.txt", FILE_WRITE);    // Abre o arquivo que serão gravados os dados
    // IMPORTANTE: o arquivo tem que ser fechado para que os dados sejam salvos
     if(dataFile){                                      // Caso o arquivo esteja disponível, escreva os dados nele
       dataFile.println(dataString);                     // Grava os dados no cartão SD
      Serial.println(dataString);                       // Printa os dados que foram gravados no Monitor Serial
      }
   }
// ****************** Sistema de desligo automatica com o tempo ************
  if((balanca.get_units()) > 20){
    i = 1;
  }
  if( ((balanca.get_units()) < Valor_ref) && (i == 1) && (tempo == 0)){
    tempo = millis();
  }
  if( ((millis()) - tempo) > 5){
    Trigger_On = false;
  }
}


void zeraBalanca ()
{
  Serial.println();   // salta uma linha
  balanca.tare();   // zera a Balança
  Serial.println("Balança Zerada ");
}
