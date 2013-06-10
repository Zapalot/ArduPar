#pragma once
#include <avr/eeprom.h>
#include "EepromAdressManager.h"
#include "Arduino.h"

//optional osc support. If enabled, the library is dependent on ArdOSCForWiFlyHQ.
// you cann comment out the next line if you  do not use Wireless Osc
//#define USE_OSC


#ifdef USE_OSC
#include <ArdOSCForWiFlyHQ.h>
OSCServer* globalArduParOscServer=0;	///< you can set this to something to avoid having to pass a server for each setting setup.
#endif

#define DEBUG 0
#define TRACE(x) do { if (DEBUG) Serial.print( x); } while (0)
#define TRACELN(x) do { if (DEBUG) Serial.println( x); } while (0)

// the instances of classes derived from AbstractArduPar maintain a global list. You can change the maximum number of settings here.
#define PAR_SETTINGS_MAX_NUMBER 32

//this determines the maximum parameter command length
#define PAR_SETTINGS_BUFFER_SIZE 32
class AbstractArduPar;
AbstractArduPar* PAR_SETTINGS_INSTANCES[PAR_SETTINGS_MAX_NUMBER];
int PAR_SETTINGS_CUR_INSTANCE_NUMBER=0;

//strcmp_P( _mes->_oscAddress,(const char PROGMEM *) sinks[i]-> getAdress()) == 0

/// A common interface for all kinds of parameter settings.
class AbstractArduPar
#ifdef USE_OSC
: 
public OscMessageSink
#endif
{
public:
   const __FlashStringHelper* cmdString;           ///< serial input is parsed for this command string, anything that follows is interpreted as parameter data
  int cmdStringLength;                            ///< used for comparisons
 /// Initialize the setting. Has to be called for the setting to become usable.
  void setup(
   const __FlashStringHelper* cmdString	///< serial input is parsed for this command string, anything that follows is interpreted as parameter data
#ifdef USE_OSC
    , OSCServer *server
#endif
  ){
    TRACE((F("New serial cmd: ")));
	TRACELN((cmdString));
	this->cmdString=cmdString;
	this->cmdStringLength=strlen_P((const char PROGMEM *)cmdString);
	
    //register instance in the global array
    if(PAR_SETTINGS_CUR_INSTANCE_NUMBER<PAR_SETTINGS_MAX_NUMBER){
      PAR_SETTINGS_INSTANCES[PAR_SETTINGS_CUR_INSTANCE_NUMBER]=this;
      PAR_SETTINGS_CUR_INSTANCE_NUMBER++;
    }
    else{
      Serial.print(F("Max Parsetting instances exceeded, could not register"));
      Serial.println(cmdString);
    }
#ifdef USE_OSC
	if(server!=0){
    TRACE((F("New osc cmd: ")));
	TRACELN((cmdString));
		server->addOscMessageSink(this);
	}
#endif
}

/// digest incoming serial data that potentially contains a "set" command
  virtual void parseSerialData(char* data)        
  {
    TRACE((F("Matching serial cmd")));
	TRACE((cmdString));
	TRACE((F("to")));
    TRACELN((data));
    int foundPos=strncmp_P(data,(const char PROGMEM *)cmdString,cmdStringLength);
	
    if(foundPos==0){
		TRACE(F("matched:"));
		TRACELN((cmdString));
		parseParameterString(data+ strlen_P((const char PROGMEM *)cmdString));
    }
  }
  
  virtual void parseParameterString(char* data)=0;    ///< derived classed implement parsing and setting parameters from a string here
  virtual void dumpParameterInfo(Stream* out)=0;       ///< derived classed can give some information about semselves this way. preferably in a machine-readable way.

//optional osc support
#ifdef USE_OSC
	/// Get the OSC Adress of the Setting
  const __FlashStringHelper* getAdress(){
    return(cmdString);
  }
  /// Return true is the message OSC Adress is identical to that of the setting.
  boolean isOscMessageForMe(OSCMessage* mes){return (strcmp_P( mes->getOSCAddress(),(const char PROGMEM *) this->getAdress()) == 0);};
#endif

};


////////////////////
/// a setting for integer values with optional eeprom persistency
class IntArduPar: 
public AbstractArduPar{
public:
  int value;
  int* valuePointer;   ///< points to the parameter value to be set
  int eepromAdress;    ///< an adress in eeprom memory for permanent storage of the parameter value. When <0 no storage will happen
  int minValue;        ///< lower Bound of the Parameter. Received values are constrained to be in the Range of [minValue,maxValue]
  int maxValue;        ///< upper Bound of the Parameter. Received values are constrained to be in the Range of [minValue,maxValue]

  ///simplest possible constructor, will use own value and no persistency
  IntArduPar():
  value(0),
  valuePointer(&this->value)
  {
  }
/// set up the setting. has to be called to make it functional
  void setup(
	  const __FlashStringHelper* cmdString,
	  int minValue,
	  int maxValue,
	  boolean isPersistent=true,      ///< should the parameter value be initialized from eeprom on startup?
    int* valuePointer=0,			///< the setting can modify an arbitrary location im memory if you give it here. 
	int fixedEEPROMAdress=-1		///< if you want a specific fixed adress, specify it here
	#ifdef USE_OSC
	  ,OSCServer *server=globalArduParOscServer
	#endif
  ){
  if(valuePointer==0)valuePointer=&this->value;
	this->valuePointer=valuePointer;
	this->minValue=minValue;
	this->maxValue=maxValue;
	#ifdef USE_OSC
		AbstractArduPar::setup(cmdString,server);
	#else
		AbstractArduPar::setup(cmdString);
	#endif
	if(isPersistent){
	
		if(fixedEEPROMAdress==-1){TRACE((F("Getting EEPROM. Adress: ")));fixedEEPROMAdress=EepromAdressManager::getAdressFor(sizeof(int));};
		this->eepromAdress=fixedEEPROMAdress;
		TRACE((F("Init from EEPROM. Adress: ")));
		TRACE((int)(eepromAdress));
		eeprom_read_block(valuePointer,(void *) eepromAdress,2); 
		TRACE((F(" value:")));
		TRACELN((*valuePointer));
	}else{
		this->eepromAdress=-1; // used to signal non-persistence to other methods
	};	
  };

  //optional osc support
#ifdef USE_OSC
  void digestMessage(OSCMessage *_mes){
    if(!isOscMessageForMe(_mes))return;
    int newValue;
    if(_mes->getArgTypeTag(0)=='i'){
      newValue =  _mes->getArgInt32(0);
    }
    else{
      if(_mes->getArgTypeTag(0)=='f'){ 
        newValue = (int)_mes->getArgFloat(0);
      }    
      else{
        return;
      }
    }
    setValue(newValue);
  };
#endif

  ///set the attached integer parameter from a string that was received
  virtual void parseParameterString(char* data){
    setValue(atoi(data));
  };

  //set the value and rpint some debug info
  void setValue(int newValue){
    newValue=constrain(newValue,minValue,maxValue);
    TRACE((F("Setting ")));
    TRACE((this->cmdString));
    TRACE((F(" to ")));
    TRACE((newValue));
    TRACE((F("\n")));
    *valuePointer=newValue;
    //save the new value
    if(eepromAdress>=0){
      TRACE((F("Writing EEProm adress")));
      TRACE(eepromAdress);
      TRACE((F("\n")));
      eeprom_write_block(valuePointer,(void *) eepromAdress,2); 

    }
  };
  /// give human&machine readably status info
  virtual void dumpParameterInfo(Stream* out){
    out->print(F("int\t"));
    out->print(this->cmdString);
    out->print(F("\t"));
    out->print(this->cmdString);
    out->print(F("\t"));
    out->print(*valuePointer);
    out->print(F("\t"));
    out->print(minValue);
    out->print(F("\t"));
    out->print(maxValue);
    out->print(F("\n"));
  } 
};

////////////////////
/// a setting for triggering a callback function
class CallbackArduPar: 
public AbstractArduPar{
public:
	void (*callbackFunction)(void);   ///< points to the function to be called      
	/// Initialize the setting. Has to be called for the setting to become usable.
	void setup(
		const __FlashStringHelper* cmdString,		///< The string that will trigger the function.
		void (*callbackFunction)(void)			///< A pointer to the function that will be triggered.
		#ifdef USE_OSC
			,OSCServer *server=globalArduParOscServer
		#endif
	){
		#ifdef USE_OSC
			AbstractArduPar::setup(cmdString,server);
		#else
			AbstractArduPar::setup(cmdString);
		#endif
		this->callbackFunction=callbackFunction;
	}

  //optional osc support
#ifdef USE_OSC
  void digestMessage(OSCMessage *_mes){
    if(!isOscMessageForMe(_mes))return;
    TRACE((F("Calling ")));
    TRACE((this->cmdString));
    TRACE((F("\n")));
    callbackFunction();
  };
#endif

  ///set the attached integer paramter from a string that was received
  virtual void parseParameterString(char* data){
    TRACE((F("Calling ")));
    TRACE((this->cmdString));
    TRACE((F("\n")));
    callbackFunction();
  };
  /// give human&machine readably status info
  virtual void dumpParameterInfo(Stream* out){
    out->print(F("trigger\t"));
    out->print(this->cmdString);
    out->print(F("\t"));
    out->print(this->cmdString);
    out->print(F("\n"));
  } 
};


////////////////////
/// a setting for string values with optional eeprom persistency
class StringArduPar: 
public AbstractArduPar{
public:
  char* valuePointer;   ///< points to the parameter value to be set
  int eepromAdress;    ///< an adress in eeprom memory for permanent storage of the parameter value. When <0 no storage will happen
  int maxLength;        ///< maximum Length the buffers can hold

 /// Initialize the setting. Has to be called for the setting to become usable.
  void setup(
	const __FlashStringHelper* cmdString,	///< Osc Adress of the setting
	char* valuePointer,	///< points to the buffer where incoming strings will be written
  	int maxLength,          ///< maximum number of chars the buffer can hold
	boolean isPersistent=true,      ///< should the parameter value be initialized from eeprom on startup?
	int fixedEEPROMAdress=-1		///< if you want a specific fixed adress, specify it here
	#ifdef USE_OSC
		,OSCServer *server=globalArduParOscServer
	#endif
	){

  	#ifdef USE_OSC
		AbstractArduPar::setup(cmdString,server);
	#else
		AbstractArduPar::setup(cmdString);
	#endif
	this->valuePointer=valuePointer;
	this->maxLength=maxLength;
	//setup reading and writing from eeprom
	if(isPersistent){
		//get a generated location for persistency if none was specified
		if(fixedEEPROMAdress==-1)fixedEEPROMAdress=EepromAdressManager::getAdressFor(maxLength);
		this->eepromAdress=fixedEEPROMAdress;
		TRACE((F("Init from EEPROM. Adress: ")));
		TRACE((int)(eepromAdress));
		eeprom_read_block(valuePointer,(void *) eepromAdress,maxLength);
		valuePointer[maxLength-1]=0;	//make sure the string is zero terminated even if we read some garbage from eeprom
		TRACE((F(" value:")));
		TRACE((valuePointer));
		TRACE(F("\n"));
	}else{
		this->eepromAdress=-1;
	}
  };

  //optional osc support
#ifdef USE_OSC
  void digestMessage(OSCMessage *_mes){
     if(!isOscMessageForMe(_mes))return;
    if(_mes->getArgTypeTag(0)=='s'){
      char* messageString=_mes->getArgStringData(0);
      int copyLength=min(maxLength,_mes->getArgStringSize(0));
      strncpy(valuePointer,messageString,copyLength);
      valuePointer[maxLength-1]=0;  //make sure the string is terminated
      saveValue();
     }

  };
#endif

  ///set the string parameter from the remaining data. one leading character after the command name is skipped
  virtual void parseParameterString(char* data){
    int dataLength=strlen(data);
    if(dataLength>1){
      //skip first char.
      data++;dataLength--;
      
      int copyLength=min(maxLength-1,dataLength);
      strncpy(valuePointer,data,copyLength);
      valuePointer[copyLength]=0;  //make sure the string is terminated
      saveValue();
    }
  };

  //set the value and rpint some debug info
  void saveValue(){
    TRACE((F("Setting ")));
    TRACE((this->cmdString));
    TRACE((F(" to ")));
    TRACE((valuePointer));
    TRACE((F("\n")));
	//save the new value
	if(eepromAdress>=0){
		TRACE((F("Writing EEProm adress")));
		TRACE(eepromAdress);
		TRACE((F("\n")));
		eeprom_write_block(valuePointer,(void *) eepromAdress,maxLength); 
	}
  };
  /// give human&machine readably status info
  virtual void dumpParameterInfo(Stream* out){
    out->print(F("string\t"));
    out->print(this->cmdString);
    out->print(F("\t"));
    out->print(this->cmdString);
    out->print(F("\t"));
    out->print(valuePointer);
    out->print(F("\n"));
  } 
};



///this function automatically distributes incoming data from a stream to the parameter setting instances
void updateParametersFromStream(Stream* inStream, int timeout){
  if(!inStream->available())return;  //if there is no data available, dont even start timeouts
  //read data from stream
  char buf[PAR_SETTINGS_BUFFER_SIZE];
  int bufPos=0;
  int inByte;
  long lastCharMillis=millis();
  //even if the buffer is full, we keep on reading to flush the remaining data.
  while((millis()-lastCharMillis)<timeout){
    inByte=inStream->read();
    if(inByte>0){
      lastCharMillis=millis();
      if(bufPos<PAR_SETTINGS_BUFFER_SIZE-1){
        buf[bufPos]=inByte;
        bufPos++;
      }
    }
  }

  buf[bufPos]=0;

  //let all the parameter setting instances have a look at the data
  if(bufPos>0){
    for(int i=0;i<PAR_SETTINGS_CUR_INSTANCE_NUMBER;i++){
      PAR_SETTINGS_INSTANCES[i]->parseSerialData(buf);
    } 
  }
};

/// write information about all parameter instances to a stream
void dumpParameterInfos(Stream* outStream){
  for(int i=0;i<PAR_SETTINGS_CUR_INSTANCE_NUMBER;i++){
    PAR_SETTINGS_INSTANCES[i]->dumpParameterInfo(outStream);
  } 
};

/// write information about all parameter instances to a stream
void dumpParameterInfos(){
  dumpParameterInfos(&Serial);
};


////////////////////
/// a setting for integer values with optional eeprom persistency
class LongArduPar: 
public AbstractArduPar{
public:
  long value;			///< will hold the value if no valuePointer was specified in setup
  long* valuePointer;   ///< points to the parameter value to be set
  int eepromAdress;    ///< an adress in eeprom memory for permanent storage of the parameter value. When <0 no storage will happen
  long minValue;        ///< lower Bound of the Parameter. Received values are constrained to be in the Range of [minValue,maxValue]
  long maxValue;        ///< upper Bound of the Parameter. Received values are constrained to be in the Range of [minValue,maxValue]

  void setup(
	  const __FlashStringHelper* cmdString,
	  long minValue,
	  long maxValue,
	boolean isPersistent=true,      ///< should the parameter value be initialized from eeprom on startup?
    long* valuePointer=0,			///< the setting can modify an arbitrary location im memory if you give it here. 
	int fixedEEPROMAdress=-1		///< if you want a specific fixed adress, specify it here
	#ifdef USE_OSC
	  ,OSCServer *server=globalArduParOscServer
	#endif
  ){
  if(valuePointer==0)valuePointer=&this->value;
	this->valuePointer=valuePointer;
	this->minValue=minValue;
	this->maxValue=maxValue;
	#ifdef USE_OSC
		AbstractArduPar::setup(cmdString,server);
	#else
		AbstractArduPar::setup(cmdString);
	#endif
	if(isPersistent){
		if(fixedEEPROMAdress==-1)fixedEEPROMAdress=EepromAdressManager::getAdressFor(sizeof(long));
		this->eepromAdress=fixedEEPROMAdress;
		TRACE((F("Init from EEPROM. Adress: ")));
		TRACE((int)(eepromAdress));
		eeprom_read_block(valuePointer,(void *) eepromAdress,4); 
		TRACE((F(" value:")));
		TRACELN((*valuePointer));
	}else{
		this->eepromAdress=-1; // used to signal non-persistence to other methods
	};	
  };

  //optional osc support
#ifdef USE_OSC
  void digestMessage(OSCMessage *_mes){
    if(!isOscMessageForMe(_mes))return;
    long newValue;
    if(_mes->getArgTypeTag(0)=='i'){
      newValue =  _mes->getArgInt32(0);
    }
    else{
      if(_mes->getArgTypeTag(0)=='f'){ 
        newValue = (long)_mes->getArgFloat(0);
      }    
      else{
        return;
      }
    }
    setValue(newValue);
  };
#endif

  ///set the attached integer parameter from a string that was received
  virtual void parseParameterString(char* data){
    long newValue=atol(data);
    setValue(newValue);
  };

  //set the value and rpint some debug info
  void setValue(long newValue){
	newValue=constrain(newValue,minValue,maxValue);
    TRACE((F("Setting ")));
    TRACE((this->cmdString));
    TRACE((F(" to ")));
    TRACE((newValue));
    TRACE((F("\n")));
    *valuePointer=newValue;
    //save the new value
    if(eepromAdress>=0){
      TRACE((F("Writing EEProm adress")));
      TRACE(eepromAdress);
      TRACE((F("\n")));
      eeprom_write_block(valuePointer,(void *) eepromAdress,4); 

    }
  };
  /// give human&machine readably status info
  virtual void dumpParameterInfo(Stream* out=&Serial){
    out->print(F("int\t"));
    out->print(this->cmdString);
    out->print(F("\t"));
    out->print(this->cmdString);
    out->print(F("\t"));
    out->print(*valuePointer);
    out->print(F("\t"));
    out->print(minValue);
    out->print(F("\t"));
    out->print(maxValue);
    out->print(F("\n"));
  } 
};

////////////////////
/// a setting for integer values with optional eeprom persistency
class FloatArduPar: 
public AbstractArduPar{
public:
  float value;
  float* valuePointer;   ///< points to the parameter value to be set
  int eepromAdress;    ///< an adress in eeprom memory for permanent storage of the parameter value. When <0 no storage will happen
  float minValue;        ///< lower Bound of the Parameter. Received values are constrained to be in the Range of [minValue,maxValue]
  float maxValue;        ///< upper Bound of the Parameter. Received values are constrained to be in the Range of [minValue,maxValue]

  ///simplest possible constructor, will use own value and no persistency
  FloatArduPar():
  value(0),
  valuePointer(&this->value)
  {
  }
/// set up the setting. has to be called to make it functional
  void setup(
	  const __FlashStringHelper* cmdString,
	  float minValue,
	  float maxValue,
	  boolean isPersistent=true,      ///< should the parameter value be initialized from eeprom on startup?
    float* valuePointer=0,			///< the setting can modify an arbitrary location im memory if you give it here. 
	int fixedEEPROMAdress=-1		///< if you want a specific fixed adress, specify it here
	#ifdef USE_OSC
	  ,OSCServer *server=globalArduParOscServer
	#endif
  ){
  if(valuePointer==0)valuePointer=&this->value;
	this->valuePointer=valuePointer;
	this->minValue=minValue;
	this->maxValue=maxValue;
	#ifdef USE_OSC
		AbstractArduPar::setup(cmdString,server);
	#else
		AbstractArduPar::setup(cmdString);
	#endif
	if(isPersistent){
	
		if(fixedEEPROMAdress==-1){TRACE((F("Getting EEPROM. Adress: ")));fixedEEPROMAdress=EepromAdressManager::getAdressFor(sizeof(float));};
		this->eepromAdress=fixedEEPROMAdress;
		TRACE((F("Init from EEPROM. Adress: ")));
		TRACE((int)(eepromAdress));
		eeprom_read_block(valuePointer,(void *) eepromAdress,sizeof(float)); 
		TRACE((F(" value:")));
		TRACELN((*valuePointer));
	}else{
		this->eepromAdress=-1; // used to signal non-persistence to other methods
	};	
  };

  //optional osc support
#ifdef USE_OSC
  void digestMessage(OSCMessage *_mes){
    if(!isOscMessageForMe(_mes))return;
    float newValue;
    if(_mes->getArgTypeTag(0)=='i'){
      newValue =  (float)_mes->getArgInt32(0);
    }
    else{
      if(_mes->getArgTypeTag(0)=='f'){ 
        newValue = _mes->getArgFloat(0);
      }    
      else{
        return;
      }
    }
    setValue(newValue);
  };
#endif

  ///set the attached integer parameter from a string that was received
  virtual void parseParameterString(char* data){
    setValue(atof(data));
  };

  //set the value and rpint some debug info
  void setValue(float newValue){
    newValue=constrain(newValue,minValue,maxValue);
    TRACE((F("Setting ")));
    TRACE((this->cmdString));
    TRACE((F(" to ")));
    TRACE((newValue));
    TRACE((F("\n")));
    *valuePointer=newValue;
    //save the new value
    if(eepromAdress>=0){
      TRACE((F("Writing EEProm adress")));
      TRACE(eepromAdress);
      TRACE((F("\n")));
      eeprom_write_block(valuePointer,(void *) eepromAdress,sizeof(float)); 

    }
  };
  /// give human&machine readably status info
  virtual void dumpParameterInfo(Stream* out){
    out->print(F("float\t"));
    out->print(this->cmdString);
    out->print(F("\t"));
    out->print(this->cmdString);
    out->print(F("\t"));
    out->print(*valuePointer);
    out->print(F("\t"));
    out->print(minValue);
    out->print(F("\t"));
    out->print(maxValue);
    out->print(F("\n"));
  } 
};




