// Shows how to set up an OSC controlled parameter using the WiFlyHQ and ArdOSCForWiFlyHQ libraries
// Created 2012 by Felix Bonowski.
// This example code is in the public domain.

//by adding this define, we tell the compiler that we want to use the OSC functionality of the library
#define USE_OSC
// osc functionality depends on other libraries that can be found on my GitHub Page
#include <WiFlyHQ.h>
#include <ArdOSCForWiFlyHQ.h>
#include <ArduPar.h>

#include <SoftwareSerial.h>

//create a software serial that uses ordinary pins to communicate with the WiFly. 
//This technique has some limiations -  see documentations in the web for details.
// Most importantly, you can only recieve at one SoftwareSerial at a time
const int softSerialRxPin =2;
const int softSerialTxPin =3;
SoftwareSerial softSerial(softSerialRxPin,softSerialTxPin);


WiFly wifly; //create an instance of a Wifly Interface
OSCServer oscServer(&wifly);  //This will receive and parse incolming messages


// Create an integer setting that can by set via Serial and will remember its value even if the board is powered off.
// It needs to be setup() to be of any use.
IntArduPar someIntSetting;


void setup(){
  Serial.begin(115200);  //start Serial Communication to PC
  pinMode(13,OUTPUT);  //use the LED Pin as an example output.
  wifly.setupForUDP<SoftwareSerial>(
    &softSerial,   //the serial you want to use (this can also be a software serial)
    19200, // if you use a hardware serial, I would recommend the full 115200. This does not work with a software serial.
    true,	// should we try some other baudrates if the currently selected one fails?
    "name ",  //Your Wifi Name (SSID)
    "pass", //Your Wifi Password 
    "WiFly",                 // Device name for identification in the network
    0,                       // IP Adress of the Wifly (in quotes). if 0 (without quotes), it will use dhcp to get an ip
    8000,                    // WiFly receive port
    "255.255.255.255",       // Where to send outgoing Osc messages. "255.255.255.255" will send to all hosts in the subnet
    8001,                     // outgoing port
    true	// show debug information on Serial (turn this off when you connect the Wifly to your primary hardware serial)
  );
 
   wifly.printStatusInfo(); //print some debug information 
   
  // By defining a global Osc server for all settings, we do not have to specify it in every setup.
  // This way, you can swith on and off osc without breaking the setup() calls in your own modules.
  globalArduParOscServer=&oscServer;
  
  //we need to set up the someIntSetting to make it useful:
  someIntSetting.setup(
    F("/someInt"),                // The command used to change the parameter. The F("blah") syntax saves memory by putting the command into flash-memory. (look up "progmem strings" if you care)
    0,                            // The lowest value the parameter can have.
    255                           // The highest value the parameter can have.
  );
  
}
void loop(){
  //continue to check the Serial input for "/someInt" and set the value if it is received...
  //Enter i.e. "/someInt 5" into the serial monitor to set the parameter to a new value.
  updateParametersFromStream(&Serial,10); // the second value (10) determines how long we will wait for new data (timeout in ms).
  
  //you can also send an OSC Message to port 8000 of the module to set the parameter.
  // send a Message to the adress "/someInt" and see how it changes. This is also possible from your smartphone, e.g. with TouchOsc
  oscServer.availableCheck();  //process incoming osc messages and update the number if a message with its adress was received
  
  
  //the current value of the settung can be accessed by its ".value" field.
  //Here, we use it to fade the LED according to the current value 
  digitalWrite(13,HIGH); // we don't use analogWrite because it does not work with the Nano LED-Pin 13
  delayMicroseconds(someIntSetting.value);
  digitalWrite(13,LOW);
  delayMicroseconds(256-someIntSetting.value); 
  
  // We can also find out if new data was received at all using the ".valueReceived" field:
  if(someIntSetting.valueReceived){
	  Serial.print("New value is: ");
	  Serial.println(someIntSetting.value);
	  someIntSetting.valueReceived=false; // clear the flag so we can be notified again.
  }
}

// Here is a Processing code that uses oscP5 by andreas schlegel to send messages to your wifly:
// You need oscP5 from http://www.sojamo.de/oscP5 to run this code
// example based on oscP5message by andreas schlegel:

/*
int i=0; // the value to send
import oscP5.*;
import netP5.*;

OscP5 oscP5;  // the "sender" instance
NetAddress myRemoteLocation; //the net adress we will send data to

void setup() {
  size(400,400);
  frameRate(20); // send 20 packets/second
  
  // start oscP5, listening for incoming messages at port 12000. The listening port does not matter for our purposes
  oscP5 = new OscP5(this,12000);
  
  // !ToDo! Set the IP adress of your Wifly here. The second Number is the receive port of the WiFly
  myRemoteLocation = new NetAddress("192.168.43.166",8000);
}


void draw() {
  background (i); //set the background to the same brightness as the led on the arduino...
  // create new message with the same adress as the ArduPar parameter:
  OscMessage myMessage = new OscMessage("/someInt"); 
  
  // add the value we want to send to the message
  myMessage.add(i);

  println("sending "+i);
  // send the message
  oscP5.send(myMessage, myRemoteLocation); 
  
  //ramp up the parameter and start over after 255
  i++;
  if(i>=255){i=0;}
}

*/
