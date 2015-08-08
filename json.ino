/***********************************************************
//    json.c
//    version 1 - 24/01/2014
//    Author : Vincent ANTOINE
//    
//    All json operation function are in this file.
//
/**********************************************************/


/********************************************************************************/
//  jsonObjectPtr : a json string representing an array (in json between [])
//  index : Index number int the array
//
// This function returns a pointer at the position of the requested element
// Ex : a = [[{"id":2,"type":1},{"id":3,"type":1}]
//      getJsonArrayElement(a,1)
//      returns : {"id":3,"type":1}]
//
/*********************************************************************************/

char* getJsonArrayElement(char * jsonObjectPtr,int index){
   char currChar = ' ';
   char * ptr;
   int skip =0;
   int currIndex = 0;
 
   ptr = jsonObjectPtr;
   if(ptr[0] != '['){
      //Serial.println(F("Not a valid json Elt Array")); 
      return NULL; 
   }
   if(index >=getJsonArrayNbrElt(jsonObjectPtr)){
     //Serial.println(F("Not a valid index"));
     return NULL;
   }
   
   while(currChar !=']' || skip !=0){
     ptr++;
     currChar = ptr[0];
     if(currChar == ',' &&  skip == 0){
        currIndex++;
     }
     if(currChar == '{' || currChar =='['){
       skip++;
     }
     if(currChar == '}' || currChar == ']'){
      if(skip>0) skip--;
     }
     if(currIndex == index){
        if(index == 0) return ptr;
        return ptr+1;
     }   
 } 
  return NULL;
  
}
/*******************************************************************/
//  jsonPtr : a json string representing an object (in json between {})
//  eltName : name of an element (in json "<eltName>":<eltValue>
//
// This function returns a pointer at the position of the requested element
// Ex : a = {"srv":1,"led":[{"id":2,"type":1},{"id":3,"type":1}],"req":1}
//      getJsonObjectElement(a,"led")
//      returns : "led":[{"id":2,"type":1},{"id":3,"type":1}],"req":1}
//
/*******************************************************************/

char* getJsonObjectElement(char * jsonPtr, char * eltName){
  int i=0;
  int j=0;
  char * ptr;
  char currChar = ' ';
  char * eltNamePtr;
  
  ptr = jsonPtr;
  if(ptr[0] != '{'){
    //Serial.println(F("Not a valid json Object")); 
    return NULL; 
  }
  while(currChar != '}'){
     ptr++;
     currChar = ptr[0];
     if(currChar != '"'){
        //Serial.println(F("Not a valid json Object")); 
        return NULL; 
     }
     //New elt check if it is the object we are looking for.
     eltNamePtr = ptr +1;
     while(eltNamePtr[j]!='"'){
       j++;
     }
     if(memcmp(eltName,eltNamePtr,j)==0){
         return ptr; 
     }
   //  Serial.println("Current Elt ");
    // Serial.println(ptr);
     ptr = nextJsonObjectElt(ptr);
     if(ptr == NULL){
      return NULL; 
     }
     j=0;        
       
 } 
 return NULL;
}
 
/*******************************************************************/
//  jsonPtr : a json string representing an object (in json between {}) or element of an object
//  eltName : name of an element (in json "<eltName>":<eltValue>
//
// This function returns a pointer at the position of next element within the object
// Ex : A given JSON object : {"srv":1,"led":[{"id":2,"type":1},{"id":3,"type":1}],"req":1}
//      if jsonObjectPtr = "led":[[{"id":2,"type":1},{"id":3,"type":1}],"req":1}
//      nextJsonObjectElement(jsonObjectPtr)
//      returns : "req":1}
//
/*******************************************************************/
char* nextJsonObjectElt(char * jsonObjectPtr){
 char currChar = ' ';
 char * ptr;
// int i=0;
 int skip;
 
 ptr = jsonObjectPtr;
 if(ptr[0] != '"'){
  //Serial.println(F("Not a valid json Elt")); 
  return NULL; 
 }
 //Skip Elt Name
 while(currChar != ':'){
     ptr++;
     currChar = ptr[0];
     //Serial.println(ptr);
 }
// i =0;
 currChar = ' ';
 skip = 0;
 //Skip elt value;
 while((currChar != ',' && currChar !='}')  || skip !=0){
     ptr++;
     //Serial.print("Skipping the value - ");
     //Serial.println(skip);
     //Serial.println(ptr);
     currChar = ptr[0];
     if(currChar == '{' || currChar =='['){
       skip++;
     }
     if(currChar == '}' || currChar == ']'){
      if(skip>0) skip--;
     }  
 }
 
 if(currChar == '}'){
   //no next Elt
   return NULL; 
 }
 //Serial.println("Next Elt : ");
 //Serial.println(ptr);
 return ptr;
} 

/*******************************************************************/
// jsonObjPtr : a json string pointing at the begeinning of a JSON  value element "<eltName>":<eltValue>
//                                                                                        ---^
// This function set myValue to eltValue converting JSONstring to long
// Ex : A given JSON object : {"srv":1,"led":[{"id":2,"type":1},{"id":3,"type":1}],"req":1}
//      if jsonObjectPtr = 1,"led":[{"id":2,"type":1},{"id":3,"type":1}],"req":1}
//      setJsonLongValue will set myValue with 1 (srv element value)
//      returns : true
//
/*******************************************************************/

boolean setJsonLongValue(char *jsonObjPtr,unsigned long* myValue){
  char currChar=' ';
  char strL[12];
  int i=0;
  while(currChar!=',' && currChar!='}' && currChar!=']'){
       currChar=jsonObjPtr[i];
       i++; 
   }
   memcpy(strL,jsonObjPtr,i-1);
   strL[i-1]='\0';
   *myValue = atol(strL);
   return true;
}

/*******************************************************************/
//  jsonObjPtr : a json string pointing at the begeinning of a JSON  value element "<eltName>":<eltValue>
//                                                                                          ---^
// This function return eltValue as a string
// Ex : A given JSON object : {"mn":"ax45vz3263"}
//      if jsonObjectPtr = "ax45vz3263"}
//      getJsonStringValue will return ax45vz3263 as a string object
//      returns : String
//
/*******************************************************************/

String getJsonStringValue(char *jsonObjPtr){
  char currChar=' ';
  int i=1;
  char * ptr = jsonObjPtr;
  if(ptr[0] != '"'){
    //Serial.println(F("Not a valid String value")); 
    return NULL; 
  }
  
  while(currChar!='"'){
       currChar=jsonObjPtr[i];
       i++; 
   }
   String sValue = jsonObjPtr;
   return sValue.substring(1,i-1);
}

/*******************************************************************/
//  jsonObjPtr : a json string pointing at the begeinning of a JSON  object element "<eltName>":<eltValue>
//                                                                               ---^
// This function return a pointer at eltValue
// Ex : A given JSON object : {"srv":1,"led":[{"id":2,"type":1},{"id":3,"type":1}],"req":1}
//      if jsonObjectPtr = "req":1}
//      getJsonObjectEltValue will return a pointer to 1}
//      returns : pointer
//
/*******************************************************************/

char * getJsonObjectEltValue(char* jsonObjectPtr){
  char currChar = ' ';
  char * ptr;
  
  ptr = jsonObjectPtr;
   if(ptr[0] != '"'){
    //Serial.println(F("Not a valid json Elt")); 
    return NULL; 
   }
   //Skip Elt Name
   while(currChar != ':'){
     ptr++;
     currChar = ptr[0];
     //Serial.println(ptr);
   }
  return ptr+1;
}

/*******************************************************************/
//  jsonObjPtr : a json string pointing at the begeinning of a JSON  array ie between []
//                                                                               
// This function return number of element in the given array
// Ex : A given JSON object : {"srv":1,"led":[{"id":2,"type":1},{"id":3,"type":1}],"req":1}
//      if jsonObjectPtr = [{"id":2,"type":1},{"id":3,"type":1}],"req":1}
//      getJsonArrayNbrElt will return 2
//      returns : integer whose value is number of element
//
/*******************************************************************/

int getJsonArrayNbrElt(char* jsonObjectPtr){
 char currChar = ' ';
 char * ptr;
 int skip;
 int numElt = 1;
 
 ptr = jsonObjectPtr;
 if(ptr[0] != '['){
  //Serial.println(F("Not a valid json Elt Array")); 
  return 0; 
 }
 
 skip = 0;
 //Skip elt value;
 while(currChar !=']' || skip !=0){
     ptr++;
     //Serial.print("Skipping the value - ");
     //Serial.println(skip);
     //Serial.println(ptr);
     currChar = ptr[0];
     if(currChar == ',' &&  skip == 0){
      numElt++; 
     }
     if(currChar == '{' || currChar =='['){
       skip++;
     }
     if(currChar == '}' || currChar == ']'){
      if(skip>0) skip--;
     }  
 } 
 return numElt; 
}

