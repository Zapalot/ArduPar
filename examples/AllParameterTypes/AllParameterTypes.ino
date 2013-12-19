// Shows all the different setting types supported by the library
// Look into the other examples line "SimpleIntSetting" for details.
// Created 2013 by Felix Bonowski.
// This example code is in the public domain.

#include <ArduPar.h>
#include <avr/eeprom.h> 

// Create an settings for a few different types that can by set via Serial and will remember its value even if the board is powered off.
// It needs to be setup() to be of any use.
IntArduPar someIntSetting;
LongArduPar someLongSetting;
FloatArduPar someFloatSetting;

CallbackArduPar someCallback;  // Callbacks can be used to remotely trigger any function.

StringArduPar someStringSetting; // Strings need some additional code to work - see SimpleStringSetting example fro details.
const int someStringMaxLength=20;
char someStringBuffer[someStringMaxLength];// The char array that holds the string. This is what you will work with in your program.


void setup(){
  Serial.begin(115200);  //start Serial Communication
  pinMode(13,OUTPUT);  //use the LED Pin as an example output.
  //we need to set up the someIntSetting to make it useful:
  someIntSetting.setup(
      F("someInt"),          // The command used to change the parameter. The F("foo") syntax saves memory by putting the command into flash-memory. (look up "progmem strings" if you care)
    0,                       // The lowest value the parameter can have. Values received via Serial will be clipped to this range.
    255                       // The highest value the parameter can have. Values received via Serial will be clipped to this range.
    );
    
  someLongSetting.setup(
      F("someLong"),             // The command used to change the parameter. The F("foo") syntax saves memory by putting the command into flash-memory. (look up "progmem strings" if you care)
    0,                           // The lowest value the parameter can have. Values received via Serial will be clipped to this range.
    100000                       // The highest value the parameter can have. Values received via Serial will be clipped to this range.
    );
    
  someFloatSetting.setup(
    F("someFloat"),             // The command used to change the parameter. The F("foo") syntax saves memory by putting the command into flash-memory. (look up "progmem strings" if you care)
    0,                           // The lowest value the parameter can have. Values received via Serial will be clipped to this range.
    2*PI                       // The highest value the parameter can have. Values received via Serial will be clipped to this range.
    );
    
   someStringSetting.setup(
    F("someString"),              // The command used to change the parameter. The F("blah") syntax saves memory by putting the name into flash-memory. (look up "progmem strings" if you care)
    someStringBuffer,             // The string is stored in this buffer.
    someStringMaxLength          // The capacity of the buffer. Incoming Strings will be truncated to fit.
  ); 
  
  // The library has a built in function that will tell you all the parameters and their values.
  //its name is dumpParameterInfos(). This "Callbacksetting with call the attached function when its command is received.
  someCallback.setup(
    F("doSomething"),
    &doSomething	//the name of the function to be called
    );
}
void loop(){
  //continue to check the Serial input for "someInt" and set the value if it is received...
  //Enter i.e. "someInt 10" into the serial monitor to set the parameter to a new value.
  updateParametersFromStream(&Serial,10);
  
  Serial.print ("Some int: " );
  Serial.print (someIntSetting.value);

  Serial.print ("\tsome long: " );
  Serial.print (someLongSetting.value);
  
  Serial.print ("\tsome float: " );
  Serial.print (someFloatSetting.value);
  
  Serial.print ("\tsome string: " );
  Serial.println (someStringBuffer); //Note that the Stringsetting does not have a .value!
  delay(200);
}

// a function that will be called every time you type "doSomething" in the Serial monitor:
void doSomething(){
  Serial.println("nope, I'm on strike!");
}
