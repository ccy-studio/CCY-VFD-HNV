#ifndef __WS2812B_H
#define __WS2812B_H

#include "sys.h"

#define RGB_LED_COUNT 2  // WS2812B 

void rgb_set_color(u8 index, u8 r, u8 g, u8 b);
void rgb_update(u8 brightness);
void rgb_clear();
void rgb_frame_update(u8 brightness);
void rgb_timer_start();
void rgb_timer_stop();
void rgb_timer_set_brightness(u8 brightness);

#endif