#ifndef _ADNS5020_H_
#define _ADNS5020_H_

#include <stdint.h>

#define SCLK 5
#define SDIO 6
#define NCS 7

void pushbyte(uint8_t c);
uint8_t pullbyte();
void mouse_reset();
uint8_t readLoc(uint8_t addr);
uint8_t writeLoc(uint8_t addr, uint8_t data);
void startBurstRead();
void stopBurstRead();
uint8_t readBurst();

#endif /* !_ADNS5020_H_ */
