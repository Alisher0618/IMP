#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RotaryEncoder.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

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

void checkPosition() { // if comes interruption of encoder (inc or dec delay)
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
  
  pinMode(GPIO_16, INPUT_PULLUP);

  encoder = new RotaryEncoder(GPIO_14, GPIO_27, RotaryEncoder::LatchMode::TWO03);
  encoder->setPosition(1);
  
  attachInterrupt(digitalPinToInterrupt(GPIO_14), checkPosition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(GPIO_27), checkPosition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(GPIO_16), switchLanguage, FALLING);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  display.display();
  delay(2000);
  display.clearDisplay();
}

void loop() {
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

  if(selectedMenuItem == 10 && chooseLang == true){
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 10);
    display.println("AHOJ");
    display.display();
  }else if(selectedMenuItem == 20 && chooseLang == true){
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 10);
    display.println("HELLO");
    display.display();
  }

  display.display();
  delay(100);

}
