/*
 * @Description:
 * @Blog: saisaiwa.com
 * @Author: ccy
 * @Date: 2023-11-07 10:29:12
 * @LastEditTime: 2023-11-07 11:52:35
 */
#include "button.h"

#define BTN_LONG_PRESS_MS 1000
#define BTN_SCAN_MS 10

static button_t btns[3];
static button_callback callback;

void hal_init_button(button_callback call) {
    callback = call;
    btns[0].key = KEY1;
    btns[1].key = KEY2;
    btns[2].key = KEY3;
    // 启动定时器
    //  10毫秒@22.1184MHz
    AUXR &= 0xFB;  // 定时器时钟12T模式
    T2L = 0x00;    // 设置定时初始值
    T2H = 0xB8;    // 设置定时初始值
    AUXR |= 0x10;  // 定时器2开始计时
    IE2 |= 0x04;   // 使能定时器2中断
}

u8 io_level(button_io_t io) {
    if (io == KEY1) {
        return KEY1_P;
    }
    if (io == KEY2) {
        return KEY2_P;
    }
    if (io == KEY3) {
        return KEY3_P;
    }
    return 0;
}

void handle(button_t* btn) {
    if (btn->last_press_time >= BTN_LONG_PRESS_MS && btn->state == BTN_PRESS) {
        btn->last_press_time += BTN_SCAN_MS;
        return;
    }
    if (!io_level(btn->key)) {
        // press
        if (btn->state == BTN_PRESS) {
            btn->last_press_time += BTN_SCAN_MS;
        } else if (btn->state == BTN_RELEASE) {
            btn->state = BTN_PRESS;
            btn->last_press_time = 0;
            callback(btn);
            return;
        }
        if (btn->state == BTN_PRESS &&
            btn->last_press_time >= BTN_LONG_PRESS_MS) {
            btn->last_press_time += BTN_SCAN_MS;
            btn->state = BTN_LONG_PRESS;
            callback(btn);
        }
    } else {
        btn->state = BTN_RELEASE;
        btn->last_press_time = 0;
        callback(btn);
    }
}

void key_button_isr(void) interrupt 12 {
    handle(&btns[0]);
    handle(&btns[1]);
    handle(&btns[2]);
}