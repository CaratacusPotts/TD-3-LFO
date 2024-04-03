#include <Arduino.h>

# define arrayDataSize 4800
# define recPin 7
# define filterOutPin A0
# define filterInPin A2
# define filterAmpPin A5
# define filterFreqPin A4
# define filterMinOut 25
# define filterMaxOut 1023
# define potTimeInterval 25
# define potBuffer 10


// MIDI Clock ***************************
unsigned long lastMidiClockTime = 0;
unsigned long midiClockPulseInterval = 0;  // Time between MIDI clock pulses in microseconds
bool clkFlag = false;

// REC **********************************
bool rec = false;
bool recReady = false;
bool recEnd = false;
bool recPinStateLast = false; 
int recKnobStart = 0;
int recArray[arrayDataSize];
int recLength = 1;
unsigned int recIndex = 0;
int filterFreq = 1;
int filterFreqDN = 0;
int filterFreqWarpCount = 0;
int filterAmpDN = 0;
int filterAmpMin = filterMinOut;
int filterAmpMax = filterMaxOut;

// Pots interface ***********************
unsigned long potTimeLast = 0;

void clk()
{
  if(rec) {recFilter(recIndex); recIndex++; if(recIndex >= arrayDataSize){recEnd=true;}}
  else    {playFilter();} 
  Serial.print(" Index ");Serial.println(recIndex);
}

void recFilter(int index)
{
 int v = analogRead(filterInPin);  Serial.println(v);
 if(v < 1) {v = recArray[index - 1];}
 recArray[index] = v;
 //Serial.print(v); 
}

void playFilter()
{
  int amount = map(recArray[recIndex],0,1023,filterAmpMin,filterAmpMax);
  if(amount < filterMinOut){amount = filterMinOut;} if(amount > filterMaxOut){amount = filterMaxOut;}
  analogWrite(filterOutPin, amount); 

  int warpTime = filterFreq;
  if(warpTime < 0) 
  {
    warpTime = 0;
    filterFreqWarpCount++;
    if(filterFreqWarpCount == 0){warpTime = 1; filterFreqWarpCount = filterFreq;}
    Serial.print(" ff ");Serial.print(filterFreq); Serial.print(" wcount ");Serial.println(filterFreqWarpCount);
  }
  recIndex = (recIndex + warpTime) % recLength; 
  //Serial.println(recArray[recIndex]);  
}

void checkInterface()
{
  bool recState = digitalRead(recPin);
  if(recState != recPinStateLast)
  {
    recPinStateLast = recState;
    if(!recState) {recKnobStart = analogRead(filterInPin); recReady = true;}
    else    {recEnd = true;}
  }
  
  int potValue = analogRead(filterAmpPin);                 // Read the potentiometer value to adjust the amplitude
  if(potValue+potBuffer > filterAmpDN || potValue-potBuffer < filterAmpDN) // knob has moved
  {
    filterAmpDN = potValue;                                //Serial.print(" filterAmpDN: ");Serial.print(filterAmpDN);  
    
    int v = map(potValue,0,1023,1,24); // (((potValue + 1) * 10 ) / 427) + 1;  // Map the potentiometer range to 1-24
    int range = filterMaxOut - filterMinOut;
    int newRange = range / (24/v);
    filterAmpMin = (filterMinOut + (range/2)) - (newRange /2);
    filterAmpMax = filterAmpMin + newRange;
    if(filterAmpMin >= filterAmpMax){filterAmpMax = filterAmpMin + 1;}
  }

  potValue = analogRead(filterFreqPin);                    // Read potentiometer value
  if(potValue+potBuffer > filterFreqDN || potValue-potBuffer < filterFreqDN) // knob has moved
  {
    filterFreqDN = potValue;                              //Serial.print(" filterFreqDN: ");Serial.print(filterFreqDN);   
    filterFreq = map(potValue,0,1023,-6,6);               // Map the potentiometer range to 1-24 and offset, so goes negative
    if(filterFreq == 0){filterFreq = 1;}
    filterFreqWarpCount = filterFreq;
  }
}


void setup() 
{
  pinMode(recPin, INPUT_PULLUP);
  //pinMode(filterOutPin, OUTPUT);  // Set pin mode before using analogWrite()
  analogWrite(filterOutPin, filterMinOut);

  recArray[0] = filterMinOut;
  recPinStateLast = digitalRead(recPin);  // get this for later.

  filterAmpDN = analogRead(filterAmpPin);
  filterFreqDN = analogRead(filterFreqPin);
  
  Serial.begin(115200);
  Serial1.begin(31250); // Initialize hardware Serial1 for MIDI communication
}

void loop() 
{
  if (Serial1.available())                                // Listen for MIDI Clock messages on Serial1
  {
    byte incomingByte = Serial1.read();
    if (incomingByte == 0xF8) {clkFlag = true;}                             // MIDI Clock message
  }
  if(clkFlag){clkFlag = false; clk();}
  
  unsigned long timeNow = millis();
  if(timeNow - potTimeLast > potTimeInterval)
  {
    potTimeLast = timeNow;
    checkInterface();
  }

  if(recReady)
  {
    int delta = analogRead(filterInPin);
    if(delta > (recKnobStart + potBuffer) || delta < (recKnobStart - potBuffer))  // if the knob has moved..
    {
      recReady = false;
      rec = true;
      recIndex = 0; 
      
      Serial.print("    Rec Start ");  
    }
  }
  if(recEnd)
  {
    recEnd = false;
    rec = false;

    // Find the Loop End Point.
    int beatsInLoop = recIndex / 24;                         // the +1 here is so loop finishes on the beat, not the step.
    int remainder = recIndex % 24;
    bool extend = false;
    if(remainder > 5){beatsInLoop ++; extend = true;}
    recLength = (beatsInLoop * 24);

    if(extend)    // if you need to extend the array because the loop needs a little more time
    {
      //Serial.print(" extend: ");
      unsigned int startFill = (recLength - 24) + remainder;// + 1;
      //Serial.print( "StartFill: ");Serial.println(startFill);
      for(int i = startFill; i<= recLength; i++)  {recFilter(i);}// Serial.println(recArray[i]);}
    }
    else if(remainder > 0) {recIndex = remainder; }  // set index so that the recording loops correctly at the next beat

    //Serial.print("    Rec STOP");
    //printArray();
    
    filterFreq = 1;  // reset
    filterAmpMin = filterMinOut; // reset
    filterAmpMax = filterMaxOut; // reset
  }
}
