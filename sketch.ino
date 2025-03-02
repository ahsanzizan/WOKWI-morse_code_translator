#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define LCD_ADDRESS      0x27
#define LCD_COLUMNS      16
#define LCD_ROWS         2
#define BUTTON_PIN       2
#define RESET_PIN        3
#define DOT_THRESHOLD    300
#define DASH_THRESHOLD   1000
#define LETTER_THRESHOLD 1500
#define WORD_THRESHOLD   3500
#define DEBOUNCE_DELAY   50

LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);

int lastButtonState = HIGH;
int buttonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long pressStartTime = 0;
unsigned long lastReleaseTime = 0;
String currentMorseCode = "";
String translatedText = "";
bool displayingTranslation = false;

const char* MORSE_ALPHABET[] = {
    ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---",  // A-J
    "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-",    // K-T
    "..-", "...-", ".--", "-..-", "-.--", "--.."                            // U-Z
};
const char* MORSE_NUMBERS[] = {
    "-----", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----."  // 0-9
};
const char* MORSE_SPECIAL[] = {
    ".-.-.-", "--..--", "..--..", "-.-.--"  // . , ? !
};
const char LETTERS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char NUMBERS[] = "0123456789";
const char SPECIAL[] = ".,?!";

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(RESET_PIN, INPUT_PULLUP);
  
  lcd.init();
  lcd.backlight();
  
  lcd.setCursor(0, 0);
  lcd.print("Morse Translator");
  lcd.setCursor(0, 1);
  lcd.print("Press to begin");
  delay(2000);
  
  resetDisplay();
}

void loop() {
  if (digitalRead(RESET_PIN) == LOW) {
    resetTranslation();
    delay(500);
    return;
  }
  
  // Read the button state with debouncing, holy shit this thing's complicated
  int reading = digitalRead(BUTTON_PIN);
  
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (reading != buttonState) {
      buttonState = reading;
      
      // Button pressed
      if (buttonState == LOW) {
        pressStartTime = millis();
        
        // Reset display if showing translated text
        if (displayingTranslation) {
          displayingTranslation = false;
          currentMorseCode = "";
          resetDisplay();
        }
      } 
      // Button released
      else {
        unsigned long pressDuration = millis() - pressStartTime;
        lastReleaseTime = millis();
        
        // Determine if . or -
        if (pressDuration < DOT_THRESHOLD) {
          currentMorseCode += ".";
        } else {
          currentMorseCode += "-";
        }
        
        // Display updated morse code
        lcd.setCursor(7, 0);
        lcd.print("            "); // Clear previous morse code
        lcd.setCursor(7, 0);
        lcd.print(currentMorseCode);
      }
    }
  }
  
  // Check for letter completion (space), also wtf is this conditional hell
  if (currentMorseCode.length() > 0 && buttonState == HIGH && 
      (millis() - lastReleaseTime) > LETTER_THRESHOLD && 
      (millis() - lastReleaseTime) < WORD_THRESHOLD) {
    
    translateAndAdd();
    updateDisplayText();
    currentMorseCode = "";
    lcd.setCursor(7, 0);
    lcd.print("            ");
  }
  
  // Check for WOrd completion (longer space)
  if (buttonState == HIGH && (millis() - lastReleaseTime) > WORD_THRESHOLD && 
      translatedText.length() > 0 && !displayingTranslation && 
      translatedText.charAt(translatedText.length()-1) != ' ') {
    
    translatedText += " ";
    updateDisplayText();
    
    // If text is too long, show full translation
    if (translatedText.length() > LCD_COLUMNS) {
      displayFullTranslation();
    }
  }
  
  lastButtonState = reading;
}

void resetTranslation() {
  translatedText = "";
  currentMorseCode = "";
  displayingTranslation = false;
  resetDisplay();
  
  lcd.setCursor(0, 1);
  lcd.print("Reset complete  ");
  delay(1000);
  resetDisplay();
}

void resetDisplay() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Morse: ");
  lcd.setCursor(0, 1);
  lcd.print("                ");
}

void updateDisplayText() {
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  
  // Truncate text if needed
  if (translatedText.length() <= LCD_COLUMNS) {
    lcd.print(translatedText);
  } else {
    // Show as much text as possible with an ellipsis to indicate there's more
    lcd.print(translatedText.substring(translatedText.length() - LCD_COLUMNS + 3));
    lcd.setCursor(0, 1);
    lcd.print("...");
  }
}

void translateAndAdd() {
  // TODO: can we use dynamic length for these?

  // Check against alphabet (A-Z)
  for (int i = 0; i < 26; i++) {
    if (currentMorseCode == MORSE_ALPHABET[i]) {
      translatedText += LETTERS[i];
      return;
    }
  }
  
  // Check against numbers (0-9)
  for (int i = 0; i < 10; i++) {
    if (currentMorseCode == MORSE_NUMBERS[i]) {
      translatedText += NUMBERS[i];
      return;
    }
  }
  
  // Check against special characters
  for (int i = 0; i < 4; i++) {
    if (currentMorseCode == MORSE_SPECIAL[i]) {
      translatedText += SPECIAL[i];
      return;
    }
  }
  
  // If no match found, add a '?' character
  translatedText += "?";
}

void displayFullTranslation() {
  displayingTranslation = true;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Translation:");
  
  // Display as much of the translated text as fits
  int endPos = min(translatedText.length(), (unsigned int)LCD_COLUMNS);
  lcd.setCursor(0, 1);
  lcd.print(translatedText.substring(0, endPos));
  
  // If text is longer than display, scroll it
  if (translatedText.length() > LCD_COLUMNS) {
    scrollLongText();
  }
}

void scrollLongText() {
  // TODO: Is there any more performant way to do this?
  for (int startPos = 0; startPos <= translatedText.length() - LCD_COLUMNS; startPos++) {
    lcd.setCursor(0, 1);
    lcd.print(translatedText.substring(startPos, startPos + LCD_COLUMNS));
    delay(300);
    
    // Check if reset button pressed during scrolling
    if (digitalRead(RESET_PIN) == LOW) {
      resetTranslation();
      return;
    }
  }
  delay(1000);
}
