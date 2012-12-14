
//by adding this define, we tell the compiler that we want to use the OSC functionality of the library
#define USE_OSC
// osc functionality depends on other libraries
#include <WiFlyHQ.h>
#include <ArdOSCForWiFlyHQ.h>
#include <ArduPar.h>

// the oscSetup.h takes care of all the complexity of setting up Wifi and osc.
#include "oscSetup.h"
//Everything that is necessary for an integer setting that can by set via Serial and will persist even if the board is powered off:

// Create an integer setting that can by set via Serial and will remember its value even if the board is powered off.
// It needs to be setup() to be of any use.
IntParameterSetting someIntSetting;


void setup(){
  Serial.begin(115200);  //start Serial Communication
   
  configureWiFly(
    &wifly,
    &Serial2,  //this assumes you use a Arduino MEGA board with additional UARTS. The Wifly will also work on a software serial, but only at low baudrates.
    115200,    // a more reasonable baudrate than the 9600 default... 
    "link-local",
    "12BZr2mM3FLrRSL8",
    "WiFly",      // for identification in the network
    0,         // IP Adress of the Wifly. if 0 (without quotes), it will use dhcp to get an ip
    8000,                    // WiFly receive port
    "255.255.255.255",       // Where to send outgoing Osc messages. "255.255.255.255" will send to all hosts in the subnet
    8001                     // outgoing port
    );
    
  // By defining a global Osc server for all settings, we do not have to specify it in every setup.
  // This way, you can swith on and off osc without breaking the setup() calls in your own modules.
  globalArduParOscServer=&oscServer;
  
  //we need to set up the someIntSetting to make it useful:
  someIntSetting.setup(
    F("/someInt"),                // The command used to change the parameter. The F("blah") syntax saves memory by putting the command into flash-memory. (look up "progmem strings" if you care)
    0,                            // The lowest value the parameter can have.
    10                           // The highest value the parameter can have.
  );
  
}
void loop(){
  //continue to check the Serial input for "/someInt" and set the value if it is received...
  //Enter i.e. "/someInt 10" into the serial monitor to set the parameter to a new value.
  updateParametersFromStream(&Serial,10);
  
  //you can also send an OSC Message to port 8000 of the module to set the parameter.
  // send a Message to the adress "/someInt" and see how it changes. This is also possible from your smartphone, e.g. with TouchOsc
  oscServer.availableCheck();  //process incoming osc messages and update the number if a message with its adress was received
  Serial.println(someIntSetting.value);
  delay(100);
}
