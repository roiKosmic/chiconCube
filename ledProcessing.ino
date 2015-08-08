boolean initLED(){
   //LED Group 0 definition
  
    myLeds[0].id = 1;
    myLeds[0].hw_type = LHWTYPE_FADDING;
    
    //Second LED Group definition
    myLeds[1].id = 2;
    myLeds[1].hw_type = LHWTYPE_FADDING;
    
    //Third LED Group definition
    myLeds[2].id = 3;
    myLeds[2].hw_type = LHWTYPE_FADDING;
    
    return true;
  
}

void ledSelected(led* sLed){
  ShiftPWM.SetAll(0);
  ShiftPWM.SetRGB(sLed->id-1,maxBrightness,maxBrightness,maxBrightness);
  
}
/*
boolean printLedDetails(led* myLed){
 unsigned short ledType;
 unsigned short ledId;
 ledType = myLed->srv_type;
 ledId = myLed->id;
 Serial.print(F("LED ID :"));
 Serial.println(ledId);
 Serial.print(F("\tValue : "));
 switch(ledType){
  case LTYPE_BINARY:
    Serial.println(myLed->value[0]);
    return true;
    break;
  case LTYPE_TRICOLOR:
      Serial.println(myLed->value[0]);
    return true;
  case LTYPE_FADDING: 
    Serial.print(myLed->value[0]);
    Serial.print(F(","));
   Serial.print(myLed->value[1]);
     Serial.print(F(","));
    Serial.println(myLed->value[2]);
    return true;
   case LTYPE_BLINKING:
    Serial.println(myLed->value[0]);
    return true; 
    
  default:
    return false;
 } 
  
  
}
*/

boolean setLEDBlinking(led* led_){
 if(led_->lastBlink == 0) led_-> lastBlink = millis();
 if(millis() - led_->lastBlink > led_->value[0] && led_->value[0] !=0){
    led_->lastBlink = millis();  
    ShiftPWM.SetRGB(led_->id-1,led_->value[1]*maxBrightness,led_->value[1]*maxBrightness,led_->value[1]*maxBrightness);
    led_->value[1] = !led_->value[1];    
 }else if(led_->value[0] ==0){
   ShiftPWM.SetRGB(led_->id-1,0,0,0);  
 }
   return true; 
}

boolean setLEDTricolor(led* led_){
  
  int ledHwType;
  int value;
  ledHwType = led_->hw_type;
  value = led_->value[0];
   switch(value){
           case 0: //OFF
             ShiftPWM.SetRGB(led_->id-1,0,0,0);
           break;
           case 1: //GREEN
               ShiftPWM.SetRGB(led_->id-1,0,maxBrightness,0);       
           break;
           case 2: //ORANGE 
               ShiftPWM.SetRGB(led_->id-1,255,128,0);        
           break;
           case 3: //RED
              ShiftPWM.SetRGB(led_->id-1,maxBrightness,0,0);
             break;
           
     }
    return true;
  
}

boolean resetLED(){
 short i,j;
 for(i=0;i<MAXLED;i++){
   myLeds[i].lastBlink = 0;
   for(j=0;j<4;j++){
     myLeds[i].value[j] = 0;
   }
 }
 ShiftPWM.SetAll(0);
 return true;
}


boolean setLEDValues(led * led_){
 unsigned short ledSrvType;
 unsigned short ledHwType;
 short value;
 
 ledSrvType = led_->srv_type;
 ledHwType = led_->hw_type;
 
 switch(ledSrvType){
    case LTYPE_BINARY:
       value = led_->value[0];
       if(value==0) value = 0;
       if(value==1) value = maxBrightness;
       ShiftPWM.SetRGB(led_->id-1,value,value,value);
     break;
     case LTYPE_TRICOLOR:
        setLEDTricolor(led_);
     break;
     case LTYPE_FADDING:
        ShiftPWM.SetRGB(led_->id-1,led_->value[0],led_->value[1],led_->value[2]);
     break;
     case LTYPE_BLINKING:
       setLEDBlinking(led_);
     break;
     case LTYPE_BRIGHTNESS:
       ShiftPWM.SetRGB(led_->id-1,led_->value[0],led_->value[0],led_->value[0]);
     break;     
   }
   
   return true;  
 }
 

