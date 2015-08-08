boolean connectToWifiNetwork(){
  
  digitalWrite(SS_ETHERNET, LOW); //Enable WIFI
  
  if (WiFi.status() == WL_NO_SHIELD) {
    return false;
  } 
  while (status != WL_CONNECTED) {     
    status = WiFi.begin((char*)ssid, pass);
    delay(10000);
  }
  
  return true;
}

boolean isWifiClientAvailable(){
  if(socketTimer == 0){
     return true; 
  }
  return false;
}

void forceSocketClose(){
  socketTimer = 0;
  client.flush();
  client.stop();

}

void updateSocketTimer(){
  
   if((millis() - socketTimer) > SOCKET_TIMER && socketTimer > 0){
      Serial.println(F("Socket TimeOut"));
      socketTimer = 0;
      client.flush();
      client.stop();
   } 
}

void createClientRequest(String query){
  client.print("GET "+WEBSERVICE_URL);
  client.print(query);
  client.print(HTTP_REQUEST);
  checkServerResponse();
  
}

boolean configRequestOverWifi(String mn){
 if(isWifiClientAvailable()){
  if(!client.connected()){
     if (client.connect(server, 80)) {
        socketTimer = millis();  
        String q= "?mn="+mn+"&cfg";
        createClientRequest(q);
     }
   }
 }
  return false;
}

boolean selectLEDOverWifi(String mn, unsigned short id){
  if(isWifiClientAvailable()){
  if(!client.connected()){ 
     if (client.connect(server, 80)) {
        socketTimer = millis(); 
       String q= "?mn="+mn+"&led="+String(id);
        createClientRequest(q);
        return true;
    } 
   }
  }
  return false;
}


boolean executeServiceOverWifi(String mn,unsigned long id){
 if(isWifiClientAvailable()){
  if(!client.connected()){ 
 if (client.connect(server, 80)) {
    socketTimer = millis();
    String q ="?mn="+mn+"&srv="+String(id);
    createClientRequest(q);
    return true;
  } 
 }
 }
  return false;
}

boolean checkServerResponse(){
  int connectLoop = 0;
  boolean dataTr = false;
  //while(connectLoop <= 10000 && dataTr==false){
  while(client.connected() && dataTr==false){  
    while(client.available()){
        dataTr = true;
        char c = client.read();
        connectLoop = 0;
        //Body part of HTTP response start
        #if defined(WITH_SERIAL)                 
          Serial.print(c);
        #endif 
        if(body == 1 && c !='\r' && c!='\n'){
          //Check if it is JSON
          if(c == '{'){
            jsonStarted = true;
           }
           if (jsonStarted==true){
             buf[strInd] = c;
             strInd++;
             //Ajouter le controle de l'overflow
             if(strInd >= RCV_BUFF_SIZE){
                #if defined(WITH_SERIAL)                      
                 Serial.println(F("Buf Overflow"));
                #endif 
                jsonStarted = false;
                stringComplete = false;
                strInd = 0;
                body = 0;
                forceSocketClose();   
                return false; 
             }
           } 
          }
       //Flag telling we enter body part of HTTP response
       if(t=='\n' && c=='\r'){
          body = 1;
       }
       t = c;
     }
     connectLoop++;
     if(connectLoop > 10000)
    {
      // then close the connection from this end.
      #if defined(WITH_SERIAL)
      Serial.println(F("Server Timeout"));
      #endif
      strInd = 0;
      jsonStarted = false;
      stringComplete = false;
       body = 0;
     forceSocketClose();
      return false;
    }
   //delay(1);
  }
   
   //Last '\n' is end of JSON
   if (jsonStarted==true){
     buf[strInd] = '\0';
     strInd = 0;
     stringComplete = true;
     jsonStarted = false;
     body = 0;
     forceSocketClose();
     return true;
   }
     
         
   #if defined(WITH_SERIAL)
   Serial.println(F("Server Disconnected"));
   #endif
   jsonStarted = false;
   stringComplete = false;
   strInd = 0;
   body = 0;
   forceSocketClose();
   return false;
}
