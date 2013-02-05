// Shows how to set up an OSC controlled parameter using the WiFlyHQ and ArdOSCForWiFlyHQ libraries
// Created 2012 by Felix Bonowski.
// This example code is in the public domain.

//by adding this define, we tell the compiler that we want to use the OSC functionality of the library
#define USE_OSC
// osc functionality depends on other libraries that can be found on my GitHub Page
#include <WiFlyHQ.h>
#include <ArdOSCForWiFlyHQ.h>
#include <ArduPar.h>

//uncomment this to use a software serial if you dont own a MEGA
//#include <SoftwareSerial.h>
//SoftwareSerial softSerial(softSerialRx,softSerialTx);


WiFly wifly; //create an instance of a Wifly Interface
OSCServer oscServer(&wifly);  //This will receive and parse incolming messages


// Create an integer setting that can by set via Serial and will remember its value even if the board is powered off.
// It needs to be setup() to be of any use.
IntArduPar someIntSetting;


void setup(){
  Serial.begin(115200);  //start Serial Communication to PC
   
  wifly.setupForUDP<HardwareSerial>(
    &Serial3,   //the serial you want to use (this can also be a software serial)
    115200, // if you use a hardware serial, I would recommend the full 115200. This does not work with a software serial.
    true,	// should we try some other baudrates if the currently selected one fails?
    "WLAN-466B23",  //Your Wifi Name (SSID)
    "SP-213B33501", //Your Wifi Password 
    "WiFly",                 // Device name for identification in the network
    "192.168.2.201",         // IP Adress of the Wifly. if 0 (without quotes), it will use dhcp to get an ip
    8000,                    // WiFly receive port
    "255.255.255.255",       // Where to send outgoing Osc messages. "255.255.255.255" will send to all hosts in the subnet
    8001,                     // outgoing port
    true	// show debug information on Serial
  );
 
   wifly.printStatusInfo(); //print some debug information 
   
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
  //Enter i.e. "/someInt 5" into the serial monitor to set the parameter to a new value.
  updateParametersFromStream(&Serial,10); // the second value (10) determines how long we will wait for new data (timeout in ms).
  
  //you can also send an OSC Message to port 8000 of the module to set the parameter.
  // send a Message to the adress "/someInt" and see how it changes. This is also possible from your smartphone, e.g. with TouchOsc
  oscServer.availableCheck();  //process incoming osc messages and update the number if a message with its adress was received
  Serial.println(someIntSetting.value);
  delay(100);
}
