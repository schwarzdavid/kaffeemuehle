#include <Arduino.h>
#include "HX711.h"
#include "Servo.h"
#include "SPI.h"
#include "Wire.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"

const uint8_t LOADCELL_DT_PIN = 2;
const uint8_t LOADCELL_SCK_PIN = 3;
const uint8_t SERVO_DT_PIN = 6;

const uint8_t LED_GREEN_PIN = 5;
const uint8_t LED_RED_PIN = 4;

const uint8_t BTN_RESERVE_PIN = A0;
const uint8_t BTN_PLUS_PIN = A1;
const uint8_t BTN_MINUS_PIN = A2;
const uint8_t BTN_START_PIN = A3;

HX711 scale;
Servo servo;
Adafruit_SSD1306 display(128, 32, &Wire, -1);

uint16_t servoPosition = 0;

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
  servo.write(servoPosition);

  Serial.println("Begin display setup");
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
      Serial.println("Cannot connect display");
      for (;;)
        ;
  }
  display.clearDisplay();
  display.display();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  Serial.println("Display setup complete");

  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(BTN_MINUS_PIN, INPUT_PULLUP);
  pinMode(BTN_PLUS_PIN, INPUT_PULLUP);
  pinMode(BTN_START_PIN, INPUT_PULLUP);
  pinMode(BTN_RESERVE_PIN, INPUT_PULLUP);

  delay(1000);
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

  if(digitalRead(BTN_MINUS_PIN) == LOW || digitalRead(BTN_PLUS_PIN) == LOW || digitalRead(BTN_START_PIN) == LOW || digitalRead(BTN_RESERVE_PIN) == LOW) {
    digitalWrite(LED_GREEN_PIN, HIGH);
    digitalWrite(LED_RED_PIN, HIGH);
  } else if(digitalRead(BTN_PLUS_PIN) == HIGH) {
    digitalWrite(LED_GREEN_PIN, LOW);
    digitalWrite(LED_RED_PIN, LOW);
  }

  delay(2000);
}
