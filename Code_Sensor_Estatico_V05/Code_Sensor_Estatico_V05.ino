 
#include "HX711.h"                 // Biblioteca HX711 
#define DOUT 2                     //  HX711 DATA OUT = pino 2 do Arduino (TEM QUE SER DIGITAL)
#define CLK  3                     //  HX711 SCK IN = pino 3 do Arduino  (TEM QUE SER DIGITAL)
 
HX711 balanca;   // define instancia balança HX711
 
float calibration_factor = 47060;                        // fator de calibração para teste inicial
float sensor_time = 0, sensor_data = 0;
// *************** Cartão SD ***************
#include <SD.h>                                          // Biblioteca do Cartão SD
#include <SPI.h>                                         // Biblioteca da Comunicação SPI
const int chipSelect = 10;                               // Seta a saída 10 como CS
File dataFile;
// ************** Controle de Estado Operacional ******************
#define Delay_Time 10000                                  //Tempo de funcionamento do Modo Online (usado para auto-desligamento).  OBS: é em milisegundos!
#define YELLOW   7                                       //LED amarelo na porta digital 7
#define GREEN   8                                        //LED verde na porta digital 8
#define buttonPin 6                                      //Botão (estilo pulldown) ligado na porta digital 9
#define Valor_ref 20                                     //Define a carga em que o sistema irá acionar o desligamento automatico por tempo.

boolean Trigger_On = false, File_open = false, forca_alta = false, buttonState = false;
float tempo_inicial = 0; //Definindo as flags e o tempo inicial do "Timer" para desligamento automático
//-----------------------------------------------------------------------

void setup()
{
  Serial.begin(9600);                                     // monitor serial 9600 Bps
  balanca.begin(DOUT, CLK);                               // inicializa a balança
  balanca.set_scale();                                    // configura a escala da Balança
  balanca.tare();                                         // zera a Balança

  //************ Inicialização do SD *************
  if(!SD.begin(chipSelect)){
    Serial.println("Card failed, or not present. CHECK WIRING!");    // Verifica se o cartão iniciou
    while(1);                                             // Não permite que o programa continue caso o cartão não eseja presente
  }
  Serial.println("card initialized");
  //************* Configuração dos LEDs **********
  pinMode(YELLOW, OUTPUT);
  pinMode(GREEN, OUTPUT);  
  digitalWrite(YELLOW, HIGH);
  digitalWrite(GREEN, HIGH);
  delay(1000);
  digitalWrite(GREEN, LOW);
  pinMode(9,OUTPUT);                        // Por conta do Layout da PCB foi necessário usar a porta digital 9 como GND,  
  digitalWrite(9, LOW);                     // ou seja, setar ela como LOW (CUIDADO, a corrente máxima é 40mA !!)
  pinMode(buttonPin, INPUT);                // Seta a porta do buttonPin como Input
}
 

void loop()
{
//      -------------------- Gravação dos Dados ----------------------------
// ******************* Trigger ***********************
  buttonState = digitalRead(buttonPin);                     // Le o se o botão esta acionado
  if(buttonState == HIGH)                                 // Se o botão foi acionado, ativa o Trigger
  {                                  
    Trigger_On = true;
    digitalWrite(YELLOW, LOW);
    digitalWrite(GREEN, LOW);
    balanca.tare();                                       // Prepara-se para realizar as medições, tarando o sensor
    delay(30000);                                         // Espera 30 segundos para que as medições e gravações comecem
    
  }
// *******************  SD Datalogger ****************
  String SensorString = "", TimeString = "", SpaceString = "   "; // Definindo as Strings que serão usadas para escrever no SD
  sensor_data = balanca.get_units();
  sensor_data *= 9.81;                                    // Transforma os dados de Kg para Newton (revisar isso)
  SensorString += String(sensor_data);                    // Armazena a leitura da Célula de Carga em uma String que será escrita no SD
  sensor_time = millis();                                 // Marca quando que os dados da dataString foram medidos (usado para fazer um gráfico posteriormente)
  TimeString += String(sensor_time);                      // Armazena esse tempo em uma String que será escrita no SD
  
  if(Trigger_On == true)                                  // Inicia o processo de gravação dos dados se estiver no modo Online
  {                                   
    digitalWrite(YELLOW, LOW);                            // Desliga o LED amarelo (que indica o Modo de Stand-by)
    digitalWrite(GREEN, HIGH);                            // Acende o LED verde (que indica o Modo Online)
    
    File dataFile = SD.open("datalog.txt", FILE_WRITE);   // Abre o arquivo que será gravado os dados
    if (dataFile)                                         // Se o arquivo estiver disponivel, a gravação de dados é iniciada:
    {                                         
      dataFile.print(SensorString);                       // Escreve as medições do Sensor no arquivo
      dataFile.print(SpaceString);                        // O SpaceString é usada para separar os números do SensorString e do TimeString 
      dataFile.println(TimeString);                       // Escreve o tempo que as medições foram feitas e pula uma linha
    }
    dataFile.close();
  }
// ****************** Sistema de desligo automatica com o tempo ************
  if((sensor_data) > 20)                                  //  Se o sensor captar um sinal maior que Newtons, a flag "forca_alta" é acionada,|
  {                                                       //  |sinalizado que tem uma força alta sendo exercida no sensor, ou seja,         |
    forca_alta = true;                                    //  |o teste já está ocorrendo                                                    |
  }
  if( ((sensor_data) < Valor_ref) && (forca_alta == true) && (tempo_inicial == 0) && (Trigger_On == true))    //  |Se a a força exercida for menor que a estipulada, e o arquiva já foi aberto, e a flag forca_alta ja foi acionada,
  {                                                                                                                   //  |isso significa que o teste esta acabando (o tempo == 0 é só para garantir que esse if só irá rodar 1 vez).  
    tempo_inicial = millis();                             // O tempo inicial, que servirá de referência para o Timer com a millis(), para auto-desligamento é iniciada.
  }
  if( ((millis()) - tempo_inicial) > Delay_Time)          // Timer do Sistema de auto-desligamento (Espera 5 segundos para desligar, mudar no define "Delay_Time")
    {                         
      Trigger_On = false;                                 // O trigger do modo de operação e desativa, o sistema volta para o modo Stand-by
      tempo_inicial = 0;                                  // Reseta o tempo_inical para 0, para caso o modo Online seja acionado novamente
      digitalWrite(YELLOW, HIGH);                         // Liga o LED amarelo, sinalizando que o modo Stand-by operando
      digitalWrite(GREEN, LOW);                           // Desliga o LED verde
    }
}
