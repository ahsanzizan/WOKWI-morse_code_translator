#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD addr 0x27, 16 chars, 2 lines

const int buttonPin = 2;
int buttonState = HIGH;
unsigned long pressStartTime;
String morseCode = "";
String translatedText = "";

const char* morseAlphabet[] = {
    ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---",
    "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-",
    "..-", "...-", ".--", "-..-", "-.--", "--.."
};
const char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Morse Code:");
}

void loop() {
  int reading = digitalRead(buttonPin);

  if (reading == LOW) {
    if (buttonState == HIGH) {
      pressStartTime = millis();
    }
  } else {
    if (buttonState == LOW) {
      unsigned long pressDuration = millis() - pressStartTime;
      
      if (pressDuration < 300) {
        morseCode += ".";
      } else {
        morseCode += "-";
      }

      lcd.setCursor(0, 1);
      lcd.print(morseCode);
      delay(500);
    }
  }

  if (morseCode.length() > 0 && millis() - pressStartTime > 1500) {
    translate();
    morseCode = "";
  }

  buttonState = reading;
}

void translate() {
  for (int i = 0; i < 26; i++) {
    if (morseCode == morseAlphabet[i]) {
      translatedText += letters[i];
      break;
    }
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Text: " + translatedText);
}
