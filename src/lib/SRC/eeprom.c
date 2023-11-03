/*
 * @Description: 
 * @Blog: saisaiwa.com
 * @Author: ccy
 * @Date: 2023-11-03 16:42:16
 * @LastEditTime: 2023-11-03 16:51:20
 */
#include "eeprom.h"

void iap_close() {
    IAP_CONTR = 0;
    IAP_CMD = 0;
    IAP_TRIG = 0;
    IAP_ADDRH = 0x80;
    IAP_ADDRL = 0;
}

u8 ee_read(u16 start_addr) {
    u8 dat;
    IAP_CONTR = 0x80;
    IAP_TPS = 22;  // 22.118Mhz
    IAP_CMD = 1;
    IAP_ADDRL = start_addr;
    IAP_ADDRH = start_addr >> 8;
    IAP_TRIG = 0x5a;
    IAP_TRIG = 0xa5;
    _nop_();
    dat = IAP_DATA;
    iap_close();
    return dat;
}

void ee_erase(u16 addr) {
    IAP_CONTR = 0x80;
    IAP_TPS = 22;  // 22.118Mhz
    IAP_CMD = 3;
    IAP_ADDRL = addr;
    IAP_ADDRH = addr >> 8;
    IAP_TRIG = 0x5a;
    IAP_TRIG = 0xa5;
    _nop_();
    iap_close();
}

void ee_write(u16 addr, u8 dat) {
    IAP_CONTR = 0x80;
    IAP_TPS = 22;  // 22.118Mhz
    IAP_CMD = 2;
    IAP_ADDRL = addr;
    IAP_ADDRH = addr >> 8;
    IAP_DATA = dat;
    IAP_TRIG = 0x5a;
    IAP_TRIG = 0xa5;
    _nop_();
    iap_close();
}