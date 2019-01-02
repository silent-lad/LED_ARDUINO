#include <FastLED.h>

#define NUM_LEDS 50
#define LED_PIN 3
#define SOUND_PIN A1

float sensorvalue = 0, lastmaxsensorvalue = 0, lastminsensorvalue = 1024;
int curshow = NUM_LEDS;
float val;

int findLEDNum(int len, float soundMin, float soundMax, float sound);

int loopCount = 0;

float fade_scale = 1.1;

float fscale(float originalMin, float originalMax, float newBegin, float newEnd, float inputValue, float curve);
void FillLEDsFromPaletteColors(uint8_t colorIndex, int curshow);

CRGBPalette16 currentPalette;
TBlendType currentBlending;

CRGB leds[NUM_LEDS];

void setup()
{
    Serial.begin(9600);
    // put your setup code here, to run once:
    FastLED.addLeds<UCS1903, LED_PIN, BRG>(leds, NUM_LEDS);
    for (int i = 0; i < NUM_LEDS; i++)
        leds[i] = CRGB(0, 0, 255);
    currentPalette = PartyColors_p;
    currentBlending = LINEARBLEND;
    FastLED.show();
    delay(100);
}

void loop()
{
    static uint8_t startIndex = 0;
    startIndex = startIndex + 1; /* motion speed */
    FillLEDsFromPaletteColors(startIndex, curshow);

    sensorvalue = analogRead(SOUND_PIN);
    if (loopCount >= 50)
    {
        if (sensorvalue > lastmaxsensorvalue)
        {
            lastmaxsensorvalue = sensorvalue;
        }
        //    if(loopCount%20==0){
        //      if(lastmaxsensorvalue-lastminsensorvalue>=0)
        //        lastmaxsensorvalue--;
        //    }

        if (sensorvalue < lastminsensorvalue)
        {
            sensorvalue = lastminsensorvalue;
        }
        curshow = fscale(lastminsensorvalue, lastmaxsensorvalue, 0, NUM_LEDS, sensorvalue, 2);
        //  curshow = findLEDNum(NUM_LEDS,lastminsensorvalue,lastmaxsensorvalue,sensorvalue);
        //  curshow = abs(curshow);
        Serial.print(sensorvalue);
        Serial.print(" ");
        Serial.println(curshow);
        FillLEDsFromPaletteColors(startIndex, curshow);
        //  Serial.print(" ");
        //  Serial.println(lastmaxsensorvalue);
        //  for (int i = 0; i < NUM_LEDS; i++){
        //    if(i>curshow){
        //      leds[i]=CRGB(leds[i].r/fade_scale, leds[i].g/fade_scale, leds[i].b/fade_scale);
        //    }
        //    else {
        //      leds[i]= CRGB(0, 0, 255);
        //    }
        //  }
        //  leds[i] = CRGB(0, 255, 0);
        FastLED.show();
        loopCount++;
        delay(50);
    }
    else
    {
        if (sensorvalue < lastminsensorvalue)
        {
            lastminsensorvalue = sensorvalue;
        }
        Serial.print(lastminsensorvalue);
        Serial.print(" ");
        Serial.println(lastmaxsensorvalue);
        loopCount++;
        delay(100);
    }
}

void FillLEDsFromPaletteColors(uint8_t colorIndex, int curshow)
{
    uint8_t brightness = 255;

    for (int i = 0; i < NUM_LEDS; i++)
    {
        if (i <= curshow)
        {
            leds[i] = ColorFromPalette(currentPalette, colorIndex, brightness, currentBlending);
            colorIndex += 1;
        }
        else
        {
            leds[i] = CRGB(leds[i].r / fade_scale, leds[i].g / fade_scale, leds[i].b / fade_scale);
        }
    }
}

float fscale(float originalMin, float originalMax, float newBegin, float newEnd, float inputValue, float curve)
{

    float OriginalRange = 0;
    float NewRange = 0;
    float zeroRefCurVal = 0;
    float normalizedCurVal = 0;
    float rangedValue = 0;
    boolean invFlag = 0;

    // condition curve parameter
    // limit range

    if (curve > 10)
        curve = 10;
    if (curve < -10)
        curve = -10;

    curve = (curve * -.1);  // - invert and scale - this seems more intuitive - postive numbers give more weight to high end on output
    curve = pow(10, curve); // convert linear scale into lograthimic exponent for other pow function

    /*
   Serial.println(curve * 100, DEC);   // multply by 100 to preserve resolution  
   Serial.println(); 
   */

    // Check for out of range inputValues
    if (inputValue < originalMin)
    {
        inputValue = originalMin;
    }
    if (inputValue > originalMax)
    {
        inputValue = originalMax;
    }

    // Zero Refference the values
    OriginalRange = originalMax - originalMin;

    if (newEnd > newBegin)
    {
        NewRange = newEnd - newBegin;
    }
    else
    {
        NewRange = newBegin - newEnd;
        invFlag = 1;
    }

    zeroRefCurVal = inputValue - originalMin;
    normalizedCurVal = zeroRefCurVal / OriginalRange; // normalize to 0 - 1 float

    // Check for originalMin > originalMax  - the math for all other cases i.e. negative numbers seems to work out fine
    if (originalMin > originalMax)
    {
        return 0;
    }

    if (invFlag == 0)
    {
        rangedValue = (pow(normalizedCurVal, curve) * NewRange) + newBegin;
    }
    else // invert the ranges
    {
        rangedValue = newBegin - (pow(normalizedCurVal, curve) * NewRange);
    }

    return rangedValue;
}
