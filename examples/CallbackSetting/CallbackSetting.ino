#include <ArduPar.h>
#include <avr/eeprom.h>
// The Arduinos EERPROM is used to save setting values when powered off. The settings themselves will take care of the adressing.
// See the "Advanced" Example for less automation and more manual control

// Create an integer setting that can by set via Serial and will remember its value even if the board is powered off.
// It needs to be setup() to be of any use.
IntArduPar someIntSetting;

CallbackArduPar parDumpCallback;  //"setting" is misleading, this will just call a function to give you all parameters currently defined



void setup(){
  Serial.begin(115200);  //start Serial Communication
  
  //we need to set up the someIntSetting to make it useful:
  someIntSetting.setup(
      F("someInt"),          // The command used to change the parameter. The F("foo") syntax saves memory by putting the command into flash-memory. (look up "progmem strings" if you care)
    0,                       // The lowest value the parameter can have. Values received via Serial will be clipped to this range.
    10                       // The highest value the parameter can have. Values received via Serial will be clipped to this range.
    );
  
  // The library has a built in function that will tell you all the parameters and their values.
  //its name is dumpParameterInfos(). This "Callbacksetting withh call the attached function when its command is received.
  parDumpCallback.setup(
    F("dump"),
    &dumpParameterInfos
    );
  
  
}
void loop(){
  //continue to check the Serial input for "someInt" and set the value if it is received...
  //Enter i.e. "someInt 10" into the serial monitor to set the parameter to a new value.
  // Type "dump" to get all currently existing settings, their types, commands, values and min/max constraints
  updateParametersFromStream(&Serial,10);
  
  
  //the current value of the settung can be accessed by its ".value" field.
  Serial.println(someIntSetting.value);
  delay(100);
}
