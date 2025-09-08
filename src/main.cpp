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
const uint8_t SCREEN_WIDTH = 128;
const uint8_t SCREEN_HEIGHT = 32;

HX711 scale;
Servo servo;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

uint16_t servoPosition = 0;

char servoPositionChars[3];
char weightChars[128];

void setup()
{
  Serial.begin(9600);
  Serial.println("Setup in progress");
  scale.begin(LOADCELL_DT_PIN, LOADCELL_SCK_PIN);
  servo.attach(SERVO_DT_PIN);

  Serial.println("Begin display setup");
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
      Serial.println("Cannot connect display");
      for (;;)
        ;
  }

  Serial.println("Display setup complete");
  display.clearDisplay();
  display.display();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  delay(1000);
}

void loop()
{
  Serial.print("Servo position: ");
  Serial.println(servoPosition);

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
  delay(2000);
}
