#include <Wire.h>

int latchPin = 10;
int clockPin = 12;
int dataPin = 11;

byte* leds[4];

void updateShiftRegister()
{
  digitalWrite(latchPin, LOW);
  for (int i = 0; i < 4; i++)
  { shiftOut(dataPin, clockPin, MSBFIRST, leds[i]);}
  digitalWrite(latchPin, HIGH);
}

void receiveEvent(int bytes)
{
  for (int i = 0; i < bytes; i++)
  {
    leds[i] = Wire.read();
  }
  updateShiftRegister();
}

void setup() {
  Wire.begin(9);
  Wire.onReceive(receiveEvent);
  Serial.begin(9600);
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
}
void loop()
{
  ;
}
