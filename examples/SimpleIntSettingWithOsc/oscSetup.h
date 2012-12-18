#pragma once
#include "Arduino.h"
#include <WiFlyHQ.h>
#include <ArdOSCForWiFlyHQ.h>
#include <ArduPar.h>


//#include "SoftwareSerial.h"

#define DEBUG 1
#ifndef TRACE
#define TRACE(x) do { if (DEBUG) Serial.print( x); } while (0)
#endif
#ifndef TRACELN
#define TRACELN(x) do { if (DEBUG) Serial.println( x); } while (0)
#endif
//we create these instances globally so they can be used from everywhere
WiFly wifly;
OSCServer oscServer(&wifly);
OSCClient oscClient(&wifly);


void configureWiFly(
WiFly* wiFly,
HardwareSerial* wiflySerial,
const uint32_t newSerialSpeed,
const bool tryOtherSpeeds,	///< should we try some other baudrates if the currently selected one fails?
const char* SSID,
const char* password,
const char* deviceID,      ///< for identifacation in the network
const char* localIP,       ///< a string with numbers, if 0, we will use dhcp to get an ip
const uint16_t localPort,
const char* remoteHost,
const uint16_t remotePort
){
  TRACE((F("Free memory: ")));
  TRACELN((wiFly->getFreeMemory()));

  boolean saveAndReboot=false;
  char buf[32];

  //try out some different serial speeds until we find one that is working...
  const uint32_t serialSpeeds[]={
    newSerialSpeed,9600, 19200, 38400, 57600, 115200      };

  int speedIndex=0;
  int maxSpeedIndex;
  if(tryOtherSpeeds){maxSpeedIndex=6;}else{ maxSpeedIndex=1;}; 
  boolean baudrateFound=false;
  TRACELN(F("Trying to connect to WiFly"));
  
  for (speedIndex=0;speedIndex<maxSpeedIndex;speedIndex++){
    TRACE(F("Connecting to Wifly using Baudrate:"));
    TRACELN(serialSpeeds[speedIndex]);
    wiflySerial->begin(serialSpeeds[speedIndex]);
    if(wiFly->begin(wiflySerial, &Serial)){
      baudrateFound=true;
      break;
    }
  }

  if(!baudrateFound){
    TRACELN(F("Could not find working Baud rate to connect to WiFly"));
    return;
  }
  else{
    TRACE(F("Working Baud rate is "));
    TRACELN(serialSpeeds[speedIndex]);
  }

  wiFly->startCommand();  //made this public to avoid the annoing waiting times
  if(serialSpeeds[speedIndex]!=newSerialSpeed){
    TRACE(F("Setting new baud rate to "));
    TRACELN(newSerialSpeed);
    //set the new wifly baud rate
    wiFly->setBaud(newSerialSpeed);
    wiFly->save();
    wiFly->reboot();
    wiflySerial->begin(newSerialSpeed);
    saveAndReboot=false;
  }
  //set
  
  wiFly->getDeviceID(buf, sizeof(buf));
  if(strcmp(buf, deviceID) != 0){
    TRACE(F("Changing device ID to "));
    TRACELN(deviceID);
    wiFly->setDeviceID(deviceID);
    saveAndReboot=true;
  }

  //setup dhcp or an ip..
  if(localIP==0){
    TRACELN(F("No fixed IP provided"));
    if(wiFly->getDHCPMode()!=WIFLY_DHCP_MODE_ON){
      TRACELN(F("Enabling DHCP"));

      wiFly->enableDHCP();
      saveAndReboot=true;
    }
  }
  else{
    if(wiFly->getDHCPMode()!=WIFLY_DHCP_MODE_OFF){
      TRACE(F("Disabling DHCP; setting IP to "));
      TRACELN(localIP);
      wiFly->disableDHCP();
      wiFly->setIP(localIP);
      saveAndReboot=true;
    }
    //set ip only if necessary...
    wiFly->getIP(buf,sizeof(buf));  
    if(strcmp(buf, localIP) != 0){
      TRACE(F("Setting IP to "));
      TRACELN(localIP);
      wiFly->setIP(localIP);
      saveAndReboot=true;
    }
  }

  //receive packets at this port...
  if(wiFly->getPort()!=localPort){
    TRACE(F("Setting listen Port to "));
    TRACELN(localPort);
    wiFly->setPort( localPort);	// Send UPD packets to this server and port
    saveAndReboot=true;
  }

  //2 millis seems to be the minimum for 9600 baud
  if (wiFly->getFlushTimeout() != 2) {
    TRACE(F("Setting Flush timeout to 2ms "));
    wiFly->setFlushTimeout(2);
    saveAndReboot=true;
  }
  
  //set SSID only if necessary...
  wiFly->getSSID(buf,sizeof(buf));  
  if(strcmp(buf, SSID) != 0){
    TRACE(F("Setting SSID to "));
    TRACELN((SSID));
    wiFly->setSSID(SSID);
    
    TRACE(F("Setting Wifi Passwd to "));
    TRACELN((password));
    wiFly->setPassphrase(password);
    saveAndReboot=true;
  }
  
  wiFly->save();
  wiFly->finishCommand();
  if(saveAndReboot==true){
    wiFly->reboot();
    wiflySerial->begin(newSerialSpeed);
  }


  wiFly->startCommand();
  //set SSID only if necessary...
  wiFly->getSSID(buf,sizeof(buf));  
  if(strcmp(buf, SSID) != 0){
    wiFly->setSSID(SSID);
    wiFly->setPassphrase(password);
  }
      
      
  /* Join wifi network if not already associated */
  if (!wiFly->isAssociated()) {
//  if(true){
    /* Setup the WiFly to connect to a wifi network */
    TRACELN(F("Joining network"));
    wiFly->setSSID(SSID);
       
    if (wiFly->join()) {
    //  wiFly->setopt("set w a ", 1);
    //if (wiFly->join(SSID, password, localIP==0, WIFLY_MODE_WEP)) {
      TRACELN(F("Joined wifi network"));
    } 
    else {
      TRACELN(F("Failed to join wifi network"));
    }
  } 
  else {
    TRACELN(F("Already joined network"));
  }

  //enable auto join
  wiFly->setJoin(WIFLY_WLAN_JOIN_AUTO);
  /* Setup for UDP packets, sent automatically */
  wiFly->setIpProtocol(WIFLY_PROTOCOL_UDP);
  wiFly->setHost(remoteHost, remotePort);	// Send UPD packets to this server and port

  TRACE("MAC: ");
  TRACELN(wifly.getMAC(buf, sizeof(buf)));
  TRACE("IP: ");
  TRACELN(wifly.getIP(buf, sizeof(buf)));
  TRACE("Netmask: ");
  TRACELN(wifly.getNetmask(buf, sizeof(buf)));
  TRACE("Gateway: ");
  TRACELN(wifly.getGateway(buf, sizeof(buf)));
  TRACE("Listen Port:");
  TRACELN(wifly.getPort());
    wiFly->finishCommand();
    TRACELN(F("WiFly setup finished"));
}

void printWiFlyInfo(){
  wifly.startCommand();
  char buf[32];
  /* Ping the gateway */
  wifly.getGateway(buf, sizeof(buf));

  TRACE("ping ");
  TRACE(buf);
  TRACE(" ... ");
  if (wifly.ping(buf)) {
    TRACELN("ok");
  } 
  else {
    TRACELN("failed");
  }

  TRACE("ping google.com ... ");
  if (wifly.ping("google.com")) {
    TRACELN("ok");
  } 
  else {
    TRACELN("failed");
  }

  TRACE("MAC: ");
  TRACELN(wifly.getMAC(buf, sizeof(buf)));
  TRACE("IP: ");
  TRACELN(wifly.getIP(buf, sizeof(buf)));
  TRACE("Netmask: ");
  TRACELN(wifly.getNetmask(buf, sizeof(buf)));
  TRACE("Gateway: ");
  TRACELN(wifly.getGateway(buf, sizeof(buf)));
  TRACE("Listen Port:");
  TRACELN(wifly.getPort());
  wifly.finishCommand();

}





