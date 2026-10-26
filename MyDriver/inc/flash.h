#ifndef __FLASH_H_
#define __FLASH_H_

long ReadEepromLong( unsigned char eAdd);
char ReadEeprom( unsigned char eAdd);
unsigned short int ReadEepromInt( unsigned int eAdd);
extern unsigned int FlashCheckData[2];//????flash???????????~{!l~}
void ReadFlashAll(void);
unsigned char WriteFlashAll(void);
char WriteEepromLong( unsigned char eAdd,long eData);
char WriteEepromInt( unsigned int eAdd,int eData);
char WriteEeprom( unsigned int eAdd,char eData);

#endif
