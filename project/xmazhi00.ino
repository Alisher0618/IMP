#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Adafruit_VL53L0X.h>
#include <RotaryEncoder.h>

#define GPIO_14 14
#define GPIO_27 27 

RotaryEncoder *encoder = nullptr; // defined in setup

#define ROTARYSTEPS 25 // increasing and decreasing step
#define ROTARYMIN 100 // min value for delay
#define ROTARYMAX 1000 // max value for delay
#define ROTARYDELAY 5 // const value for changing and operating the delay

#define ERRORDIST 819.10

#define OLED_RESET -1
Adafruit_SSD1306 display(OLED_RESET);
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

int interrupt_time = 0;
int initial_delay = 500; // Initial value of delay

void checkPosition(){ // if comes interrauption of encoder(inc or dec delay)
  if ((millis() - interrupt_time) > ROTARYDELAY){
    encoder->tick();
    interrupt_time = millis();
  }
}

void setup() {
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  if(!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while(1);
  }

  encoder = new RotaryEncoder(GPIO_14, GPIO_27, RotaryEncoder::LatchMode::TWO03);
  encoder->setPosition(20);

  attachInterrupt(digitalPinToInterrupt(GPIO_14), checkPosition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(GPIO_27), checkPosition, CHANGE);

  
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

    int newPos = encoder->getPosition() * ROTARYSTEPS;
    if (newPos < ROTARYMIN){ // cross the min value of DELAY
        encoder->setPosition(ROTARYMIN / ROTARYSTEPS);
        newPos = ROTARYMIN;

    } else if (newPos > ROTARYMAX){ // cross the max value of DELAY
      encoder->setPosition(ROTARYMAX / ROTARYSTEPS);
      newPos = ROTARYMAX;
    }

    if (initial_delay != newPos){  // Serial output, if delay is changed
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
    if(distance_cm > 101.0){
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0,0);
      display.print(F("Dist.: out of range"));
    }else{
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0,0);
      display.print(F("Distance: "));
      display.print(distance_cm);
      display.print(F(" cm"));

      if(distance_cm > max_dist && distance_cm != ERRORDIST){
        max_dist = distance_cm;
      }
    }
    
    display.setCursor(0,10);
    display.print(F("Delay: "));
    display.print(initial_delay);
    display.print(F(" ms"));

    display.setCursor(0,20);
    display.print(F("Max dist.: "));
    display.print(max_dist);
    display.print(F(" cm"));

    display.display();
  } else {
    Serial.println(F("Out of range"));
  }
  
  delay(initial_delay);
}