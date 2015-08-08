#ifndef LEDPROCESSING_H_   /* Include guard */
#define LEDPROCESSING_H_


typedef struct {
  unsigned short id;
  unsigned short hw_type;
  unsigned short srv_type;
  short value[4];
  unsigned long lastBlink;
} led;


#endif
