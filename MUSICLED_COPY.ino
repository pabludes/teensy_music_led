#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <FastLED.h>

#define NUM_LEDS 196
#define DATA_PIN 2
#define POT_DIM A18
#define POT_START_COLOR A16
#define POT_COLOR_STEP A14
#define MODE_SWITCH 32

CRGB leds[NUM_LEDS];

const int myInput = AUDIO_INPUT_LINEIN;
int setDim, setStartColor, setColorStep;
CHSV colors[NUM_LEDS];

AudioInputUSB        audioInput;         
AudioAnalyzeRMS      rms_L;
AudioAnalyzeRMS      rms_R;
AudioAnalyzeFFT1024  myFFT;
AudioOutputI2S       audioOutput;        

AudioConnection c3(audioInput, 0, rms_L, 0);
AudioConnection c4(audioInput, 1, rms_R, 0);
AudioConnection c5(audioInput, 0, audioOutput, 0);
AudioConnection c6(audioInput, 1, audioOutput, 1);
AudioConnection c7(audioInput, 0, myFFT, 0);

AudioControlSGTL5000 audioShield;


void setup() {
  pinMode(MODE_SWITCH, INPUT_PULLUP);
  AudioMemory(12);
  audioShield.enable();
  //audioShield.inputSelect(myInput);
  audioShield.volume(0.8);
  myFFT.windowFunction(AudioWindowHanning1024);
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setMaxRefreshRate(100, false);
  memcpy(colors, leds, sizeof(leds));
  test();
}

void loop() {
  
  setDim = map(analogRead(POT_DIM), 0, 1023, 255, 0);
  setStartColor = map(analogRead(POT_START_COLOR), 0, 1023, 360, 0);
  setColorStep = map(analogRead(POT_COLOR_STEP), 0, 1023, 20, -2); 
  
  if(digitalRead(MODE_SWITCH)){
    music_both_sides_rms(setStartColor, setColorStep, setDim);
  } else {
    music_middle_out_rms(setStartColor, setColorStep, setDim);
  }
}

void test() {
  for(int value = 0; value < 256; value++){
    for(int i = 0; i < NUM_LEDS; i++){
      leds[i] = CHSV(0, 0, value);
    }
    FastLED.show();
    delay(10);
  }
  delay(100);
  for(int i = 0; i < NUM_LEDS; i++){
    leds[i] = CRGB::Black;
  }
  FastLED.show();
  delay(10);
}

void music_both_sides_rms(int colorHue, int colorStep, int colorValue){
  
  if (rms_L.available() && rms_R.available()){
    
    uint8_t rms = (rms_L.read() + rms_R.read()) * 100;

    for(int i = 0; i < NUM_LEDS / 2; i++){
      if(i < rms){
        leds[i] = CHSV(colorHue + i * colorStep, 255, colorValue);
      } else {
        leds[i] = CHSV(0, 0, colorValue / 10);
      }      
    }
    
    int count = 0;
    for(int j = NUM_LEDS; j > NUM_LEDS / 2 - 1; j--){      
      if(count < rms){
        leds[j] = CHSV(colorHue + count * colorStep, 255, colorValue);
      } else {
        leds[j] = CHSV(0, 0, colorValue / 10);
      }
      count++;
    }
    
  FastLED.show();
  delay(10);
  }
}

void music_middle_out_rms(int colorHue, int colorStep, int colorValue){
  
  if (rms_L.available() && rms_R.available()){
    
    uint8_t rms = (rms_L.read() + rms_R.read()) * 80;
 
    for(int i = NUM_LEDS; i > 0; i--){
      colors[i] = colors[i - 1];
    }
    colors[0] = CHSV(colorHue + rms, 255, colorValue);         
    
    int count = 0;
    for(int i = NUM_LEDS / 2; i >= 0; i--){
      if(count < rms){
        leds[i] = CHSV(colors[count].hue, 255, colorValue);
      } else {
        leds[i] = CHSV(colors[count].hue, 255, colorValue / 7); //CRGB::Black;
      }
      count++;      
    }
    
    count = 0;
    for(int j = NUM_LEDS / 2; j < NUM_LEDS; j++){      
      if(count < rms){
        leds[j] = CHSV(colors[count].hue, 255, colorValue);
      } else {
        leds[j] = CHSV(colors[count].hue, 255, colorValue / 7); //CRGB::Black;
      }
      count++;
    }
    
  FastLED.show();
  delayMicroseconds(10);
  }
}
