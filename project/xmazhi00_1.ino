#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Adafruit_VL53L0X.h>
#include <RotaryEncoder.h>

#define GPIO_14 14
#define GPIO_27 27
#define GPIO_16 16

RotaryEncoder *encoder = nullptr;

#define DELAY_STEP 25
#define DELAY_MIN_VALUE 100
#define DELAY_MAX_VALUE 1000
#define DELAY_TIME 5 

#define ERRORDIST 819.10

#define OLED_RESET -1
Adafruit_SSD1306 display(OLED_RESET);
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

int interrupt_time = 0;
int initial_delay = 500;

bool isEnglish = true;

void checkPosition() { // if comes interruption of encoder (inc or dec delay)
  if ((millis() - interrupt_time) > DELAY_TIME) {
    encoder->tick();
    interrupt_time = millis();
  }
}

void switchLanguage() {
  isEnglish = !isEnglish;
}

void setup() {
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  pinMode(GPIO_16, INPUT_PULLUP);

  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while (1);
  }

  encoder = new RotaryEncoder(GPIO_14, GPIO_27, RotaryEncoder::LatchMode::TWO03);
  encoder->setPosition(20);

  attachInterrupt(digitalPinToInterrupt(GPIO_14), checkPosition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(GPIO_27), checkPosition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(GPIO_16), switchLanguage, FALLING);
  
  display.display();
  delay(2000);
  display.clearDisplay();
}

float max_dist = 0.0; // for additional info about max distance

void loop() {

  VL53L0X_RangingMeasurementData_t measure;

  lox.rangingTest(&measure, false);

  if (measure.RangeStatus != 4) {  // Check for a valid measurement

    encoder->tick(); // Regularly check the state of the encoder

    int newPos = encoder->getPosition() * DELAY_STEP;
    if (newPos < DELAY_MIN_VALUE) { // cross the min value of DELAY
      encoder->setPosition(DELAY_MIN_VALUE / DELAY_STEP);
      newPos = DELAY_MIN_VALUE;

    } else if (newPos > DELAY_MAX_VALUE) { // cross the max value of DELAY
      encoder->setPosition(DELAY_MAX_VALUE / DELAY_STEP);
      newPos = DELAY_MAX_VALUE;
    }

    if (initial_delay != newPos) {  // Serial output, if delay is changed
      Serial.print("Delay changed: ");
      Serial.println(newPos);
      initial_delay = newPos;
    }

    Serial.print(F("Distance (mm): "));
    Serial.println(measure.RangeMilliMeter);

    float distance_cm = measure.RangeMilliMeter / 10.0; // Convert mm to cm

    //------------------------- OUTPUT ON DISPLAY -------------------------

    display.clearDisplay();
    display.setTextSize(1);
    if (distance_cm > 101.0) {
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 0);
      display.print(F("Dist.: out of range"));
    } else {
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 0);
      if (isEnglish) {
        display.print(F("Distance: "));
      } else {
        display.print(F("Vzdalenost: "));
      }
      display.print(distance_cm);
      display.print(F(" cm"));

      if (distance_cm > max_dist && distance_cm != ERRORDIST) {
        max_dist = distance_cm;
      }
    }

    display.setCursor(0, 10);
    if (isEnglish) {
      display.print(F("Delay: "));
    } else {
      display.print(F("Zpozdeni: "));
    }
    display.print(initial_delay);
    display.print(F(" ms"));

    display.setCursor(0, 20);
    if (isEnglish) {
      display.print(F("Max dist.: "));
    } else {
      display.print(F("Max vzdl.: "));
    }
    display.print(max_dist);
    display.print(F(" cm"));

    display.display();
  } else {
    Serial.println(F("Out of range"));
  }

  delay(initial_delay);
}
