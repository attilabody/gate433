// Do not remove the include below
#include <Arduino.h>"
#include "LiquidCrystal.h"

LiquidCrystal lcd(4,5,6,7,8,9);

void setup() {
  Serial.begin(19200);
  lcd.begin(16, 2);
  lcd.print("hello, world!");
  pinMode(13, OUTPUT);
}

void loop() {
  static uint16_t lastsec(0);
  lcd.setCursor(0, 1);
  uint16_t cursec(millis()/1000);
  if(lastsec != cursec) {
    lcd.print(cursec);
    digitalWrite(13, (cursec & 1) ? HIGH : LOW);
    Serial.println(cursec);
    lastsec = cursec;
  }
}

