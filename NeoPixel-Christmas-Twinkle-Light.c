// CHRISTMAS TWINKLE LIGHTS USING NEOPIXEL STRIP
// Copyright © 2018 - Barron Anderson
// License - Open Source - Free to use both personally and commercially.  Free to redistribute and/or modify as long as my copyright information
// remains at the top of the source code.  Not liable for any harm caused by this program.  This program is as-is.  No warranty is expressed or given.
// use at your own risk.
//
// ASSUMPTIONS:   YOU AS A DEVELOPER ARE FAMILIAR WITH THE ARDUINO IDE, THE NEOPIXEL PRODUCTS, AND SOME BASIC ELECTRONICS KNOWLEDGE.  YOU HAVE 
//                SUCCESSFULLY CONFIGURED AND TESTED AN ARDUINO DEVELOPMENT ENVIRONMENT WITH A NEOPIXEL STRIP, SHOWING THE ELECTRONICS PORTION
//                OF THE PROJECT IS IN WORKING ORDER BEFORE CONSIDERING AND/OR USING THIS SOURCE CODE PROJECT.
//
// WHAT IT DOES:  AN ARDUINO PROGRAM WHICH RENDERS COLORS TO RGB-BASED LIGHT EMITTING DIODES IN RANDOM FASHION TO MIMIC CHRISTMAS TWINKLE LIGHTS.
//                EACH LED CAN RENDER ONE OF 6 COLORS: RED, GREEN, BLUE, YELLOW, PURPLE, OR ORANGE.
//                THE SPEED AT WHICH THE LED WILL 'TURN ON' IS RANDOM.  THE AMOUNT OF TIME WHERE THE LIGHT REMAINS FULL BRIGHTNESS IS RANDOM.
//                THE AMOUNT OF TIME WHERE THE LIGHT REMAINS COMPLETELY OFF IS RANDOM.  EACH PIXEL CAN RANDOMLY BE A BLINKER OR REMAIN STATICALLY ON.
//                ALL THESE RANDOM VALUES ARE CONTROLLED BY GLOBAL CONSTANTS.  CHANGING THE CONSTANTS CAN HAVE A DRAMATIC EFFECT ON THE BEHAVIOR
//                OF THE CHRISTMAS LIGHTS.
//
// HOW DOES IT WORK: GLOBALLY WE CREATE AN ARRAY OF ChristmasPixel OBJECTS - EACH ONE REPRESENTS ONE LED.  IN SETUP FOR EACH
//                ELEMENT IN THE ARRAY WE CALL RandomizeColor(), RandomizeSpeedFactor(), RandomizeBlinkability(), RandomizeZeroBrightnessDelay(),
//                AND RandomizeFullBrightnessDelay() - THIS WILL SETUP AND RANDOMIZE EACH LED INDEPENDENT OF EACH OTHER IN TERMS OF COLOR, SPEED,
//                AND DELAYS BOTH FULL BRIGHTNESS AND OFF.  IF A PIXEL IS SELECTED AS A BLINKER, FOR EACH COMPLETE CYCLE OF A BLINK IT WILL RE-RANDOMIZE.
//                THIS MEANS THE COLOR OF THE LED CAN CHANGE TO A DIFFERENT COLOR EACH TIME IT BLINKS - AGAIN IT IS RANDOM SO IT MAY SELECT THE COLOR
//                IT ALREADY WAS.  TO MAKE THE LED APPEAR TO BLINK WE MUST CONSIDER HOW FAST IT SHOULD BLINK.  WE SET THE BRIGHTNESS OF EACH LED WITH
//                REGARDS TO ITS COLOR.  IN OTHER WORDS WE DIM IT BY REDUCING BRIGHTNESS ON EACH LED.  EACH LOOP WILL CALCULATE HOW MUCH BRIGHTNESS SHOULD
//                BE APPLIED BY CALCULATING HOW LONG SINCE WE STARTED TO DIM, OR BRIGHTEN THE LED AND COMPARE TO HOW FAST IT SHOULD ILLUMINATE OR DIM.
//                SEE FUNCTION _CalculateBlinkAnimationParameters() FOR IMPLEMENTATION OF THESE CALCULATIONS.
//
// TECHNICAL NOTES:  FOR A 1 METER STRAND OF 60 LEDS I SHOW ABOUT 0.6 AMPS AT 5V ARE USED.  IN MY PROJECT I USED THE ADAFRUIT PRO TRINKET 5V TO CONTROL 60 LEDS.
//                BUT I USED AN EXTERNAL POWER SOURCE, AND DID NOT TRY TO DERIVE POWER FOR THE LEDS FROM THE TRINKET ITSELF.
//                THIS SKETCH USES 5170 BYTES (18%) OF PROGRAM STORAGE SPACE AND 1485 BYTES OF DYNAMIC MEMORY FOR GLOBAL VARIABLES.
//
// HOW TO USE:    1.) SET CONSTANT 'PIXEL_COUNT' TO THE NUMBER OF PIXELS YOU INTEND TO DRIVE WITH THIS PROGRAM.
//                2.) SET THE CONSTANT 'PIN' TO THE ARDUINO LOGIC PIN DRIVING THE NEOPIXEL STRAND
//                3.) OPTIONAL - SET THE BLINK CONSTANTS TO OTHER VALUES.  THE CONSTANT 'BLINK_PROBABILITY' IS SET TO ZERO.  THIS MEANS
//                    ALL LEDS WILL BLINK.  IF YOU WANT ONLY 50% OF THE LEDS TO BLINK SET TO 1.  IF YOU WANT 33% OF THE LEDS TO BLINK SET IT
//                    TO 2, AND SO FORTH.
//
// DEPENDENCIES:  THIS PROGRAM RELIES ON THE ADAFRUIT NEOPIXEL LIBRARY "Adafruit_Neopixel".  PLEASE SEE ADAFRUIT FOR COPYRIGHT INFORMATION FOR THEIR LIBRARY.
//

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIXEL_COUNT 60
#define PIN 3
#define BLINK_PROBABILITY 0
#define BLINK_SPEED_FACTOR_PROBABILITY 4
#define BLINK_SPEED_FACTOR_MULTIPLIER_IN_MILLISECONDS 100
#define BLINK_SPEED_MINIMUM_IN_MILLISECONDS 70
#define BLINK_ZERO_BRIGHTNESS_DELAY_PROBABILITY_IN_MILLISECONDS 5000
#define BLINK_ZERO_BRIGHTNESS_DELAY_MINIMUM_IN_MILLISECONDS 500
#define BLINK_FULL_BRIGHTNESS_DELAY_PROBABILITY_IN_MILLISECONDS 15000
#define BLINK_FULL_BRIGHTNESS_DELAY_MINIMUM_IN_MILLISECONDS 500
#define MAIN_LOOP_DELAY_COUNT 255

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIN, NEO_GRB + NEO_KHZ800);

uint8_t loopCount = 0;

enum hue {
  red = 0,
  green = 1,
  blue = 2,
  yellow = 3,
  purple = 4,
  orange = 5
};

enum direction {
  up,
  down
};

class ChristmasPixel {
private:
  hue _hue;
  direction _direction;
  uint8_t _brightness;
  bool _shouldBlink;
  unsigned long _setTime;
  unsigned long _timeZeroBrightnessWasEncountered;
  unsigned long _timeFullBrightnessWasEncountered;
  void _CalculateBlinkAnimationParameters();
  uint8_t _blinkSpeedFactor;
  uint16_t _zeroBrightnessDelayInMilliseconds;
  uint16_t _fullBrightnessDelayInMilliseconds;
  uint16_t _CalculateBlinkSpeed();
public:
  ChristmasPixel();
  uint8_t Id;
  uint32_t GetColor();
  void RandomizeColor();
  void RandomizeSpeedFactor();
  void RandomizeBlinkability();
  void RandomizeZeroBrightnessDelay();
  void RandomizeFullBrightnessDelay();
};

ChristmasPixel::ChristmasPixel() {
  this->_setTime = millis();
  this->_brightness = 255;
  this->_direction = down;  
}

uint32_t ChristmasPixel::GetColor() {
  this->_CalculateBlinkAnimationParameters();
  
  switch(this->_hue) {
    case red:
      return strip.Color(this->_brightness*255/255, 0, 0);
      break;
    case green:
      return strip.Color(0, this->_brightness*255/255, 0);
      break;
    case blue:
      return strip.Color(0, 0, this->_brightness*255/255);
      break;
    case yellow:
      return strip.Color(this->_brightness*255/255, this->_brightness*100/255, 0);
      break;
    case purple:
      return strip.Color(this->_brightness*200/255, 0, this->_brightness*200/255);
      break;
    case orange:
      return strip.Color(this->_brightness*255/255, this->_brightness*30/255, 0);
      break;
  }
}

void ChristmasPixel::RandomizeColor() {
  unsigned long systemUptime = millis();
  randomSeed(systemUptime);
  this->_hue = (hue)random(6);
}

void ChristmasPixel::RandomizeSpeedFactor() {
  unsigned long systemUptime = millis();
  randomSeed(systemUptime);
  this->_blinkSpeedFactor = random(BLINK_SPEED_FACTOR_PROBABILITY);  
}

void ChristmasPixel::RandomizeBlinkability() {
  unsigned long systemUptime = millis();
  randomSeed(systemUptime);
  this->_shouldBlink = random(BLINK_PROBABILITY) == 0;
}

void ChristmasPixel::RandomizeZeroBrightnessDelay() {
  unsigned long systemUptime = millis();
  randomSeed(systemUptime);
  this->_zeroBrightnessDelayInMilliseconds = random(BLINK_ZERO_BRIGHTNESS_DELAY_PROBABILITY_IN_MILLISECONDS) + BLINK_ZERO_BRIGHTNESS_DELAY_MINIMUM_IN_MILLISECONDS;
}

void ChristmasPixel::RandomizeFullBrightnessDelay() {
  unsigned long systemUptime = millis();
  randomSeed(systemUptime);
  this->_fullBrightnessDelayInMilliseconds = random(BLINK_FULL_BRIGHTNESS_DELAY_PROBABILITY_IN_MILLISECONDS) + BLINK_FULL_BRIGHTNESS_DELAY_MINIMUM_IN_MILLISECONDS;
}

uint16_t ChristmasPixel::_CalculateBlinkSpeed() {
  uint16_t response = (this->_blinkSpeedFactor * BLINK_SPEED_FACTOR_MULTIPLIER_IN_MILLISECONDS) + BLINK_SPEED_MINIMUM_IN_MILLISECONDS;
  return response;
}

void ChristmasPixel::_CalculateBlinkAnimationParameters() {
  if(this->_shouldBlink == true) {
    if (this->_direction == down) {
      if(this->_brightness == 0) {
        // IF WE HAVE REACHED ZERO BRIGHTNESS WE NEED TO SEE IF THERE IS A DELAY BEFORE BEGINNING TO BRIGHTEN.
        unsigned long millisecondsSinceZeroBrightness = millis() - this->_timeZeroBrightnessWasEncountered;

        if(millisecondsSinceZeroBrightness > this->_zeroBrightnessDelayInMilliseconds) {
          // REDEFINE THE BLINKING LIGHT.
          this->_direction = up;
          this->RandomizeColor();
          this->RandomizeSpeedFactor();
          this->RandomizeZeroBrightnessDelay();
          this->RandomizeFullBrightnessDelay();
          this->_setTime = millis();
        }
      }
      else {
        // BRIGHTNESS IS NOT ZERO - CONTINUE TO DECREASE BRIGHTNESS
        uint16_t blinkSpeed = this->_CalculateBlinkSpeed();
        unsigned long millisecondsSinceSet = millis() - this->_setTime;
        float elapsedTimePercent = (float)millisecondsSinceSet/(float)blinkSpeed;
        
        if (elapsedTimePercent > 1.0) {
          elapsedTimePercent = 1.0;
        }
 
        this->_brightness = (255 - (elapsedTimePercent * 255));
 
        if (this->_brightness < 0) {
          this->_brightness = 0;
        }

        if(this->_brightness == 0) {
          this->_timeZeroBrightnessWasEncountered = millis();
        }
      }
    }
    else {
      // DIRECTION IS NOT DOWN (IT IS UP)
      if(this->_brightness == 255) {
        // IF WE HAVE REACHED FULL BRIGHTNESS WE NEED TO SEE IF THERE IS A DELAY BEFORE BEGINNING TO DIM.
        unsigned long millisecondsSinceFullBrightness = millis() - this->_timeFullBrightnessWasEncountered;

        if(millisecondsSinceFullBrightness > this->_fullBrightnessDelayInMilliseconds) {
          this->_direction = down;
          this->_brightness = 255;
          this->_setTime = millis();
        }
      }
      else{
        // BRIGHTNESS IS NOT 255.  CONTINUE TO INCREASE BRIGHTNESS
        uint16_t blinkSpeed = this->_CalculateBlinkSpeed();
        unsigned long millisecondsSinceSet = millis() - this->_setTime;
        float elapsedTimePercent = (float)millisecondsSinceSet/(float)blinkSpeed;
        
        if (elapsedTimePercent > 1.0) {
          elapsedTimePercent = 1.0;
        }
 
        this->_brightness = (elapsedTimePercent * 255);
 
        if (this->_brightness > 255) {
          this->_brightness = 255;
        }

        if(this->_brightness == 255) {
          this->_timeFullBrightnessWasEncountered = millis();
        }
      }
    }
  }
}

ChristmasPixel pixels[PIXEL_COUNT];

void setup() {
 
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif

  strip.begin();

  for(uint8_t i=0; i<PIXEL_COUNT; i++) {
    pixels[i].Id = i;
    pixels[i].RandomizeColor();
    pixels[i].RandomizeSpeedFactor();
    pixels[i].RandomizeBlinkability();
    pixels[i].RandomizeZeroBrightnessDelay();
    pixels[i].RandomizeFullBrightnessDelay();
    delay(1);
  }
  
  strip.show();
}

void loop() {
  for (uint8_t i=0; i<PIXEL_COUNT; i++) {
    uint32_t color = pixels[i].GetColor();
    strip.setPixelColor(i, color);
  }
  
  strip.show();
  
  if(loopCount < MAIN_LOOP_DELAY_COUNT){
    loopCount++;
  }
  else {
    loopCount = 0;
    // ALLOW THE RANDOMIZER TO GET A NEW VALUE EACH TIME WE LOOP
    delay(1);
  }
}


