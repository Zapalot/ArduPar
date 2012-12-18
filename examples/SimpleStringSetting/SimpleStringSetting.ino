// Shows how to set up an string setting that can be controlled via serial input
// Created 2012 by Felix Bonowski.
// This example code is in the public domain.

#include <ArduPar.h>
#include <avr/eeprom.h>

//Everything that is necessary for an string setting that will persist even if the board is powered off:

//1. The StringParameterSetting instance. This controls everything.
StringArduPar someStringSetting;

//2. the maximum length the string can be:
const int someStringMaxLength=20;

//3. The char array that holds the string. This is what you will work with in your program.
char someStringBuffer[someStringMaxLength];


void setup(){
  Serial.begin(115200);  //start Serial Communication
  
  //we need to set up the someStringSetting to make it useful:
    
   strcpy(someStringBuffer,"foobar");  //this is just to show that the current value will be overwritten with whatever garbage is in EEPROM
   someStringSetting.setup(
    F("someString"),              // The command used to change the parameter. The F("blah") syntax saves memory by putting the name into flash-memory. (look up "progmem strings" if you care)
    someStringBuffer,             // The string is stored in this buffer.
    someStringMaxLength          // The capacity of the buffer. Incoming Strings will be truncated to fit.
  );
}
void loop(){
  Serial.print("Current String value: ");
  //continue to check the Serial input for "someString" and set the value if it is received...
  //Enter i.e. "someString foo" into the serial monitor to set the parameter to a new value.
  updateParametersFromStream(&Serial,10);
  Serial.println(someStringBuffer);
  delay(500);
}
