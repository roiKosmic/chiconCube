
/***********************************************************
//    chiconCube_v0_1a.ino
//    version 0.1a - 07/08/2015
//    Author : Vincent ANTOINE
//    
//    MAIN FILE of CHICON HDW Management
/**********************************************************/


#define WITH_SERIAL 1
#include <SPI.h>

#include <MemoryFree.h>
#include <WiFi.h>
#include <WiFiClient.h>

#include "ledprocessing.h"
#include "chicon.h"
#include "wifiManager.h"
#include "config.h"

#define SS_SD_CARD   4
#define SS_ETHERNET 10
#define FIRST_CONFIG_FREQUENCY 5000
#define MAXLED 3

#define STATUS_LED_PIN 6
#define ERROR_LED_PIN 14
#define RCV_BUFF_SIZE 400
// You can choose the latch pin yourself.
const int ShiftPWM_latchPin=8;

// ** uncomment this part to NOT use the SPI port and change the pin numbers. This is 2.5x slower **
#define SHIFTPWM_NOSPI
const int ShiftPWM_dataPin = 3;
const int ShiftPWM_clockPin = 5;


// If your LED's turn on if the pin is low, set this to true, otherwise set it to false.
const bool ShiftPWM_invertOutputs = false; 

// You can enable the option below to shift the PWM phase of each shift register by 8 compared to the previous.
// This will slightly increase the interrupt load, but will prevent all PWM signals from becoming high at the same time.
// This will be a bit easier on your power supply, because the current peaks are distributed.
const bool ShiftPWM_balanceLoad = false;
#include <ShiftPWM.h>


unsigned char maxBrightness = 255;
unsigned char pwmFrequency = 75;
byte numRegisters = 2;
//int numRGBleds = 3;
unsigned long lastConfigCheck = 0;
char buf[RCV_BUFF_SIZE];
boolean stringComplete = false;
//short strInd=0;
service* myServices;
led myLeds[MAXLED];
short nbrConfigService = -1;
boolean jsonStarted;
boolean enrollMe = false;


volatile boolean isALedSelected =false;
volatile boolean isALedSelectedConfirmed =false;
volatile short selectedLEDID = 0;
volatile short selectionTime = 4000;
volatile short preventDoubleClickTimer = 500;
volatile boolean doubleClickDetected = false;
volatile unsigned long last_selectedLed_time = 0;

/***********************************************************
//    Interrupt function executed each time button is pressed
//    if button is pressed before selectionTime value, a request is sent to the server for action
//    if not, selectedLEDID is incremented (next LED selected).
//    In LEDSelection process
//      Updated global variable:
//        selectedLEDID : Current selected LED (incr each time function is called
//        last_selectedLed_time : Current time when button is pressed
//        isALedSelected set to true to indicate to loop function button has been pressed and a LED is currently selected
//    When a LED has been selected and selection confimed by a second button push with selectionTime
//      Updated global variable:
//        isALedSelected set to false to indicate to loop function selection process has ended
//        last_selectedLed_time set to 0 to allow a next selection process
//        
/**********************************************************/

void selectLED(){
  if(!doubleClickDetected){
    doubleClickDetected=true;
    if(last_selectedLed_time == 0){
      if(selectedLEDID==MAXLED){
        selectedLEDID = 0; 
      }
      selectedLEDID++;
      #if defined(WITH_SERIAL)
        Serial.print(F("Led Selected"));
        Serial.println(selectedLEDID);
      #endif
      isALedSelected = true;
    }else{
      //On a rappuyer dans le temps de la selection on balance la requête de selection
      isALedSelectedConfirmed = true;
      manageBuiltinStatusLed(false); //on met le status led en vert pour indiquer qu'on est plus en mode sélection
      last_selectedLed_time =0;
    }
  }
}

/***********************************************************
//    processLEDValues
//    This function goes through all configured services
//    For each service it goes trough each LED and call the setLEDValues(led*) to light on the LED according to received values  
/**********************************************************/

boolean processLEDValues(){
 byte  i;
 byte j;
 led * currLed;
 for(i=0;i<nbrConfigService;i++){
    for(j=0;j<myServices[i].nbrLed;j++){
       currLed = myServices[i].ledTab[j];
       setLEDValues(currLed);
    }
 } 
  
 return true; 
}

/***********************************************************
//    getLedByID
//    This function goes through the myLeds array and return the led with id id_
//    input : id of the searched led
//    return : search led or NULL if not found
/**********************************************************/

led* getLedById(int id_){
  byte i;
  for(i=0;i<MAXLED;i++){
   if(myLeds[i].id ==id_){
      return &myLeds[i]; 
   }
  }
   return NULL;
}

service* getServiceById(unsigned long id_){
  byte i;
  for(i=0;i<nbrConfigService;i++){
   if(myServices[i].id ==id_){
      return &myServices[i]; 
   }
  }
   return NULL;
  
}
void manageBuiltinStatusLed(boolean error){
  digitalWrite(STATUS_LED_PIN,!error);
  digitalWrite(ERROR_LED_PIN,error);
}

void setup() {
     //#if defined(WITH_SERIAL)
       Serial.begin(9600);
       Serial.print(F("Booting Firmware 0.1a\n"));
   // #endif
    //Shift PWM init
    ShiftPWM.SetAmountOfRegisters(numRegisters);
    //ShiftPWM.SetPinGrouping(1);
    ShiftPWM.Start(pwmFrequency,maxBrightness);
    
    //Led Init
    initLED();
    
    //Error & Status LED init
    pinMode(STATUS_LED_PIN,OUTPUT);
    pinMode(ERROR_LED_PIN,OUTPUT);
   manageBuiltinStatusLed(true);
    
    //Init select button
    pinMode(2,INPUT);
    attachInterrupt(0, selectLED, RISING);
    
   //init shield  
    pinMode(SS_ETHERNET, OUTPUT);
    digitalWrite(SS_ETHERNET, HIGH); // Wifi Not active
   
    while(!connectToWifiNetwork()); //if no connection loop trying to connect
    manageBuiltinStatusLed(false);
    Serial.print(F("freeMemory()="));
    Serial.println(freeMemory());
}


void checkServiceTimer(){
  byte i=0;
    for(i=0;i<nbrConfigService;i++){
        if((millis() - myServices[i].lastCheck > myServices[i].frequency) ){
          myServices[i].lastCheck = millis();
          #if defined(WITH_SERIAL)
          Serial.print(F("@Srv ID "));
          Serial.println(myServices[i].id);
          #endif
          executeServiceOverWifi(magicNumber,myServices[i].id);
          processServerResponse();
          
        }
    }
  
}

boolean checkServiceConfig(){
   if(nbrConfigService == -1 && (millis() - lastConfigCheck > FIRST_CONFIG_FREQUENCY)){
     #if defined(WITH_SERIAL)
     Serial.println(F("@First Config Request"));
     #endif
     configRequestOverWifi(magicNumber);
     processServerResponse();
     lastConfigCheck = millis();
   }
   //else if(millis() - lastConfigCheck > CONFIG_UPDATE_FREQUENCY){
     // Serial.println("@Config Update Request");
      //lastConfigCheck = millis();
   //} 
   
   return true;
}



void loop() {
    if(!isALedSelected && !isALedSelectedConfirmed){
      checkServiceConfig();
      checkServiceTimer();   
      processLEDValues();
      updateSocketTimer();
    }
    checkLedSelection();
   
    
     
}

void checkLedSelection(){
   if(isALedSelected){
      manageBuiltinStatusLed(true); //On met le status LED en rouge afin d'indiquer qu'on est en mode sélection
      if(millis() - last_selectedLed_time > preventDoubleClickTimer && last_selectedLed_time!=0 ){
         doubleClickDetected = false; 
      }
      if(last_selectedLed_time==0){ //On est en mode selection
         ledSelected(getLedById(selectedLEDID));
         last_selectedLed_time=millis(); 
      }

      if(isALedSelectedConfirmed){ //On a confirmer la selection de la LED
      #if defined(WITH_SERIAL)
        Serial.print("LS-confirmed");
        Serial.println(selectedLEDID);
      #endif
        isALedSelected = false;
        last_selectedLed_time = 0;
        isALedSelectedConfirmed = false;
        manageBuiltinStatusLed(false);
        forceSocketClose();
        selectLEDOverWifi(magicNumber,selectedLEDID);
		delay(preventDoubleClickTimer);
        processServerResponse();
        doubleClickDetected = false; 
      }
     
      if(millis() - last_selectedLed_time > selectionTime && last_selectedLed_time!=0){
        //L'utilisateur n'a pas appuyé dans le temps imparti
        isALedSelected = false;
        last_selectedLed_time = 0;
        manageBuiltinStatusLed(false); //On met le status LED à vert afin d'indiquer qu'on est plus en mode sélection
        doubleClickDetected = false; 
      }
    }
}

void processServerResponse(){
    if (stringComplete) {
        //Serial.print("JSON Processing  ");
        //Serial.println(buf);
       if(buf[0]=='{'){
          #if defined(WITH_SERIAL)
          Serial.print(F("\nfreeMemory()="));
          Serial.println(freeMemory());
          Serial.print("JSON Processing  ");
          Serial.println(buf);
          #endif
          parseJson(buf);
        }
     stringComplete = false;  
    }
  
}

int parseJson(char * jsonString){
 unsigned short i=0;
 unsigned short j=0;
 char* srvPtr;
 char* objPtr;
 char * eltPtr;
 char * eltValuePtr;
 char * ledPtr;
 char * ledArrayPtr;
 char * ledEltPtr;
 char * ledEltObjPtr;
 char * ledEltObjValuePtr;
 char * ledValueArrayPtr;
 char * ledValueEltPtr;
 unsigned short srvNbr=0;
 unsigned short ledNbr = 0;
 unsigned short idLed;
 unsigned long idSrv;
 unsigned short srvTypeLed;
 led* currLed;
 service* currService;
 int* ledValue;
 int nbrValue;
 int value;
 

 
 srvPtr = getJsonObjectElement(jsonString,"srvconfig");
 
 
 if(srvPtr !=NULL){
 //We are receiving a new configuration string 
   if(nbrConfigService != -1){
     for(i=0;i<nbrConfigService;i++){
       free(myServices[i].ledTab);
     }
     free(myServices);
   }
 
   //Pause the checkService.
   nbrConfigService = 0;
   //Reset leds
   resetLED();  
   srvPtr = getJsonObjectEltValue(srvPtr);
   srvNbr = getJsonArrayNbrElt(srvPtr);

   myServices = (service*)malloc(srvNbr*sizeof(service)); 
   for(i=0;i<srvNbr;i++){
     eltPtr = getJsonArrayElement(srvPtr,i);
     if(eltPtr !=NULL){
      objPtr = getJsonObjectElement(eltPtr,"id");
      if(objPtr != NULL){
         eltValuePtr = getJsonObjectEltValue(objPtr);
         setJsonLongValue(eltValuePtr,&(myServices[i].id));      
      }    
       objPtr = getJsonObjectElement(eltPtr,"freq");
       if(objPtr != NULL){
         eltValuePtr = getJsonObjectEltValue(objPtr);
         setJsonLongValue(eltValuePtr,&(myServices[i].frequency));
       }
       ledPtr = getJsonObjectElement(eltPtr,"led");
       ledArrayPtr = getJsonObjectEltValue(ledPtr);
       if(objPtr != NULL){
         ledNbr = getJsonArrayNbrElt(ledArrayPtr);
         Serial.print("LedNbr:");
         Serial.println(ledNbr);
         myServices[i].nbrLed = ledNbr;
         myServices[i].ledTab = (led**)(malloc(ledNbr*sizeof(led*)));
         for(j=0;j<ledNbr;j++){
           ledEltPtr = getJsonArrayElement(ledArrayPtr,j);
           if(ledEltPtr != NULL){
             ledEltObjPtr = getJsonObjectElement(ledEltPtr,"id");
             
             
             ledEltObjValuePtr = getJsonObjectEltValue(ledEltObjPtr);
             setJsonLongValue(ledEltObjValuePtr,(unsigned long*)&idLed);
          
             ledEltObjPtr = getJsonObjectElement(ledEltPtr,"type");
             ledEltObjValuePtr = getJsonObjectEltValue(ledEltObjPtr);
             setJsonLongValue(ledEltObjValuePtr,(unsigned long*)&srvTypeLed);
             currLed = getLedById(idLed);
             if(currLed!=NULL){ 
                 currLed->srv_type = srvTypeLed;
                myServices[i].ledTab[j] = currLed;
             }
           } 
         }
         
       }
       
       #if defined(WITH_SERIAL)
       Serial.print("New service configured ");
         Serial.print("id ");
         Serial.print(myServices[i].id);
         Serial.print(" - freq ");
         Serial.println(myServices[i].frequency);
         Serial.print("with ");
         Serial.print(myServices[i].nbrLed);
         Serial.println(" LEDs configured");
         for(j=0;j<myServices[i].nbrLed;j++){
           Serial.print("\tLED id ");
           Serial.print(myServices[i].ledTab[j]->id);
           Serial.print(" - Type  ");
           Serial.println(myServices[i].ledTab[j]->srv_type);
         }
      #endif         
        
     }
  
   }
   nbrConfigService = srvNbr;
   lastConfigCheck = millis();
   return srvNbr;
 }else{
   srvPtr = getJsonObjectElement(jsonString,"srvset");  
   if(srvPtr !=NULL && nbrConfigService > 0){
    //We are receiving a srv set string
    //{"srvset":{"id":12300,"led":[{"id":1,"value":1},{"id":3,"value":[34,26,62]}]}}
    
    
    objPtr = getJsonObjectEltValue(srvPtr);
    eltPtr = getJsonObjectElement(objPtr,"id");
    if(eltPtr != NULL){
      eltValuePtr = getJsonObjectEltValue(eltPtr);
      setJsonLongValue(eltValuePtr,&idSrv);
      currService = getServiceById(idSrv);
      if(currService != NULL){
        ledPtr = getJsonObjectElement(objPtr,"led");
         
         if(ledPtr != NULL){
           ledArrayPtr = getJsonObjectEltValue(ledPtr);
         
            for(i=0;i<currService->nbrLed;i++){
              ledEltPtr = getJsonArrayElement(ledArrayPtr,i);
                 if(ledEltPtr != NULL){
                   ledEltObjPtr = getJsonObjectElement(ledEltPtr,"id");
                   if(ledEltObjPtr != NULL){
                     ledEltObjValuePtr = getJsonObjectEltValue(ledEltObjPtr);
                     setJsonLongValue(ledEltObjValuePtr,(unsigned long*)&idLed);
                     currLed = getLedById(idLed);
                     ledEltObjPtr = getJsonObjectElement(ledEltPtr,"value");
                     if(ledEltObjPtr != NULL){
                       ledValueArrayPtr = getJsonObjectEltValue(ledEltObjPtr);
                       nbrValue = getJsonArrayNbrElt(ledValueArrayPtr);
                       for(j=0;j<nbrValue;j++){
                         ledValueEltPtr = getJsonArrayElement(ledValueArrayPtr,j);
                         setJsonLongValue(ledValueEltPtr,(unsigned long*)&value);
                         currLed->value[j] = value;
                       }
                     }
                    }
                 } 
            }
       /*     
             Serial.print("New service set ");
             Serial.print("id ");
             Serial.println(currService->id);
             Serial.println(" LEDs Value Set");
               for(j=0;j<currService->nbrLed;j++){
                 printLedDetails(currService->ledTab[j]);
               }
         */      
              return 0;
         }
       }
      }
   }
 }
 
 return -1;
}


