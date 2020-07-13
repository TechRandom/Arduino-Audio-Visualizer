#include <arduinoFFT.h>
#include <FastLED.h>

#define SAMPLES 64        // Must be a power of 2
#define MIC_IN A0         // Use A0 for mic input
#define LED_PIN     2     // Data pin to LEDS
#define NUM_LEDS    64  
#define BRIGHTNESS  150    // LED information 
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB 
#define BUTTON_PIN 3
#define xres 8            // Total number of  columns in the display
#define yres 8            // Total number of  rows in the display

double vReal[SAMPLES];
double vImag[SAMPLES];

int Intensity[xres] = { }; // initialize Frequency Intensity to zero
int Displacement = 1;

CRGB leds[NUM_LEDS];            // Create LED Object
arduinoFFT FFT = arduinoFFT();  // Create FFT object

void setup() {
  pinMode(MIC_IN, INPUT);
  Serial.begin(115200);         //Initialize Serial
  delay(3000);                  // power-up safety delay
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip ); //Initialize LED strips
  FastLED.setBrightness(BRIGHTNESS);
}

void loop() {
  Visualizer(); 
}

void Visualizer(){
  //Collect Samples
  getSamples();
  
  //Update Display
  displayUpdate();
  
  FastLED.show();
}

void getSamples(){
  for(int i = 0; i < SAMPLES; i++){
    vReal[i] = analogRead(MIC_IN);
    Serial.println(vReal[i]);
    vImag[i] = 0;
  }

  //FFT
  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);

  //Update Intensity Array
  for(int i = 2; i < (xres*Displacement)+2; i+=Displacement){
    vReal[i] = constrain(vReal[i],0 ,2047);            // set max value for input data
    vReal[i] = map(vReal[i], 0, 2047, 0, yres);        // map data to fit our display

    Intensity[(i/Displacement)-2] --;                      // Decrease displayed value
    if (vReal[i] > Intensity[(i/Displacement)-2])          // Match displayed value to measured value
      Intensity[(i/Displacement)-2] = vReal[i];
  }
}

void displayUpdate(){
  int color = 0;
  for(int i = 0; i < xres; i++){
    for(int j = 0; j < yres; j++){
      if(j <= Intensity[i]){                                // Light everything within the intensity range
        if(j%2 == 0){
          leds[(xres*(j+1))-i-1] = CHSV(color, 255, BRIGHTNESS);
        }
        else{
          leds[(xres*j)+i] = CHSV(color, 255, BRIGHTNESS);
        }
      }
      else{                                                  // Everything outside the range goes dark
        if(j%2 == 0){
          leds[(xres*(j+1))-i-1] = CHSV(color, 255, 0);
        }
        else{
          leds[(xres*j)+i] = CHSV(color, 255, 0);
        }
      }
    }
    color += 255/xres;                                      // Increment the Hue to get the Rainbow
  }
}
