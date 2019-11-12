 
/****************************  Manual de Utilização    *********************************
 * Esse código foi desenvolvido com 4 ceúlas de carga de 50 Kg cada, 1 arduino Pro-mini e 1 módulo
 * HX711. Essa versão, sendo de caráter emergencial, será a mais simplificada possível. O tempo será o fator 
 * utilizado para o inicio da gravação e dos dados.
 *    --- Funcionamento: 
 * - O sistema irá inicilizar, definir as variáveis e inicializar os módulos. 
 * - Caso todos os processos sejam sucedidos, os LED's amarelo e verde piscaram juntos 1 vez,
 *   para em seguida ficar acesso somente o verde. Quando o verde ficar acesso, isso significa que em 30 segundos será iniciada a obtenção dos dados.
 * - O sistema irá rodar por 60 segundos, até que o "Delay_time" atingido. (60000 ms por padrão, é configurável nos #defines).
 * - Quando o processo inteiro for concluído (30 + 60 segundos no total), o LED verde irá começar a piscar DEVAGAR.
 * - Erros sinalizados pelos LED's:
 * Amarelo piscando RAPIDAMENTE -(após inicialização) : falha em reconhecer o módulo do cartão SD, o próprio cartão SD não está inserido corretamente.
 * Amarelo piscando DEVAGAR -(após inicialização): falha em abrir o arquivo no cartão SD, problema no cartão.
 * 
 */

#include "HX711.h"                 // Biblioteca HX711 
#define DOUT 2                     //  HX711 DATA OUT = pino 2 do Arduino (TEM QUE SER DIGITAL)
#define CLK  3                     //  HX711 SCK IN = pino 3 do Arduino  (TEM QUE SER DIGITAL)
#define vcc_hx 4                   //  Esse pino será settado como OUTPUT e usado como VCC para o HX711
 
HX711 balanca;   // define instancia balança HX711
 
float calibration_factor = 47060/2;                      // fator de calibração para teste inicial (divido por 2 por conta de ser 2 pontes de wheatstone)
float sensor_time = 0, sensor_data = 0;
// *************** Cartão SD ***************
#include <SdFat.h>                                       // Biblioteca do Cartão SD
#include <SPI.h>                                         // Biblioteca da Comunicação SPI
const int chipSelect = 10;                               // Seta a saída 10 como CS
SdFile dataFile;
SdFat sd;
// ************** Controle de Estado Operacional ******************
#define Delay_Time 60000                                 //Tempo de funcionamento do Modo Online (usado para auto-desligamento).  OBS: é em milisegundos!
#define YELLOW   7                                       //LED amarelo na porta digital 7
#define GREEN   8                                        //LED verde na porta digital 8

float tempo_inicial = 0, falha_sd = 1; //Definindo as flags e o tempo inicial do "Timer" para desligamento automático
//-----------------------------------------------------------------------

void setup()
{
  pinMode(vcc_hx,OUTPUT);                                 // A porta vcc_hx, por conta do Layout, é usada como power para o HX711
  digitalWrite(vcc_hx,HIGH);
  pinMode(YELLOW, OUTPUT);
  pinMode(GREEN, OUTPUT);  
  pinMode(9,OUTPUT);                                      // Por conta do Layout da PCB foi necessário usar a porta digital 9 como GND,  
  digitalWrite(9, LOW);                                   // ou seja, setar ela como LOW (CUIDADO, a corrente máxima é 40mA !!)
  balanca.begin(DOUT, CLK);                               // inicializa a balança
  balanca.set_scale(calibration_factor);                  // configura a escala da Balança
  balanca.tare();                                         // Zera a balança
  //************ Inicialização do SD *************
  if (!sd.begin(chipSelect, SPI_HALF_SPEED)){      //Testar futaramente para SPI_FULL_SPEED
    while(falha_sd == 1){
      digitalWrite(YELLOW, LOW);               // Caso o módulo/cartão falhem em iniciar, ou não estejam presente, o LED amarelo começara a piscar
      delay(300);
      digitalWrite(YELLOW, HIGH);
      delay(300);    
      }
    }
  if (!dataFile.open("dataFile.txt", O_RDWR | O_CREAT | O_AT_END)) { //Testa a abertura do arquivo de teste (o arquivo permanece aberto para a gravação de dados)
    while(falha_sd == 1){                     // Caso o arquivo falhe em abrir, o LED verde começará a piscar
      digitalWrite(YELLOW, LOW);
      delay(1000);
      digitalWrite(YELLOW, HIGH);
      delay(1000);    
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
  String SpaceString = "   "; // Define a String de "Espaço" que será usada para separa os dados no cartão SD
  sensor_data = balanca.get_units();
  sensor_data *= 9.81;                                    // Transforma os dados de Kg para Newton (revisar isso)
  sensor_time = millis() - tempo_inicial;                 // Marca quando que os dados da dataString foram medidos (usado para fazer um gráfico posteriormente)
                                                          // Inicia o processo de gravação dos dados se estiver no modo Online
                                                                              
      dataFile.print(sensor_data);                       // Escreve as medições do Sensor no arquivo
      dataFile.print(SpaceString);                        // O SpaceString é usada para separar os números do sensor_data e sensor_time 
      dataFile.println(sensor_time);                       // Escreve o tempo que as medições foram feitas, em forma de float, e pula uma linha

    if((millis() - tempo_inicial) >= (Delay_time)){
      dataFile.close();
      while(millis()> 0){                                 // Quando a operação terminar, os LED acenderão intercalando, rapidamente
      digitalWrite(GREEN, HIGH);
      delay(1000);                         
      digitalWrite(GREEN, LOW);
      delay(1000);
      }
    }
}
 
    
