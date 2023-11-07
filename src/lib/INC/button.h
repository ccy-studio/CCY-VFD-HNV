#ifndef __BUTTON_H
#define __BUTTON_H
#include "sys.h"

#define KEY1_P P33
#define KEY2_P P34
#define KEY3_P P35
typedef enum { KEY1, KEY2, KEY3 } button_io_t;

typedef enum { BTN_RELEASE = 0, BTN_PRESS, BTN_LONG_PRESS } button_state_t;

typedef struct {
    button_io_t key;
    u32 last_press_time;
    button_state_t state;
} button_t;

typedef void (*button_callback)(button_t* btn);

void hal_init_button(button_callback callback);

#endif