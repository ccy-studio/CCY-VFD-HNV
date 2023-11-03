/*
 * @Description:
 * @Blog: saisaiwa.com
 * @Author: ccy
 * @Date: 2023-11-02 11:19:34
 * @LastEditTime: 2023-11-03 17:51:23
 */
/*
 * @Description:
 * @Blog: saisaiwa.com
 * @Author: ccy
 * @Date: 2023-11-02 11:19:34
 * @LastEditTime: 2023-11-03 17:06:25
 */

#include "eeprom.h"
#include "gui.h"
#include "rx8025.h"
#include "ws2812b.h"

#define KEY_FLAG_LP 1
#define KEY_FLAG_LS 2
#define KEY_FLAG_M 3
#define KEY_FLAG_S 4

#define PAGE_FLAG_CLOCK_TIME 0x01
#define PAGE_FLAG_CLOCK_DATE 0x02
#define PAGE_FLAG_MENU 0x03
#define PAGE_FLAG_SET_CLOCK 0x10

#define PAGE_MENU_FLAG_RGB_BLK 0x00
#define PAGE_MENU_FLAG_RGB_OPEN 0x01
#define PAGE_MENU_FLAG_SET_CLOCK 0x02

u8 vfd_brightness = 7;                // VFD亮度等级 1~7
u8 rgb_brightness = 3;                // RGB亮度等级 1~255对应1~3级调节
bool rgb_open = true;                 // RGB开关
u8 config = 0;                        // 用户配置信息
u8 page_flag = PAGE_FLAG_CLOCK_TIME;  // 页面显示内容
u8 page_menu_flag = PAGE_MENU_FLAG_RGB_BLK;  // 菜单选项Flag
rx8025_timeinfo timeinfo;
u8 data buffer[7];    // 显示缓存
bool colon_flag = 0;  // 冒号显示状态

void page_menu();
void page_main();

void main() {
    P_SW2 |= 0x80;  // 使能EAXFR寄存器 XFR
    hal_init_all_gpio();
    hal_init_systick();
    hal_init_uart();
    rx8025t_init();
    vfd_gui_init();
    config = ee_read(STORE_ADDR);
    // 规则： 8bit 最高位和最低位如果不是1则需要清数据
    if ((config & 0x81) >= 1) {
        ee_erase(STORE_ADDR);
        config = 0x81;
    } else {
        // 读取数据 低位起： 第二位是rgb开关控制位，第三第四位是rgb亮度调节
        rgb_open = config & 0x02;
        rgb_brightness = (config >> 2) & 0x03;
    }
    rgb_timer_set_brightness((u8)map(rgb_brightness, 0, 3, 0, 255));
    rgb_timer_start();
    while (1) {
        if (page_flag == PAGE_FLAG_CLOCK_TIME ||
            page_flag == PAGE_FLAG_CLOCK_DATE) {
            page_main();
        } else if (page_flag == PAGE_FLAG_MENU) {
            page_menu();
        }
    }
}

void page_main() {
    rx8025_time_get(&timeinfo);
    memset(buffer, 0, sizeof(buffer));
    if (page_flag == PAGE_FLAG_CLOCK_TIME) {
        colon_flag = !colon_flag;
        formart_time(&timeinfo, &buffer);
    } else if (page_flag == PAGE_FLAG_CLOCK_DATE) {
        colon_flag = 0;
        formart_date(&timeinfo, &buffer);
    }
    vfd_gui_set_text(buffer, colon_flag);
    delay_ms(500);
}

void page_menu() {
    memset(buffer, 0, sizeof(buffer));
    if (page_menu_flag == PAGE_MENU_FLAG_RGB_BLK) {
    } else if (page_menu_flag == PAGE_MENU_FLAG_RGB_OPEN) {
    } else if (page_menu_flag == PAGE_MENU_FLAG_SET_CLOCK) {
    }
    delay_ms(500);
}

void page_menu_click_handle(u8 key) {
    if (page_flag == PAGE_FLAG_MENU) {
    }
}

/**
 * 按键中断触发
 */
void user_key_isr(void) interrupt 13 {  // 借用13号 原中断号40
    u8 intf = P3INTF;
    u8 key_flag;
    if (intf) {
        P3INTF = 0;
        // 单点逻辑判断
        if (intf & 0x08) {
            key_flag = KEY_FLAG_LP;
        } else if (intf & 0x10) {
            key_flag = KEY_FLAG_LS;
        } else if (intf & 0x20) {
            key_flag = KEY_FLAG_M;
        } else if (intf & 0x40) {
            key_flag = KEY_FLAG_S;
        }
        page_menu_click_handle(key_flag);
    }
}