#include <Arduino.h>
#include "pins.h"
#include "melody.h"
#include "display.h"
#define ENM1 11
#define ENM2 12
#define PWM1 13
#define PWM2 14
#define DIR1 3
#define DIR2 15

Display display;
void setup()
{

  display.setup();
  display.drawLoadingScreen();

  pinMode(ENM1, OUTPUT);
  pinMode(ENM2, OUTPUT);
  pinMode(PWM1, OUTPUT);
  pinMode(PWM2, OUTPUT);
  pinMode(DIR1, OUTPUT);
  pinMode(DIR2, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  digitalWrite(ENM1, HIGH);
  digitalWrite(ENM2, HIGH);

  digitalWrite(DIR1, LOW);
  digitalWrite(DIR2, LOW);

  digitalWrite(BUZZER, HIGH);
  delay(200);
  digitalWrite(BUZZER, LOW);

  playMelody();
  delay(500);
  digitalWrite(DIR1, HIGH);
}

void loop()
{
  display.print("Zsolt Buzi", 42.0);
  analogWrite(PWM1, 128);
  // analogWrite(PWM2, 128);
  digitalWrite(ENM1, HIGH);
  // digitalWrite(ENM2, LOW);
  delay(1000);
  analogWrite(PWM1, 50);
  // analogWrite(PWM2, 50);
  // digitalWrite(ENM1, HIGH);
  // digitalWrite(ENM2, HIGH);
  delay(1000);
  // analogWrite(PWM1, 50);
  // analogWrite(PWM2, 50);
  digitalWrite(ENM1, LOW);
  // digitalWrite(ENM2, HIGH);
  delay(1000);
}