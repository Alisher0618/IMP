/*
  Projekt: Měření vzdálenosti laserovým senzorem
  Author: Alisher Mazhirinov, xmazhi00
  VUT FIT, 2023
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_VL53L0X.h>
#include <RotaryEncoder.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

const int GPIO_14 = 14; 
const int GPIO_27 = 27; 
const int GPIO_16 = 16; //Button click

const int menuItems = 2; 
int selectedMenuItem = 10; 

const char* lang[] = {"Czech/Cestina", "English/Anglictina"};

RotaryEncoder *encoder = nullptr;

#define STEP 10
#define MIN_VALUE 10
#define MAX_VALUE 20
#define ERRORDIST 819.10

void checkPosition() { // if comes interruption of encoder (inc or dec)
  encoder->tick();
}

bool chooseLang = false;
int counter = 0;

void switchLanguage() {
  if(chooseLang == false){
    chooseLang = true;
  }else{
    chooseLang = false;
  }
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
  encoder->setPosition(1);
  
  attachInterrupt(digitalPinToInterrupt(GPIO_14), checkPosition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(GPIO_27), checkPosition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(GPIO_16), switchLanguage, FALLING);

  display.display();
  delay(2000);
  display.clearDisplay();
}

float max_dist = 0.0; // for additional info about max distance

void loop() {
  if(chooseLang == false){
    display.clearDisplay();
    display.setTextSize(1.2);
    display.setTextColor(SSD1306_WHITE);
    
    display.setCursor(0, 0);
    display.println("Choose Language/");
    display.println("Vyberte Jazyk");

    for (int i = 1; i <= menuItems; ++i) {
      display.setCursor(0, (i + 2) * 10);
      if (i == (selectedMenuItem / 10)) {
        display.println("> " + String(lang[i - 1]));
      } else {
        display.println("  " + String(lang[i - 1]));
      }
    }

    int newPos = encoder->getPosition() * STEP;
    
    if (newPos < MIN_VALUE) {
      encoder->setPosition(MIN_VALUE / STEP);
      newPos = MIN_VALUE;
    } 
    else if (newPos > MAX_VALUE) { 
      encoder->setPosition(MAX_VALUE / STEP);
      newPos = MAX_VALUE;
    }

    selectedMenuItem = newPos;
  }
  
  VL53L0X_RangingMeasurementData_t measure;

  lox.rangingTest(&measure, false);
  

  if (measure.RangeStatus != 4) {

    //encoder->tick(); // Regularly check the state of the encoder

    float distance_cm = measure.RangeMilliMeter / 10.0;
    
    if(selectedMenuItem == 10 && chooseLang == true){
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 0);
      display.println("> Zpatky");
      if (distance_cm > 101.0) {
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 20);
        display.print(F("Vzdl.: Mimo dosah"));
      } else {
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 20);
        display.print(F("Vzdalenost: "));
        display.print(distance_cm);
        display.print(F(" cm"));
        if (distance_cm > max_dist && distance_cm != ERRORDIST) {
          max_dist = distance_cm;
        }
      }
      display.setCursor(0, 30);
      display.print(F("Max vzdl.: "));
      display.print(max_dist);
      display.print(F(" cm"));

      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 40);
      display.println("Autor: xmazhi00");
    }else if(selectedMenuItem == 20 && chooseLang == true){
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 0);
      display.println("> Back");
      if (distance_cm > 101.0) {
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 20);
        display.print(F("Dist.: Out of range"));
      } else {
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 20);
        display.print(F("Distance: "));
        display.print(distance_cm);
        display.print(F(" cm"));

        if (distance_cm > max_dist && distance_cm != ERRORDIST) {
          max_dist = distance_cm;
        }
      }

      display.setCursor(0, 30);
      display.print(F("Max dist.: "));
      display.print(max_dist);
      display.print(F(" cm"));

      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 40);
      display.println("Author: xmazhi00");
    }
  }else {
    Serial.println(F("Out of range"));
  }

  display.display();
  delay(100);
}