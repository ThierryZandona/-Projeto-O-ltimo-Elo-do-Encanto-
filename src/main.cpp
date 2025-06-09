#include <Arduino.h>
#include <Bounce2.h>
#include <LiquidCrystal_I2C.h>

// === Protótipos ===
void mostrarInstrucoesMenu();
void exibirItemMenu();
void atualizarBotoes();
void atualizarJoystick();
void verificarLDR();
void sortearCartas();
const char* LendaNome(int codigo);

// === Pinos ===
const int pinoLDR = 32;
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
const unsigned long debounceJoystick = 150;
int UltimaDirecaoX = 0;
int UltimaDirecaoY = 0;

// === Estado do jogo ===
int moedas = 2;

// === Tempo da mecânica LDR ===
const unsigned long intervaloMoeda = 500;
unsigned long ultimaAtualizacaoMoeda = 0;

// === Ícone personalizado ===
byte moedaChar[8] = {
    B00100,
    B01110,
    B11111,
    B11011,
    B11111,
    B01110,
    B00100,
    B00000};

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
    return "COLETA 3 MOEDAS";
  case 2:
    return "ROUBA 2 MOEDAS ";
  case 3:
    return "BLOQUEIO       ";
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

  lcd.clear();
  lcd.setCursor(4, 1);
  lcd.print("ULTIMO ELO");
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

  do {
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
  lcd.print("Menu de Acoes:");
  lcd.setCursor(0, 1);
  lcd.print("> ");
  lcd.print(obterDescricaoAcao(indiceMenu + 1));
  lcd.setCursor(0, 2);
  lcd.print("C1:");
  lcd.print(LendaNome(lenda1));
  lcd.print(" C2:");
  lcd.print(LendaNome(lenda2));
  lcd.setCursor(0, 3);
  lcd.print("Moedas: ");
  lcd.print(moedas);
  lcd.write(byte(0));
}

// === Loop principal ===
void loop()
{
  if (!cartasSorteadas)
  {
    sortearCartas();
    return;
  }

  atualizarBotoes();
  atualizarJoystick();
  verificarLDR();
}

// === Atualizar botões ===
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
      lcd.print("Ação selecionada:");
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
    return;

  int y = analogRead(joystickYPin) - 2048;
  int x = analogRead(joystickXPin) - 2048;
  int dirX = 0;
  int dirY = 0;

  if (abs(y) > deadzone)
    dirY = (y > 0) ? 1 : -1;
  if (abs(x) > deadzone)
    dirX = (x > 0) ? 1 : -1;

  unsigned long agora = millis();
  if (agora - ultimaLeituraJoystick < debounceJoystick)
    return;

  if (dirY != UltimaDirecaoY)
  {
    if (dirY == 1)
    {
      indiceMenu = (indiceMenu - 1 + totalAcoes) % totalAcoes;
      exibirItemMenu();
    }
    else if (dirY == -1)
    {
      indiceMenu = (indiceMenu + 1) % totalAcoes;
      exibirItemMenu();
    }
    ultimaLeituraJoystick = agora;
  }

  UltimaDirecaoY = dirY;

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

  if (leituraLDR > 1200 && agora - ultimaAtualizacaoMoeda > intervaloMoeda)
  {
    moedas = max(0, moedas - 1);
    lcd.setCursor(0, 3);
    lcd.print("Perdeu 1 moeda! ");
    delay(1000);
    exibirItemMenu();
    ultimaAtualizacaoMoeda = agora;
  }
}
