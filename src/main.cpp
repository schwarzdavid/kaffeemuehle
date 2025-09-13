#include "Arduino.h"
#include "HX711.h"
#include "Servo.h"
#include "SPI.h"
#include "Wire.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include "Button2.h"
#include "EEPROM.h"

const uint8_t LOADCELL_DT_PIN = 2, 
              LOADCELL_SCK_PIN = 3, 
              SERVO_DT_PIN = 6, 
              LED_GREEN_PIN = 5,
              LED_RED_PIN = 4,
              BTN_RESET_PIN = A0,
              BTN_PLUS_PIN = A1,
              BTN_MINUS_PIN = A2,
              BTN_START_PIN = A3;

HX711 scale;
Servo servo;
Adafruit_SSD1306 display(128, 32, &Wire, -1);
Button2 btnStart, btnPlus, btnMinus, btnReset;

uint16_t servoPosition = 0;
uint8_t savedTargetWeightAddress = 0;

char servoPositionChars[3];
char weightChars[128];

void setup()
{
  Serial.begin(9600);
  Serial.println("Setup in progress");

  Serial.println("Setup scale");
  scale.begin(LOADCELL_DT_PIN, LOADCELL_SCK_PIN);

  Serial.println("Setup servo");
  servo.attach(SERVO_DT_PIN);
  servo.write(0);

  Serial.println("Setup buttons");
  btnStart.begin(BTN_START_PIN);
  btnPlus.begin(BTN_PLUS_PIN);
  btnMinus.begin(BTN_MINUS_PIN);

  Serial.println("Setup display");
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
      Serial.println("Cannot connect display");
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.display();

  Serial.println("Setup leds");
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);

  digitalWrite(LED_GREEN_PIN, HIGH);
  digitalWrite(LED_RED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_GREEN_PIN, LOW);
}

void loop()
{

  display.clearDisplay();
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

  delay(2000);
}
