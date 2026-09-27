#include "Arduino.h"
#include "HX711.h"
#include "Servo.h"
#include "SPI.h"
#include "Wire.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include "Button2.h"
#include "EEPROM.h"
#include "Fonts/FreeSans12pt7b.h"

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

//////////////////////////////////////////////////
// GLOBAL CONSTANTS
//////////////////////////////////////////////////

const uint8_t BTN_CLICK_DEBOUNCE_MS = 30;
const uint16_t BTN_PERSIST_DEBOUNCE_MS = 1000;

const uint8_t TARGET_WEIGHT_EEPROM_ADDRESS = 0;

const uint8_t STATE_TARE = 0,
              STATE_READY = 1,
              STATE_RUNNING = 2;

const float SCALE_FACTOR = 2160.12;

uint16_t SERVO_RESTING_POSITION = 0;
uint16_t SERVO_ACTIVE_POSITION = 20;

//////////////////////////////////////////////////
// GLOBAL VARIABLES
//////////////////////////////////////////////////

HX711 scale;
Servo servo;
Adafruit_SSD1306 display(128, 32, &Wire, -1);
Button2 btnStart, btnPlus, btnMinus, btnReset;

uint16_t servoPosition = 0;
uint8_t targetWeight = 16;
uint8_t currentState = STATE_TARE;

//////////////////////////////////////////////////
// HELPER FUNCTIONS
//////////////////////////////////////////////////

void printText(const String text)
{
  int16_t offsetX, offsetY;
  uint16_t width, height;
  display.setTextSize(1);
  display.getTextBounds(text, 0, 0, &offsetX, &offsetY, &width, &height);
  display.clearDisplay();
  display.setCursor((display.width() - offsetX - width) / 2, 26);
  display.print(text);
  display.display();
}

void printText(const char chars[])
{
  printText(String(chars));
}

//////////////////////////////////////////////////
// CALLBACKS
//////////////////////////////////////////////////

void onButtonPlusMinusClick(Button2 &btn)
{
  if (btn.getID() == BTN_PLUS_PIN)
  {
    targetWeight++;
  }
  else if (btn.getID() == BTN_MINUS_PIN)
  {
    targetWeight--;
  }
  else
  {
    return;
  }

  Serial.print("New target weight: ");
  Serial.println(targetWeight);
}

bool targetSavedThisHold = false;
unsigned long plusMinusPressedSince = 0;
void checkBothPlusMinusPressed()
{
  if (btnPlus.isPressed() && btnMinus.isPressed())
  {
    if (plusMinusPressedSince == 0)
    {
      plusMinusPressedSince = millis();
    }
    else if (targetSavedThisHold == false && millis() - plusMinusPressedSince >= BTN_PERSIST_DEBOUNCE_MS)
    {
      EEPROM.update(TARGET_WEIGHT_EEPROM_ADDRESS, targetWeight);
      targetSavedThisHold = true;
      Serial.println("Saved target weight to EEPROM");

      printText("Saved");
      delay(3000);
      btnPlus.resetPressedState();
      btnMinus.resetPressedState();
    }
  }
  else
  {
    plusMinusPressedSince = 0;

    // Allow new save after both buttons have been released
    if (!btnPlus.isPressed() && !btnMinus.isPressed())
    {
      targetSavedThisHold = false;
    }
  }
}

void onButtonStartClick(Button2 &btn) {
  if(currentState == STATE_TARE) {
    scale.tare();
    currentState = STATE_READY;
    return;
  }

  if(currentState == STATE_READY) {
    if(scale.get_units(5) < 60) {
      printText("CUP");
      delay(3000);
      return;
    }
    currentState = STATE_RUNNING;
    return;
  }

  if(currentState == STATE_RUNNING) {
    Serial.println("Im here");
    currentState = STATE_READY;
    return;
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
  if (savedTargetWeight != 255)
  {
    targetWeight = savedTargetWeight;
    Serial.print("Loaded previously saved target weight: ");
    Serial.println(targetWeight);
  }

  Serial.println("Setup scale");
  scale.begin(LOADCELL_DT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(SCALE_FACTOR);
  scale.wait_ready_timeout(10000);

  Serial.println("Setup servo");
  servo.attach(SERVO_DT_PIN);
  servo.write(SERVO_RESTING_POSITION);

  // BUTTON SETUP
  Serial.println("Setup buttons");

  // Start
  btnStart.begin(BTN_START_PIN);
  btnStart.setDebounceTime(BTN_CLICK_DEBOUNCE_MS);
  btnStart.setClickHandler(onButtonStartClick);

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
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println("Cannot connect display");
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setFont(&FreeSans12pt7b);
  display.display();
}

void loopTare() {
  digitalWrite(LED_GREEN_PIN, LOW);
  digitalWrite(LED_RED_PIN, HIGH);

  printText("TARE");

  btnStart.loop();
}

void loopReady() {
  digitalWrite(LED_GREEN_PIN, HIGH);
  digitalWrite(LED_RED_PIN, LOW);

  btnPlus.loop();
  btnMinus.loop();
  btnStart.loop();

  checkBothPlusMinusPressed();
  printText(String(targetWeight) + " g");
}

void mill() {
  digitalWrite(LED_GREEN_PIN, HIGH);
  digitalWrite(LED_RED_PIN, HIGH);

  float startWeight = scale.get_units(5);

  servo.write(SERVO_ACTIVE_POSITION);
  delay(100);

  while(currentState == STATE_RUNNING) {
    float currentWeight = scale.get_units(5);
    float milledCoffee = currentWeight - startWeight;


    printText(String(milledCoffee) + " / " + String(targetWeight) + " g");

    if(milledCoffee >= targetWeight) {
      break;
    }

    btnStart.loop();
  }

  servo.write(SERVO_RESTING_POSITION);
  delay(100);

  currentState = STATE_READY;
}

void loop()
{
  switch (currentState)
  {
  case STATE_TARE:
    loopTare();
    break;
  case STATE_READY:
    loopReady();
    break;
  case STATE_RUNNING:
    mill();
    break;
  }
}