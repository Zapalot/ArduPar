// Shows how to set up an integer setting that can be controlled via serial input
// Created 2012 by Felix Bonowski.
// This example code is in the public domain.

#include <ArduPar.h>
#include <avr/eeprom.h>

// Create an integer setting that can by set via Serial and will remember its value even if the board is powered off.
// It needs to be setup() to be of any use.
IntArduPar someIntSetting;

// The Arduinos EERPROM is used to save setting values when powered off. The settings themselves will take care of the adressing.
// See the "Advanced" Example for less automation and more manual control

void setup(){
  Serial.begin(115200);  //start Serial Communication
  pinMode(13,OUTPUT);  //use the LED Pin as an example output.
  //we need to set up the someIntSetting to make it useful:
  someIntSetting.setup(
      F("someInt"),          // The command used to change the parameter. The F("foo") syntax saves memory by putting the command into flash-memory. (look up "progmem strings" if you care)
    0,                       // The lowest value the parameter can have. Values received via Serial will be clipped to this range.
    255                       // The highest value the parameter can have. Values received via Serial will be clipped to this range.
    );
  
}
void loop(){
  //continue to check the Serial input for "someInt" and set the value if it is received...
  //Enter i.e. "someInt 10" into the serial monitor to set the parameter to a new value.
  updateParametersFromStream(&Serial,10);
  
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

/*
// The following code runs in Processing and sends commands to the Arduino:
// You will see the LED flicker because it is turned off while receiving values in the example code
// Created 2013 by Felix Bonowski. This code is in the public domain.
import processing.serial.*;

Serial myPort;  // Create object from Serial class
int curNumber=0; // The Number we will send to the Arduino

void setup() 
{
  // I know that the last port in the serial list on my PC
  // is always my  FTDI adaptor, so I open Serial.list()[Serial.list().length-1].
  // If the whole thing does not work, try a different port... 
  // you can also just directly use the port name  as a parameter:
  // (i.e. type '"COM18"' instead of 'Serial.list()[Serial.list().length-1]')
  
  // Open whatever port is the one you're using.
  String portName = Serial.list()[Serial.list().length-1];
  myPort = new Serial(this, portName, 115200); //open a connection with 115200Baud - this has to match the Baudrate in your Arduino sketch!
  frameRate(20); // send 20 values per second
}

void draw() {
  myPort.write("someInt "+curNumber+"\n"); //send a line containing the name and value separated by a space to the Serial.
  curNumber++;   // count on...
  if (curNumber>255)curNumber=0; // start over at zero once we have reached 255...  
}
*/