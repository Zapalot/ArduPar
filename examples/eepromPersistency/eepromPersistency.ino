// Shows how persistency of settings can be controlled.
// The Arduinos EERPROM is used to save setting values when powered off.
// The value is saved every time the "setValue" method of the parameter is used. This is done automatically when you set them over Serial or OSC.
// If you want to chaneg the value in your code, use "setValue" if you want the change to be persistent.
//
// The settings a static instance of the EepromAdressManager class to get unique EEPROM adresses.
//
// If you plan to mix this with other code that uses EEPROM, take a look at the EepromAdressManager.h/.cpp code in the library.
//
// Created 2013 by Felix Bonowski.
// This example code is in the public domain.

#include <ArduPar.h>
#include <avr/eeprom.h>

IntArduPar somePersistentSetting; // This one will remember its value even when you power off the Arduino
IntArduPar someVolatileSetting;   // This one will forget its value  when you power off the Arduino

void setup(){
  Serial.begin(115200);  //start Serial Communication
  pinMode(13,OUTPUT);  //use the LED Pin as an example output.

  //if you want persistency, you have to 
  somePersistentSetting.setup(
  F("remember"),         // The command used to change the parameter. The F("foo") syntax saves memory by putting the command into flash-memory. (look up "progmem strings" if you care)
  0,                       // The lowest value the parameter can have. Values received via Serial will be clipped to this range.
  255,                     // The highest value the parameter can have. Values received via Serial will be clipped to this range.
  true                     // Remember the value. This is the default behavior. 
  );

  someVolatileSetting.setup(
  F("forget"),          // The command used to change the parameter. The F("foo") syntax saves memory by putting the command into flash-memory. (look up "progmem strings" if you care)
  0,                      // The lowest value the parameter can have. Values received via Serial will be clipped to this range.
  255,                    // The highest value the parameter can have. Values received via Serial will be clipped to this range.
  false                   // Don't remember the value...
  );
}
void loop(){
  //continue to check the Serial input for "someInt" and set the value if it is received...
  //Enter i.e. "someInt 10" into the serial monitor to set the parameter to a new value.
  updateParametersFromStream(&Serial,10);

  Serial.print("Remember: ");
  Serial.print(somePersistentSetting.value);
  Serial.print("\tforget: ");
  Serial.println(someVolatileSetting.value);
  delay (10);
}


