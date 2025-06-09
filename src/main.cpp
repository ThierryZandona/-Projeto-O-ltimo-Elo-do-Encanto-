#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#define led1 2
#define led2 18
#define led3 3
#define botao 2

LiquidCrystal_I2C lcd(0x27, 16, 4);

bool sorteado = false;
int moedas = 2;

byte moedaChar[8] = {
    B00100,
    B01110,
    B11111,
    B11011,
    B11111,
    B01110,
    B00100,
    B00000};

const char *LendaNome(int codigo)
{
  switch (codigo)
  {
    // case 1:
    // return "BOTO";
  case 2:
    return "CURUPIRA";
  case 3:
    return "IARA";
  case 4:
    return "SACI";
    // case 5:
    // return "MULA";
    // case 6:
    //  return "CAIPORA";
    // default:
    //   return "ERRO";
  }
}

const char *acaoLendas(int acao)
{
  switch (acao)
  {
  case 1:
    return "COLETA 3 MOEDAS";
  case 2:
    return "ROUBA 2 MOEDAS";
  case 3:
    return "BLOQUEIO";
  case 4:
    return "ELIMINAR CARTA";
  case 5:
    return "TROCAR CARTAS";
  case 6:
    return "OLHAR CARTA";
  default:
    return "ERRO";
  }
}

void setup()
{
  randomSeed(analogRead(36));

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(botao, INPUT_PULLUP);

  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
  lcd.createChar(0, moedaChar);

  lcd.setCursor(5, 0);
  lcd.print("BEM VINDO");

  lcd.setCursor(8, 1);
  lcd.print("AO");

  lcd.setCursor(4, 2);
  lcd.print("ULTIMO ELO");

  lcd.setCursor(5, 3);
  lcd.print("ENCANTADO");

  delay(10000);

  lcd.clear();
  lcd.setCursor(3, 1);
  lcd.print("INICIANDO JOGO");
  delay(2000);
}

void loop()
{
  if (digitalRead(botao) == LOW && !sorteado)
  {
    sorteado = true;
    {

      digitalWrite(led1, LOW);
      digitalWrite(led2, LOW);

      for (int i = 0; i < 5; i++)
      {
        digitalWrite(led1, HIGH);
        digitalWrite(led2, HIGH);
        delay(500);
        digitalWrite(led1, LOW);
        digitalWrite(led2, LOW);
        delay(500);
      }

      lcd.clear();
      lcd.print("SORTEANDO...");
      delay(1000);

      int lenda1 = random(1, 4);
      int lenda2;

      do
      {
        lenda2 = random(1, 4);
      } while (lenda2 == lenda1);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("CARTA 1: ");
      lcd.print(LendaNome(lenda1));

      lcd.setCursor(0, 1);
      lcd.print("CARTA 2: ");
      lcd.print(LendaNome(lenda2));

      lcd.setCursor(18, 0);
      lcd.print(moedas);
      lcd.write(byte(0));

      
    }
    lcd.setCursor(0, 3);
      lcd.print("ACOES: ");
      //lcd.print(acaoLendas);
      delay(10000);
  }
}
 