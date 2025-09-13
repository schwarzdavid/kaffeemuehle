#include "Arduino.h"
#include "HX711.h"
#include "Servo.h"
#include "SPI.h"
#include "Wire.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include "Button2.h"
#include "EEPROM.h"
#include "Fonts/FreeSans18pt7b.h"

//////////////////////////////////////////////////
// DEFINE PINS
//////////////////////////////////////////////////

const uint8_t LOADCELL_DT_PIN = 2, 
              LOADCELL_SCK_PIN = 3, 
              SERVO_DT_PIN = 6, 
              LED_GREEN_PIN = 5,
              LED_RED_PIN = 4,
              BTN_PLUS_PIN = A0,
              BTN_MINUS_PIN = A1,
              BTN_START_PIN = A2,
              BTN_RESET_PIN = A3;

const uint8_t BTN_CLICK_DEBOUNCE_MS = 30;
const uint16_t BTN_PERSIST_DEBOUNCE_MS = 1000;

const uint8_t TARGET_WEIGHT_EEPROM_ADDRESS = 0;

//////////////////////////////////////////////////
// DEFINE GLOBAL VARIABLES
//////////////////////////////////////////////////

HX711 scale;
Servo servo;
Adafruit_SSD1306 display(128, 32, &Wire, -1);
Button2 btnStart, btnPlus, btnMinus, btnReset;

uint16_t servoPosition = 0;
uint8_t targetWeight = 16;

char servoPositionChars[3], weightChars[128];

//////////////////////////////////////////////////
// CALLBACKS
//////////////////////////////////////////////////

void onButtonPlusMinusClick(Button2& btn) {
  if(btn.getID() == BTN_PLUS_PIN) {
    targetWeight++;
  } else if(btn.getID() == BTN_MINUS_PIN) {
    targetWeight--;
  } else {
    return;
  }

  Serial.print("New target weight: ");
  Serial.println(targetWeight);
}

bool targetSavedThisHold = false;
unsigned long bothPressedSince = 0;
void checkBothPlusMinusPressed() {
  if(btnPlus.isPressed() && btnMinus.isPressed()) {
    if(bothPressedSince == 0) {
      bothPressedSince = millis();
    } else if(targetSavedThisHold == false && millis() - bothPressedSince >= BTN_PERSIST_DEBOUNCE_MS) {
      EEPROM.update(TARGET_WEIGHT_EEPROM_ADDRESS, targetWeight);
      targetSavedThisHold = true;
      Serial.println("Saved target weight to EEPROM");

      btnPlus.resetPressedState();
    }
  } else {
    bothPressedSince = 0;

    // Allow new save after both buttons have been released
    if(!btnPlus.isPressed() && !btnMinus.isPressed()) {
      targetSavedThisHold = false;
    }
  }
}

//////////////////////////////////////////////////
// SETUP & LOOP
//////////////////////////////////////////////////

void setup()
{
  Serial.begin(9600);
  Serial.println("Setup in progress");

  Serial.println("Setup leds");
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  digitalWrite(LED_GREEN_PIN, HIGH);
  digitalWrite(LED_RED_PIN, HIGH);

  Serial.println("Setup saved target weight");
  uint8_t savedTargetWeight;
  EEPROM.get(TARGET_WEIGHT_EEPROM_ADDRESS, savedTargetWeight);
  if(savedTargetWeight != 255) {
    targetWeight = savedTargetWeight;
    Serial.print("Loaded previously saved target weight: ");
    Serial.println(targetWeight);
  }

  Serial.println("Setup scale");
  scale.begin(LOADCELL_DT_PIN, LOADCELL_SCK_PIN);

  Serial.println("Setup servo");
  servo.attach(SERVO_DT_PIN);
  servo.write(0);

  // BUTTON SETUP
  Serial.println("Setup buttons");

  // Start
  btnStart.begin(BTN_START_PIN);

  // Plus
  btnPlus.begin(BTN_PLUS_PIN);
  btnPlus.setID(BTN_PLUS_PIN);
  btnPlus.setDebounceTime(BTN_CLICK_DEBOUNCE_MS);
  btnPlus.setTapHandler(onButtonPlusMinusClick);

  // Minus
  btnMinus.begin(BTN_MINUS_PIN);
  btnMinus.setID(BTN_MINUS_PIN);
  btnMinus.setDebounceTime(BTN_CLICK_DEBOUNCE_MS);
  btnMinus.setTapHandler(onButtonPlusMinusClick);

  Serial.println("Setup display");
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
      Serial.println("Cannot connect display");
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setFont(&FreeSans18pt7b);
  display.display();

  delay(1000);
  digitalWrite(LED_GREEN_PIN, LOW);
}

void loop()
{
  btnPlus.loop();
  btnMinus.loop();
  btnStart.loop();
  btnReset.loop();

  checkBothPlusMinusPressed();

  display.clearDisplay();
  display.setCursor(10, 30);
  display.print(targetWeight);
  display.print("G");
  display.display();

  /*display.clearDisplay();
  display.setCursor(0, 0);
  display.write("Servo: ");
  itoa(servoPosition, servoPositionChars, 10);
  display.write(servoPositionChars);

  servo.write(servoPosition);
  servoPosition = (servoPosition + 30) % 210;

  display.setCursor(0, 12);

  if (scale.wait_ready_timeout(1000)) {
    long reading = scale.read();

    Serial.print("Weight: ");
    Serial.println(reading);

    display.write("Gewicht: ");
    itoa(reading, weightChars, 10);
    display.write(weightChars);

  } else {
    Serial.println("Scale not found");
    display.write("Keine Waage");
  }

  display.display();

  if(digitalRead(BTN_MINUS_PIN) == LOW || digitalRead(BTN_PLUS_PIN) == LOW || digitalRead(BTN_START_PIN) == LOW || digitalRead(BTN_RESET_PIN) == LOW) {
    digitalWrite(LED_GREEN_PIN, HIGH);
    digitalWrite(LED_RED_PIN, HIGH);
  } else if(digitalRead(BTN_PLUS_PIN) == HIGH) {
    digitalWrite(LED_GREEN_PIN, LOW);
    digitalWrite(LED_RED_PIN, LOW);
  }

  delay(2000);*/
}