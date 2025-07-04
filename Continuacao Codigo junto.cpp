#include <Arduino.h>
#include <Bounce2.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <PubSubClient.h>
#include <internet.h>
#include <WiFi.h>
#include <ArduinoJson.h>

// Pinos conectados ao RC522
#define SS_PIN 5               // SDA do RC522
#define RST_PIN 22             // RST do RC522
MFRC522 rfid(SS_PIN, RST_PIN); // cria o objeto do leitor
WiFiClient espClient;
PubSubClient client(espClient);

const char *mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char *mqtt_id = "ProjetoCoup";
const char *mqtt_topic_sub = "senai134/Dealer/esp_inscrito";
const char *mqtt_topic_pub = "senai134/Jogador/esp_publicando";

void limitMoedas();
void amountMoedas();
void drawPhase();
void callback(char *, byte *, unsigned int);
void mqttConnect(void);
// TODO : Adicionar a parte do Curupira, a funcao de eliminar cartas(nao sei se e no esp do diller ou do jogador) e adicionar o MQTT

struct Values
{
  int Moedas = 6;           // Variável para armazenar a quantidade de fragmentos do Elo
  const int boto = 3;       // Valor da carta Boto
  const int saci = -3;      // Valor da carta Saci
  const int curupira = -2;  // Valor da carta Curupira
  const int maxMoedas = 12; // Valor máximo de fragmentos que o jogador pode ter
};
Values value; // Cria uma instância da estrutura Values

struct CardsState
{
  bool Boto = false;     // Variável para armazenar o estado da carta boto cor de rosa
  bool Saci = false;     // Variável para armazenar o estado da carta saci
  bool Curupira = false; // Variável para armazenar o estado da carta curupira
};
CardsState cards; // Cria uma instância da estrutura cards

struct playerState
{
  bool draw = false; // Variável para armazenar o estado da compra
  // bool endturn = false; // Variável para armazenar o estado do turno (provavelmente pro diller)
  bool ritual = false; // Variável para armazenar o estado da carta golpe de estado
};
playerState player; // Cria uma instância da estrutura player

// === Protótipos ===
void mostrarInstrucoesMenu();
void exibirItemMenu();
void atualizarBotoes();
void atualizarJoystick();
void verificarLDR();
void sortearCartas();
const char *LendaNome(int codigo);

// === Pinos ===
const int pinoLDR = 33;
const int botaoSelecionar = 14;
const int botaoVoltar = 12;
const int botaoC = 27;
const int botaoD = 13;

const int joystickXPin = 34;
const int joystickYPin = 35;

const int led1 = 25;
const int led2 = 18;
const int led3 = 26;

// === LCD ===
LiquidCrystal_I2C lcd(0x27, 16, 4);

// === Botões com debounce ===
Bounce btnSelecionar = Bounce();
Bounce btnVoltar = Bounce();
Bounce btnC = Bounce();
Bounce btnD = Bounce();

// === Menu ===
int indiceMenu = 0;
const int totalAcoes = 6;
bool emMenu = false;

// === Controle do joystick ===
const int deadzone = 400;
unsigned long ultimaLeituraJoystick = 0;
const unsigned long debounceJoystick = 250;
int UltimaDirecaoX = 0;
int UltimaDirecaoY = 0;

// === Estado do jogo ===
int moedas = 2;

// === Tempo ===
const unsigned long intervaloMoeda = 1000;
unsigned long ultimaAtualizacaoMoeda = 0;

// === Icones Personalizados ===
byte moedaChar[8] = {
    0B00100,
    0B01110,
    0B10101,
    0B01100,
    0B00111,
    0B10101,
    0B01110,
    0B00100,
};

byte aChar[8] = {
    0B01101,
    0B10010,
    0B00000,
    0B01111,
    0B00001,
    0B11111,
    0B10001,
    0B01111,
};

byte oChar[8] = {
    0B01101,
    0B10010,
    0B00000,
    0B01110,
    0B10001,
    0B10001,
    0B10001,
    0B01110,
};

byte cChar[8] = {
    0B00000,
    0B01110,
    0B10000,
    0B10000,
    0B10000,
    0B01110,
    0B00100,
    0B01100,
};

byte uChar[8] = {
    0B00010,
    0B10101,
    0B10001,
    0B10001,
    0B10001,
    0B10001,
    0B10001,
    0B01110,
};

// === Cartas sorteadas ===
int lenda1 = -1;
int lenda2 = -1;
bool cartasSorteadas = false;

// === Funções auxiliares ===

const char *obterDescricaoAcao(int acao)
{
  switch (acao)
  {
  case 1:
    return "PEGUE 3 MOEDAS";
  case 2:
    return "ROUBA 2 MOEDAS ";
  case 3:
    return "BLOQUEIO   ";
  case 4:
    return "ELIMINAR CARTA ";
  case 5:
    return "TROCAR CARTAS  ";
  case 6:
    return "OLHAR CARTA    ";
  default:
    return "ERRO           ";
  }
}

const char *LendaNome(int codigo)
{
  switch (codigo)
  {
  case 2:
    return "CURUPIRA";
  case 3:
    return "IARA    ";
  case 4:
    return "SACI    ";
  default:
    return "N/A     ";
  }
}

// === Setup ===
void setup()
{
  Serial.begin(9600);

  SPI.begin();     // Inicia SPI com GPIOs padrão do ESP32
  rfid.PCD_Init(); // Inicia o RC522
  delay(1000);
  if (rfid.PCD_PerformSelfTest()) {
  Serial.println("RC522 OK");
} else {
  Serial.println("Falha no RC522!");
}
  conectaWiFi();                            // Conecta ao WiFi
  client.setServer(mqtt_server, mqtt_port); // Define o servidor MQTT e a porta
  client.setCallback(callback);             // Define a função de callback para receber mensagens MQTT

  randomSeed(analogRead(36));

  pinMode(botaoSelecionar, INPUT_PULLUP);
  pinMode(botaoVoltar, INPUT_PULLUP);
  pinMode(botaoC, INPUT_PULLUP);
  pinMode(botaoD, INPUT_PULLUP);

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(pinoLDR, INPUT);

  btnSelecionar.attach(botaoSelecionar);
  btnSelecionar.interval(25);
  btnVoltar.attach(botaoVoltar);
  btnVoltar.interval(25);
  btnC.attach(botaoC);
  btnC.interval(25);
  btnD.attach(botaoD);
  btnD.interval(25);

  lcd.init();
  lcd.backlight();

  lcd.createChar(0, moedaChar);
  lcd.createChar(1, aChar);
  lcd.createChar(2, oChar);
  lcd.createChar(3, cChar);
  lcd.createChar(4, uChar);

  lcd.clear();
  lcd.setCursor(4, 1);
  lcd.write(byte(4));
  lcd.setCursor(5, 1);
  lcd.print("LTIMO ELO");
  lcd.setCursor(4, 2);
  lcd.print("ENCANTADO");
  delay(2000);

  mostrarInstrucoesMenu();
}

// === Mostrar instruções ===
void mostrarInstrucoesMenu()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Use Joystick UP/DN");
  lcd.setCursor(0, 1);
  lcd.print("A=selecionar B=sair");
  delay(1500);
}

// === Sortear cartas e mostrar na tela ===
void sortearCartas()
{
  lcd.clear();
  lcd.setCursor(3, 1);
  lcd.print("SORTEANDO CARTAS");
  delay(1500);

  do
  {
    lenda1 = random(2, 5); // 2,3,4
    lenda2 = random(2, 5);
  } while (lenda2 == lenda1);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CARTA 1: ");
  lcd.print(LendaNome(lenda1));

  lcd.setCursor(0, 1);
  lcd.print("CARTA 2: ");
  lcd.print(LendaNome(lenda2));

  lcd.setCursor(0, 3);
  lcd.print("Pressione A...");
  cartasSorteadas = true;
}

// === Exibe o menu de ações com cartas e moedas ===
void exibirItemMenu()
{
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("A");
  lcd.setCursor(1, 0);
  lcd.write(byte(3));
  lcd.setCursor(2, 0);
  lcd.write(byte(2));
  lcd.setCursor(3, 0);
  lcd.print("es: ");

  lcd.setCursor(6, 0);
  lcd.print(obterDescricaoAcao(indiceMenu + 1));
  lcd.setCursor(0, 1);
  lcd.print("C1: ");
  lcd.print(LendaNome(lenda1));

  lcd.setCursor(0, 2);
  lcd.print("C2: ");
  lcd.print(LendaNome(lenda2));
  lcd.setCursor(0, 3);
  lcd.print("Moedas:");
  lcd.print(moedas);
  lcd.write(byte(0));
}

// === Loop principal ===
void loop()
{
  if (btnD.changed()) // esconde a carta 1 quando pressionado
  {
    lcd.setCursor(0, 1);
    lcd.print("C1:      ");
    lcd.setCursor(0, 3);
    lcd.print("Moedas:");
    lcd.print(moedas);
    lcd.write(byte(0));
  }

  if (btnD.changed()) // esconde a carta 1 quando pressionado
  {
    lcd.setCursor(0, 1);
    lcd.print("C1:        ");
    lcd.print(LendaNome(lenda1));
    lcd.setCursor(0, 3);
    lcd.print("Moedas:");
    lcd.print(moedas);
    lcd.write(byte(0));
  }

  if (btnC.changed()) // esconde a carta 2 quando pressionado
  {
    lcd.setCursor(0, 2);
    lcd.print("C2:         ");
    lcd.setCursor(0, 3);
    lcd.print("Moedas:");
    lcd.print(moedas);
    lcd.write(byte(0));
  }
  if (btnC.changed()) // mostra a carta 2 quando pressionado
  {
    lcd.setCursor(0, 2);
    lcd.print("C2:        ");
    lcd.print(LendaNome(lenda2));
    lcd.setCursor(0, 3);
    lcd.print("Moedas:");
    lcd.print(moedas);
    lcd.write(byte(0));
  }

  if (!cartasSorteadas)
  {
    sortearCartas();
    return;
  }

  atualizarBotoes();
  atualizarJoystick();
  verificarLDR();

  // Verifica se um novo cartão está presente
  if (rfid.PICC_IsNewCardPresent())
  return;
  {
    // Lê o cartão
    if (rfid.PICC_ReadCardSerial())
    return;
    {
      // Constrói a string do UID
      String uid = "";
      for (byte i = 0; i < rfid.uid.size; i++)
      {
        if (rfid.uid.uidByte[i] < 0x10) uid += "0"; // Adiciona zero à esquerda se necessário
        uid += String(rfid.uid.uidByte[i], HEX); // Converte o byte para hexadecimal
      }
      uid.toUpperCase(); // Converte para maiúsculas

      // Imprime o UID no Serial
      Serial.print("UID lido: ");
      Serial.println(uid); 

      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
    }
  }

  checkWiFi();   // Verifica a conexão WiFi
  mqttConnect(); // Conecta ao MQTT se não estiver conectado
  client.loop(); // Mantém o loop do cliente MQTT ativo

  //* Parte do Boto
  if (cards.Boto)
  {
    if (value.Moedas < value.maxMoedas) // Se a carta for Boto e moedas forem menores que 10
    {
      Serial.println("Carta Boto ativada! Você ganhou 3 moedas");
      value.Moedas += value.boto; // Adiciona o valor da carta Boto
      delay(50);
      amountMoedas(); // Chama a função para mostrar a quantidade de fragmentos
    }
    if (value.Moedas > value.maxMoedas) // Garante que não ultrapasse o máximo
    {
      limitMoedas();
    }

    cards.Boto = false; // Reseta o estado da carta Boto
  }
  //* Parte do Saci
  if (cards.Saci)
  {
    if (value.Moedas >= 3)
    {
      value.Moedas += value.saci; // Subtrai o valor da carta Saci

      // todo : Adicionar funcao para eliminar uma carta do jogo

      Serial.println("Carta Saci ativada! Você gastou 3 fragmentos para eliminar uma carta.");
      Serial.print("Quantidade de fragmentos: ");
      Serial.println(value.Moedas);
    }
    else
    {
      Serial.println("Você não tem fragmentos suficientes para usar a carta Saci.");
      Serial.print("Quantidade de fragmentos: ");
      Serial.println(value.Moedas);
    }
    cards.Saci = false; // Reseta o estado da carta Saci
  }

  //* Parte da Draw(Compra de cartas)

  if (player.draw)
  {
    Serial.println("Você pode comprar uma carta. Pressione o botão para comprar.");
    drawPhase(); // Chama a função de compra de cartas
  }
  else if(rfid.PICC_IsNewCardPresent())
  {
   
      Serial.println("Você já comprou um fragmento.");
      Serial.print("Quantidade de fragmentos: ");
      Serial.println(value.Moedas);
    
  }

  //* Parte do Ritual do Esquecimento (Golpe de Estado)

  if (value.Moedas >= 7 && player.ritual == true) // Se tiver pelo menos 7 fragmentos
  {
    // colocar opcao de iniciar o ritual no (provavelmente um botao) LCD
    value.Moedas -= 7;     // Gasta 7 fragmentos
    player.ritual = false; // Reseta o estado do ritual
    Serial.println("Ritual do Esquecimento ativado! Você eliminou uma carta do jogo.");
    Serial.print("Quantidade de fragmentos restantes: ");
    Serial.println(value.Moedas);

    // Mandar mensagem para o dealer resetando as cartas do jogador afetado pelo ritual
  }

  //* Parte do Curupira

  if (cards.Curupira)
  {
    if (value.Moedas >= 2) // Se a carta for Curupira e moedas forem maiores que 2
    {
      value.Moedas += value.curupira; // Subtrai o valor da carta Curupira
      Serial.println("Carta Curupira ativada! Você perdeu 2 fragmentos.");
      amountMoedas(); // Chama a função para mostrar a quantidade de fragmentos
    }
    else
    {
      Serial.println("Você não tem fragmentos suficientes para usar a carta Curupira.");
      amountMoedas(); // Chama a função para mostrar a quantidade de fragmentos
    }
    cards.Curupira = false; // Reseta o estado da carta Curupira
  }

  delay(2000);
}

void atualizarBotoes()
{
  btnSelecionar.update();
  btnVoltar.update();
  btnC.update();
  btnD.update();

  if (btnSelecionar.fell())
  {
    if (!emMenu && cartasSorteadas && lenda1 != -1 && lenda2 != -1)
    {
      emMenu = true;
      indiceMenu = 0;
      exibirItemMenu();
      return;
    }

    if (emMenu)
    {
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("ACAO SELECIONADA:");
      lcd.setCursor(0, 2);
      lcd.print(obterDescricaoAcao(indiceMenu + 1));
      delay(3000);
      exibirItemMenu();
    }
  }

  if (btnVoltar.fell() && emMenu)
  {
    lcd.clear();
    lcd.print("Saindo do menu.");
    delay(1500);
    lcd.clear();
    lcd.print("Pressione A p/menu");
    emMenu = false;
  }
}

// === Atualizar joystick para navegar menu ===
void atualizarJoystick()
{
  if (!emMenu)
  {
    return;
  }

  int y = analogRead(joystickYPin) - 4000;
  int x = analogRead(joystickXPin) - 4000;
  int dirX = 0;
  int dirY = 0;

  if (abs(y) > deadzone)
    dirY = (y > 0) ? 1 : -1;
  if (abs(x) > deadzone)
    dirX = (x > 0) ? 1 : -1;

  unsigned long agora = millis();
  if (agora - ultimaLeituraJoystick < debounceJoystick)
    return;

  if (dirY != 0 && dirY != UltimaDirecaoY)
  {
    if (dirY == 1)
    {
      indiceMenu = (indiceMenu - 1 + totalAcoes);
      exibirItemMenu();
    }
    else if (dirY == -1)
    {
      indiceMenu = (indiceMenu + 1) % totalAcoes;
      exibirItemMenu();
    }
    ultimaLeituraJoystick = agora;
    UltimaDirecaoY = dirY;
  }
  else if (dirY == 0)
  {
    // Libera o joystick: permite nova navegação depois
    UltimaDirecaoY = 0;
  }

  if (dirX != UltimaDirecaoX)
  {
    if (dirX == 1)
      Serial.println("Joystick para direita");
    else if (dirX == -1)
      Serial.println("Joystick para esquerda");
    UltimaDirecaoX = dirX;
  }
  }


// === Verificar sensor LDR para diminuir moedas ===
void verificarLDR()
{
  int leituraLDR = analogRead(pinoLDR);
  unsigned long agora = millis();

  if (btnC.fell())
  {
    if (leituraLDR > 2000 && agora - ultimaAtualizacaoMoeda > intervaloMoeda)
    {
      moedas = max(0, moedas - 1);
      lcd.setCursor(0, 3);
      lcd.print("Perdeu 1 moeda! ");
      exibirItemMenu();
      ultimaAtualizacaoMoeda = agora;
    }
  }
}

void limitMoedas()
{
  if (value.Moedas > value.maxMoedas) // Garante que não ultrapasse o máximo
  {
    value.Moedas = value.maxMoedas; // Reseta as moedas para 10 se exceder o limite
    Serial.println("Você atingiu o máximo de fragmentos. Use o Ritual do Esquecimento para eliminar uma carta.");
  }
}

void amountMoedas()
{
  Serial.print("Quantidade de fragmentos: ");
  Serial.println(value.Moedas); // Mostra a quantidade de moedas após a compra
}

void drawPhase()
{

  value.Moedas += 1; // Adiciona 1 fragmento ao total
  Serial.println("Você coletou um Fragmento do Elo");
  Serial.print("Quantidade de fragmentos restantes: ");
  if (value.Moedas > 10)
  {
    limitMoedas(); // Chama a função para limitar os fragmentos
  }

  player.draw = false;
}

void callback(char *topic, byte *payload, unsigned int length)
{
  // Serial.printf("Mensagem recebida em %s: ", topic);

  String mensagem = "";
  for (unsigned int i = 0; i < length; i++)
  {
    char c = (char)payload[i];
    mensagem += c;
  }

  mensagem.trim();
  mensagem.toLowerCase();

  // Serial.println(mensagem);

  if (mensagem == "boto")
  {
    cards.Boto = true; // Ativa a carta Boto
    // Serial.println("Carta Boto ativada!");
  }
  else if (mensagem == "saci")
  {
    cards.Saci = true; // Ativa a carta Saci
    // Serial.println("Carta Saci ativada!");
  }
  else if (mensagem == "curupira")
  {
    cards.Curupira = true; // Ativa a carta Curupira
    // Serial.println("Carta Curupira ativada!");
  }
  else if (mensagem == "ritual")
  {
    player.ritual = true; // Ativa o ritual do esquecimento
    // Serial.println("Ritual do Esquecimento ativado!");
  }
}
void mqttConnect()
{
  while (!client.connected())
  {
    Serial.println("Conectando ao MQTT...");

    if (client.connect(mqtt_id))
    {
      Serial.println("Conectado com sucesso");
      client.subscribe(mqtt_topic_sub);
    }
    else
    {
      Serial.print("Falha, rc=");
      Serial.println(client.state());
      Serial.println("Tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

