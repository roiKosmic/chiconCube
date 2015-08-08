#ifndef CHICON_H_   /* Include guard */
#define CHICON_H_

#define LTYPE_BINARY       1
#define LTYPE_TRICOLOR     2
#define LTYPE_FADDING      4
#define LTYPE_BLINKING     8
#define LTYPE_BRIGHTNESS   16

#define LHWTYPE_FADDING      31
#define LHWTYPE_TRICOLOR     11
#define LHWTYPE_BINARY       25 

typedef struct {
  unsigned long id;
  short nbrLed;
  led** ledTab;
  unsigned long frequency;
  unsigned long lastCheck;
} service;



#endif
