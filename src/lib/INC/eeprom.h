#ifndef __EEPROM_H
#define __EEPROM_H
#include "sys.h"

#define STORE_ADDR 0x00

u8 ee_read(u16 start_addr);
void ee_erase(u16 addr);
void ee_write(u16 addr, u8 dat);

#endif