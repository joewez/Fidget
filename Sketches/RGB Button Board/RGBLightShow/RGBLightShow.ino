/*
  RGB Light Show
  Version 1.2
  Created 2014-06-07 J.G. Wezensky  

  This sketch uses an RGB LED and two pushbuttons to control the displaying of multi-colored light
  sequences.  Pressing the primary button will change display modes and pressing the second will
  cycle through options for each mode.  A visible multi-colored flash will indicate that the button
  press was accepted.
  
  Current Modes:
    Solid Color Mode - Will display a color continuously.  Second button changes color.
    Blink Color Mode - Will display a blinking color.  Second button changes the blink rate.
    Fade Color - Will fade out selected solid color.  Second button controls rate.
    Rainbow Mode - Will cycle through the various colors. Second button changes the color-change rate.
    Random Blink - Will blink random colors.  Second button controls rate.
    Random Rainbow - Will fast switch between random colors.  Second button controls rate.
    Fade Random - Will fade out a random color. Second button controls rate.
*/  

#define DEBUG

// adjust these for the physical device
const int LEFT_BUTTON_PIN = 2;
const int RIGHT_BUTTON_PIN = 4;
const int RED_PIN =  9;
const int GREEN_PIN = 10;
const int BLUE_PIN = 11;

// some global constants for the sketch
const byte MODE_RAINBOW = 0;
const byte MODE_SOLID = 1;
const byte MODE_BLINK = 2;
const byte MODE_FADE_COLOR = 3;
const byte MODE_RANDOM_BLINK = 4;
const byte MODE_RANDOM_RAINBOW = 5;
const byte MODE_FADE_RANDOM = 6;
const byte MODE_COUNT = 7;

const byte COLORS_RED[] = {255, 0, 0, 255, 255, 0, 127, 0, 255, 0, 127, 127, 0, 127, 127, 255 };
const byte COLORS_GREEN[] = {0, 255, 0, 255, 0, 255, 255, 255, 127, 127, 127, 0, 127, 127, 255, 127 };
const byte COLORS_BLUE[] = {0, 0, 255, 0, 255, 255, 0, 127, 0, 255, 0, 127, 127, 255, 127, 127 };
const byte COLOR_COUNT = 16;

const long DEBOUNCE_DELAY = 50;

// tracks the left button
int leftButtonState;
int leftLastButtonState = HIGH;
long leftLastDebounceTime = 0;

// tracks the right button
int rightButtonState;
int rightLastButtonState = HIGH;
long rightLastDebounceTime = 0;

// track the current mode of the program
byte currentMode = 0;
byte lastMode = 255;

// track the current color for Mode 0 & 1
byte currentColor = 0;
byte lastColor = 255;

// track blinking for Mode 1
int blinkDelay = 0;
int blinkDelays[] = {250, 500, 1000, 2000};
int blinkDelayCount = 4;
long lastBlinkTime = 0;
boolean blinkOn = false;

// track the current color for Mode 2
byte currentWheel = 0;
int wheelDelay = 1;
int wheelDelays[] = {50, 350, 700, 1000};
int wheelDelayCount = 4;
long lastWheelChangeTime = 0;

byte lastRed = 0;
byte lastGreen = 0;
byte lastBlue = 0;
float fadeAmount = 10.0;
boolean fadeUp = false;

void setup() {
  
#if defined DEBUG
  Serial.begin(9600);
#endif

  // watch for these two pins to go to GND
  pinMode(LEFT_BUTTON_PIN, INPUT);
  digitalWrite(LEFT_BUTTON_PIN, HIGH);
  pinMode(RIGHT_BUTTON_PIN, INPUT);
  digitalWrite(RIGHT_BUTTON_PIN, HIGH);
}

void loop() {

  // check the left button for a mode change
  int leftReading = digitalRead(LEFT_BUTTON_PIN);
  if (leftReading != leftLastButtonState)
  {
    if (leftReading == LOW)
      NextMode();
    leftLastDebounceTime = millis();
  } 
  if ((millis() - leftLastDebounceTime) > DEBOUNCE_DELAY)
    leftButtonState = leftReading;
  leftLastButtonState = leftReading;

  // check the right button for a color change
  int rightReading = digitalRead(RIGHT_BUTTON_PIN);
  if (rightReading != rightLastButtonState)
  {
    if (rightReading == LOW)
      NextSubmode();
    rightLastDebounceTime = millis();
  } 
  if ((millis() - rightLastDebounceTime) > DEBOUNCE_DELAY)
    rightButtonState = rightReading;
  rightLastButtonState = rightReading;

  // if we changed mode, re-initialize
  if (currentMode != lastMode)
  {
    StartMode();
    lastMode = currentMode;
  }
  else // otherwise continue current mode display
  {
    ContinueMode();
  }
  
}

// -----------------------------------------------------------------------------
// Routine to handle the change of modes
// -----------------------------------------------------------------------------
void StartMode()
{
  Flash();
  switch (currentMode)
  {
      case MODE_RAINBOW:
        SetWheelColor(currentWheel);
        lastWheelChangeTime = millis();
        break;
      case MODE_SOLID:
        SetDefinedColor(currentColor);
        break;
      case MODE_BLINK:
        SetDefinedColor(currentColor);
        blinkOn = true;
        lastBlinkTime = millis();
        break;
      case MODE_FADE_COLOR:
        SetDefinedColor(currentColor);
        fadeUp = true;
        lastBlinkTime = millis();
        break;
      case MODE_RANDOM_BLINK:
        SetWheelColor(random(256));
        blinkOn = true;
        lastBlinkTime = millis();
        break;        
      case MODE_RANDOM_RAINBOW:
        SetWheelColor(random(256));
        lastBlinkTime = millis();
        break;
      case MODE_FADE_RANDOM:
        SetWheelColor(random(256));
        fadeUp = true;
        lastBlinkTime = millis();
        break;
  }  
}

// -----------------------------------------------------------------------------
// Routine to process a loop cycle with no user changes
// -----------------------------------------------------------------------------
void ContinueMode()
{
  switch (currentMode)
  {
    case MODE_RAINBOW:
      if ((millis() - lastWheelChangeTime) > wheelDelays[wheelDelay])
      {
        NextWheelColor();
        lastWheelChangeTime = millis();
      }
      break;
    case MODE_SOLID:
      if (currentColor != lastColor)
      {
        SetDefinedColor(currentColor);
        lastColor = currentColor;
      }
      break;
    case MODE_BLINK:
      if ((millis() - lastBlinkTime) > blinkDelays[blinkDelay])
      {
        if (blinkOn)
          SetColor(0,0,0);
        else
          SetDefinedColor(currentColor);
        blinkOn = !blinkOn;
        lastBlinkTime = millis();
      }      
      break;
    case MODE_FADE_COLOR:
      if ((millis() - lastBlinkTime) > blinkDelays[blinkDelay])
      {
        if (lastRed <= 0 && lastGreen <= 0 && lastBlue <= 0)
          SetDefinedColor(currentColor);
        else
          FadeColor();
        lastBlinkTime = millis();
      }      
      break;    
    case MODE_RANDOM_BLINK:
      if ((millis() - lastBlinkTime) > blinkDelays[blinkDelay])
      {
        if (blinkOn)
          SetColor(0,0,0);
        else
          SetWheelColor(random(256));
        blinkOn = !blinkOn;
        lastBlinkTime = millis();
      }      
      break;
    case MODE_RANDOM_RAINBOW:
      if ((millis() - lastBlinkTime) > blinkDelays[blinkDelay])
      {
        SetWheelColor(random(256));
        lastBlinkTime = millis();
      }      
      break;
    case MODE_FADE_RANDOM:
      if ((millis() - lastBlinkTime) > blinkDelays[blinkDelay])
      {
        if (lastRed <= 0 && lastGreen <= 0 && lastBlue <= 0)
          SetWheelColor(random(256));
        else
          FadeColor();
        lastBlinkTime = millis();
      }      
      break;    
  }
}

// -----------------------------------------------------------------------------
// Visual indicator that a button press was caught
// -----------------------------------------------------------------------------
void Flash()
{
  byte i;
  for (i = 0; i < 3; i++)
  {
    SetColor(0,0,0);
    delay(10);
    SetColor(255,0,0);
    delay(50);
    SetColor(0,255,0);
    delay(50);
    SetColor(0,0,255);
    delay(50);
    SetColor(0,0,0);
  }
}

// -----------------------------------------------------------------------------
// Processes the press of the primary button
// -----------------------------------------------------------------------------
void NextMode()
{
  if ((currentMode + 1) >= MODE_COUNT)
    currentMode = 0;
  else
    currentMode++;
}

// -----------------------------------------------------------------------------
// Processes a press of the secondary button
// -----------------------------------------------------------------------------
void NextSubmode()
{
  switch (currentMode)
  {
    case MODE_RAINBOW:
      NextWheelRate();
      Flash();
      SetWheelColor(currentWheel);
      break;
    case MODE_SOLID:
      NextColor();
      break;
    case MODE_BLINK:
      NextBlinkRate();
      Flash();
      break;
    case MODE_FADE_COLOR:
      NextBlinkRate();
      Flash();
      break;      
    case MODE_RANDOM_BLINK:
      NextBlinkRate();
      Flash();
      break;
    case MODE_RANDOM_RAINBOW:
      NextBlinkRate();
      Flash();
      break;      
    case MODE_FADE_RANDOM:
      NextBlinkRate();
      Flash();
      break;      
  }
}

// -----------------------------------------------------------------------------
// Increases the delay for blinking
// -----------------------------------------------------------------------------
void NextBlinkRate()
{
  if ((blinkDelay + 1) >= blinkDelayCount)
    blinkDelay = 0;
  else
    blinkDelay++;
}

// -----------------------------------------------------------------------------
// Increases the delay for blinking
// -----------------------------------------------------------------------------
void NextWheelRate()
{
  if ((wheelDelay + 1) >= wheelDelayCount)
    wheelDelay = 0;
  else
    wheelDelay++;
}

// -----------------------------------------------------------------------------
// Moves to the next pre-defined color
// -----------------------------------------------------------------------------
void NextColor()
{
  if ((currentColor + 1) >= COLOR_COUNT)
    currentColor = 0;
  else
    currentColor++;
}

// -----------------------------------------------------------------------------
// Moves to the next position on the color wheel an displays it
// -----------------------------------------------------------------------------
void NextWheelColor()
{
  if (currentWheel >= 255)
    currentWheel = 0;
  else
    currentWheel++;
    
  SetWheelColor(currentWheel);
}

// -----------------------------------------------------------------------------
// Will set the RGB led color based on a 0-255 position on a color wheel
// -----------------------------------------------------------------------------
void SetWheelColor(byte WheelPos) 
{
  byte w;
  if(WheelPos < 85) 
  {
    w = WheelPos * 3;
    SetColor(w, 255 - w, 0);
  }
  else if(WheelPos < 170) 
  {
    w = (WheelPos - 85) * 3;
    SetColor(255 - w, 0, w);
  } 
  else 
  {
    w = (WheelPos - 170) * 3;
    SetColor(0, w, 255 - w);
  }
}

// -----------------------------------------------------------------------------
// Will slightly darken the current RGB color
// -----------------------------------------------------------------------------
void FadeColor()
{
  byte newRed = 0;
  byte newGreen = 0;
  byte newBlue = 0;
  
  if (lastRed > (byte)fadeAmount)
    newRed = lastRed - (byte)((fadeAmount / 100.0)* lastRed);
  if (lastGreen > fadeAmount)
    newGreen = lastGreen - (byte)((fadeAmount / 100.0)* lastGreen);
  if (lastBlue > fadeAmount)
    newBlue = lastBlue - (byte)((fadeAmount / 100.0)* lastBlue);

  SetColor(newRed, newGreen, newBlue);
}

// -----------------------------------------------------------------------------
// Will set the RGB led to one of a pre-defined set of colors
// -----------------------------------------------------------------------------
void SetDefinedColor(byte DefinedColor)
{
  SetColor(COLORS_RED[DefinedColor], COLORS_GREEN[DefinedColor], COLORS_BLUE[DefinedColor]);
}

// -----------------------------------------------------------------------------
// Base routine to set a color on the RGB led
// -----------------------------------------------------------------------------
void SetColor(byte Red, byte Green, byte Blue)
{
    analogWrite(RED_PIN, Red);
    analogWrite(GREEN_PIN, Green);
    analogWrite(BLUE_PIN, Blue);
    lastRed = Red;
    lastGreen = Green;
    lastBlue = Blue;  
#if defined DEBUG
    Serial.print("R=");
    Serial.print(lastRed);
    Serial.print(" G=");
    Serial.print(lastGreen);
    Serial.print(" B=");
    Serial.println(lastBlue);
#endif
}

