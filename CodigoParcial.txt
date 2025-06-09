#include <Arduino.h>
#include <Bounce2.h>

const int LDR_PIN = 2;

const int a_button = 14;
const int b_button = 12;
const int c_button = 27;
const int d_button = 13;

const int joystickXPin = 34;
const int joystickYPin = 35;

int UltimaDirecaoX = 0; // -1 esquerda, 0 neutro, 1 direita
int UltimaDirecaoY = 0; // -1 baixo, 0 neutro, 1 cima

Bounce btnA = Bounce();
Bounce btnB = Bounce();
Bounce btnC = Bounce();
Bounce btnD = Bounce();

unsigned long UltimaAtualizacaoMoeda = 0;
const unsigned long IntervaloMoeda = 500;

const int deadzone = 400;

void setup()
{
  Serial.begin(9600);

  pinMode(a_button, INPUT_PULLUP);
  pinMode(b_button, INPUT_PULLUP);
  pinMode(c_button, INPUT_PULLUP);
  pinMode(d_button, INPUT_PULLUP);

  btnA.attach(a_button);
  btnB.attach(b_button);
  btnC.attach(c_button);
  btnD.attach(d_button);

  btnA.interval(25);
  btnB.interval(25);
  btnC.interval(25);
  btnD.interval(25);
}

void loop()
{
  btnA.update();
  btnB.update();
  btnC.update();
  btnD.update();

  int ldrValue = analogRead(LDR_PIN);
  float voltage = ldrValue * (3.3 / 4095.0);

  if (ldrValue > 1200 && millis() - UltimaAtualizacaoMoeda > IntervaloMoeda)
  {
    Serial.println("Você Perdeu 1 Moeda");
    Serial.println("---------------------------");
    UltimaAtualizacaoMoeda = millis();
  }

  if (btnA.fell())
  {
    Serial.println("Botão A pressionado");
  }
  if (btnB.fell())
  {
    Serial.println("Botão B pressionado");
  }
  if (btnC.fell())
  {
    Serial.println("Botão C pressionado");
  }
  if (btnD.fell())
  {
    Serial.println("Botão D pressionado");
  }
  int xValue = analogRead(joystickXPin) - 2048;
  int yValue = analogRead(joystickYPin) - 2048;

  if (abs(xValue) < deadzone) xValue = 0;
  if (abs(yValue) < deadzone) yValue = 0;

  // Determinar direção atual
  int dirX = 0;
  int dirY = 0;

  if (xValue > 1000) dirX = 1;       // Direita
  else if (xValue < -1000) dirX = -1; // Esquerda

  if (yValue > 1000) dirY = 1;       // Cima
  else if (yValue < -1000) dirY = -1; // Baixo

  // Imprimir só se a direção X mudou
  if (dirX != UltimaDirecaoX) {
    if (dirX == 1) Serial.println("Joystick para direita");
    else if (dirX == -1) Serial.println("Joystick para esquerda");
    UltimaDirecaoX = dirX;
  }

  // Imprimir só se a direção Y mudou
  if (dirY != UltimaDirecaoY) {
    if (dirY == 1) Serial.println("Joystick para cima");
    else if (dirY == -1) Serial.println("Joystick para baixo");
    UltimaDirecaoY = dirY;
  }
}