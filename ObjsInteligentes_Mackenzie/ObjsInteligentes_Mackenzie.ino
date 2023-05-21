#include <ESP8266WiFi.h>  //Biblioteca para conexão WiFi
#include <PubSubClient.h>
//_____Definição de LOGINS e SENHAS___________________________
//WiFi
const char* ssid = "xxxxxxxxx";    // Rede WiFi a ser conectada
const char* senha = "xxxxxxxxxx";  //senha da rede WiFi a ser conectada
//MQTT
const char* mqttServidor = "broker.shiftr.io";  //Endereço do servidor a ser conectado
const char* mqttUsuario = "XXXXXXXX";           //Nome do usuário que será conectado no servidor
const char* mqttSenha = "XXXXXXXX";             //senha do dispositivo MQTT
const int mqttPorta = 1883;                     //número da porta que será conectada no servidor, a padrão é 1883
const char* mqttTopicoSub1 = "porta/sensor";    //endereço dos pacotes enviados/recebidos, podem haver mais
const char* mqttTopicoSub2 = "porta/atuador";
WiFiClient Mackenzie;            //Cria o nome do cliente que usará o WiFI
PubSubClient client(Mackenzie);  //Cria uma instância "client” para ser utilizada pelo MQTT
//____________________________________________________________
//_____Chamada de variáveis Globais___________________________
#define botao 1
#define rele 2
//____________________________________________________________
//_____Criação de Funções Auxiliares_____________________________________
//Função que efetua a conexão com o WiFi
void conectar_WiFi() {
  WiFi.mode(WIFI_STA);      //Definindo o modo de conexão como Station Mode
  WiFi.begin(ssid, senha);  //Chamada da conexão WiFi informando os parâmetros
  Serial.println("");       //imprimindo um espaço em branco na linha de baixo do debug
  //Comando para ficar tentando conectar na rede
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);                            // cria uma pausa no programa para a comunicação
    Serial.println("Conectando no WiFi");  //O Serial.print e o Serial println servem para escrever
    mensagem no debug
  }
  //depois de conectado
  Serial.print("Conectado na rede: ");
  Serial.println(ssid);
  Serial.print("Com o endereço de IP: ");
  Serial.println(WiFi.localIP());  // a função WiFi.localIP() estabelece o ip local do dispositivo
}
//Função que efetua a conexão com o Servidor MQTT
void conectar_MQTT() {
  49 client.setServer(mqttServidor, mqttPorta);  //Inicia a conexão com servidor enviando os parâmetros
  client.setCallback(recebe_Valores);            //Define o que acontece quando o client recebe um pacote de
  informações
    Serial.println(" ");
  //Comando para ficar tentando conectar ao Servidor MQTT
  while (!client.connected()) {
    Serial.println("Conectando ao Servidor MQTT");
    if (client.connect("MQTT_Extender_1ynqoBbMinTuB6m", mqttUsuario, mqttSenha))  //após conectar ao servidor, é necessário
    {
      Serial.println("Conexão com servidor MQTT confirmada!");
    } else {
      Serial.print("Falha na conexão com o servidor MQTT, estado: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
  //depois de conectado
  //subescreve no tópico
  client.subscribe(mqttTopicoSub1);
  client.subscribe(mqttTopicoSub2);
}
//Função para manter conectado ao Servidor
void reconectar_MQTT() {
  if (!client.connected()) {
    Serial.println("Conexão com o Servidor MQTT perdida, reconectando");
    conectar_MQTT();
  }
}
// Função para manter conectado ao WiFi
void reconectar_WiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    conectar_WiFi();
  }
}
//Função para enviar infomações ao servidor MQTT
void envia_Valores() {
  static bool estadoBotao = HIGH;
  static bool estadoBotaoAnt = HIGH;
  static unsigned long debounceBotao;
  estadoBotao = digitalRead(botao);
  50 if ((millis() - debounceBotao) > 30) {  //Elimina efeito Bouncing
    //OBS: millis() é o comando que calcula os
    //milissegundos que passaram desde que o programa iniciou
    if (!estadoBotao && estadoBotaoAnt) {
      //Botao Apertado
      client.publish(mqttTopicoSub1, "TRANCADA");
      Serial.println("Botao APERTADO. Payload enviado.");
      debounceBotao = millis();
    } else if (estadoBotao && !estadoBotaoAnt) {
      //Botao Solto
      client.publish(mqttTopicoSub1, "DESTRANCADA");
      Serial.println("Botao1 SOLTO. Payload enviado.");
      debounceBotao = millis();
    }
  }
  estadoBotaoAnt = estadoBotao;
}
//Função Para receber Informações do servidor MQTT
void recebe_Valores(char* topic, byte* payload, unsigned int length) {
  static unsigned long espera;  //variavel que servira para rearmar fechadura
  String msg;
  //obtem a string do payload (pacote) recebido
  for (int i = 0; i < length; i++) {
    char c = (char)payload[i];
    msg += c;
  }
  //Escreve a mensagem recebida pelo servidor no debug
  Serial.print("Mensagem chegou do tópico: ");
  Serial.println(topic);
  Serial.print("Mensagem: ");
  Serial.println(msg);
  Serial.println();
  Serial.println("__________________");
  if (msg == "0") {
    digitalWrite(rele, HIGH);  //se a mensagem recebida for “0” mantem relé desativado
  }
  if (msg == "1") {
    digitalWrite(rele, LOW);  //se a mensagem recebida for “1” ativa relé
    delay(2000);
    client.publish(mqttTopicoSub2, "0");
    51
  }
}
//____________________________________________________________
void setup() {
  Serial.begin(115200);  //Inicia a comunicação serial com a velocidade
  //de 115200 para comunicação serial
  // que serve para acompanhar o funcionamento do programa
  delay(10);
  pinMode(botao, INPUT_PULLUP);
  pinMode(rele, OUTPUT);
  conectar_WiFi();  //chama a função conectar_WiFi
  //o mesmo ocorrerá com as outras funções auxiliares criadas
  conectar_MQTT();
}
void loop() {
  reconectar_MQTT();
  reconectar_WiFi();
  envia_Valores();
  client.loop();  //comando para manter o sincronismo
  // entre o dispositivo e o servidor
  //fundamental para o funcionamento correto
  // das funções envia_Valores e recebe_Valores
}
