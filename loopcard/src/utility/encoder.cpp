#include "encoder.h"

// set up encoder object
MD_REncoder R = MD_REncoder(M_A, M_B);

void encoder_init() 
{
  R.begin();
}

void encoder_read() 
{
  uint8_t x = R.read();
  
  if (x) 
  {
    Serial.print(x == DIR_CW ? "\n+1" : "\n-1");
#if ENABLE_SPEED
    Serial.print("  ");
    Serial.print(R.speed());
#endif
  }
}
