/*
 * @Description:
 * @Blog: saisaiwa.com
 * @Author: ccy
 * @Date: 2023-11-02 11:19:34
 * @LastEditTime: 2023-11-07 16:29:44
 */
// #include "eeprom.h"
#include "button.h"
#include "gui.h"
#include "rx8025.h"
#include "ws2812b.h"

/**
 * 0x0n 一级页面
 * 0x1x 二级页面(设置子页面)
 */
#define PAGE_FLAG_CLOCK_TIME 0x01          // 时间显示页面
#define PAGE_FLAG_CLOCK_DATE 0x02          // 日期显示页面
#define PAGE_FLAG_MENU 0x03                // 菜单页面
#define PAGE_FLAG_SET_CLOCK 0x10           // 设置时间子页面
#define PAGE_FLAG_SET_VFD_BRIGHTNESS 0x11  // 设置vfd亮度页面

#define MENU_ITEM_VFD_BRIGHTNESS 0x01  // 菜单选项卡:vfd亮度调整
#define MENU_ITEM_VFD_OPEN 0x02        // 菜单选项卡:vfd开关
#define MENU_ITEM_SET_CLOCK 0x03       // 菜单选项卡:时间设定
#define MENU_ITEM_OPEN_ACG 0x04        // 菜单选项卡:vfd开关动画特效
#define MENU_ITEM_OPEN_VFD_SAVER 0x05  // 菜单选项卡:vfd开关屏幕保护程序
#define MENU_ITEM_LAST MENU_ITEM_OPEN_VFD_SAVER

#define SCREEN_SAVER_TIME 60000UL  // 屏幕保护程序间隔执行时间单位毫秒

u8 vfd_brightness = 7;        // VFD亮度等级 1~7
bool vfd_open = true;         // VFD开关
bool acg_open = false;        // vfd动画特效开关
bool vfd_saver_open = false;  // vfd屏幕保护程序开关

u8 rgb_brightness = 3;  // RGB亮度等级 1~255对应1~3级调节
bool rgb_open = true;   // RGB开关
u8 config = 0;          // 用户配置信息

u8 page_display_flag;                          // 页面显示内容
u8 page_menu_item = MENU_ITEM_VFD_BRIGHTNESS;  // 菜单选项Flag
rx8025_timeinfo timeinfo;
rx8025_timeinfo set_timeinfo_cache;          // 设置timeinfo时的缓存
bool save_timeinfo_flag = false;             // 触发保存时间的操作flag
u8 data buffer[10];                          // vfd显示缓存
bool colon_flag = 0, left_first_colon_flag;  // vfd冒号显示状态

u32 data time_wait_count;
u32 data page_wait_count;
u32 data acg_wait_count;
u32 data saver_wait_count;
u32 data last_key_press_time;
u32 data rgb_wait_count;

extern u32 data _systick_ccr;

bool interval_check(u32 select, u32 t);
void page_home();
void menu_display_refresh();
void button_callback_fun(button_t* btn);

void main() {
    P_SW2 |= 0x80;  // 使能EAXFR寄存器 XFR
    hal_init_all_gpio();
    hal_init_systick();
    hal_init_uart();
    hal_init_button(button_callback_fun);
    rx8025t_init();
    vfd_gui_init();
    vfd_gui_set_blk_level(vfd_brightness);
    // config = ee_read(STORE_ADDR);
    // // 规则： 8bit 最高位和最低位如果不是1则需要清数据
    // if ((config & 0x81) >= 1) {
    //     ee_erase(STORE_ADDR);
    //     config = 0x81;
    // } else {
    //     // 读取数据 低位起： 第二位是rgb开关控制位，第三第四位是rgb亮度调节
    //     rgb_open = config & 0x02;
    //     rgb_brightness = (config >> 2) & 0x03;
    // }
    rx8025_set_time(23, 11, 6, 1, 22, 8, 6);
    page_display_flag = PAGE_FLAG_CLOCK_TIME;
    while (1) {
        // 开关VFD显示
        if (vfd_open) {
            if (!VFD_EN) {
                vfd_gui_init();
            }
        } else {
            if (VFD_EN) {
                vfd_gui_stop();
            }
        }
        // 如果VFD是关闭的状态就忽略执行
        if (!vfd_open) {
            continue;
        }
        if ((_systick_ccr - rgb_wait_count) >= 500) {
            // run rgb update
            // rgb_frame_update((u8)map(rgb_brightness, 0, 3, 0, 255));
            // printf("Time:%bd\t",rgb_wait_count);
            // printf("Hello\n");
            rgb_wait_count = _systick_ccr;
        }

        // 主页面内容筛选
        if (page_display_flag == PAGE_FLAG_CLOCK_TIME ||
            PAGE_FLAG_CLOCK_TIME == PAGE_FLAG_CLOCK_DATE) {
            if (interval_check(time_wait_count, 500)) {
                page_home();
                time_wait_count = _systick_ccr;
            }
        } else {
            // 菜单子页面内容刷新
            if (interval_check(page_wait_count, 200)) {
                if (page_display_flag == PAGE_FLAG_MENU) {
                    menu_display_refresh();
                }
                vfd_gui_set_text(buffer, 0, 0);
                page_wait_count = _systick_ccr;
            }
        }

        // 时间设定
        if (save_timeinfo_flag) {
            rx8025_set_time(set_timeinfo_cache.year, set_timeinfo_cache.month,
                            set_timeinfo_cache.day, 1, set_timeinfo_cache.hour,
                            set_timeinfo_cache.min, set_timeinfo_cache.sec);
            save_timeinfo_flag = false;
        }

        // vfd特效动画
        if (acg_open) {
            if (interval_check(acg_wait_count, 100)) {
                vfd_gui_acg_update();
                acg_wait_count = _systick_ccr;
            }
        }

        // 屏幕保护程序
        if (vfd_saver_open) {
            if (interval_check(saver_wait_count, SCREEN_SAVER_TIME)) {
                vfd_gui_display_protect_exec();
                saver_wait_count = _systick_ccr;
            }
        }
    }
}

bool interval_check(u32 select, u32 t) {
    return select > _systick_ccr || (_systick_ccr - select) >= t;
}

void page_home() {
    rx8025_time_get(&timeinfo);
    memset(buffer, 0, sizeof(buffer));
    if (page_display_flag == PAGE_FLAG_CLOCK_TIME) {
        colon_flag = !colon_flag;
        formart_time(&timeinfo, &buffer);
        left_first_colon_flag = 1;
    } else if (page_display_flag == PAGE_FLAG_CLOCK_DATE) {
        colon_flag = 0;
        formart_date(&timeinfo, &buffer);
        left_first_colon_flag = 0;
    }
    vfd_gui_set_text(buffer, colon_flag, left_first_colon_flag);
}

/**
 * 菜单选项卡显示内容刷新函数
 */
void menu_display_refresh() {
    memset(buffer, 0x00, sizeof(buffer));
    if (page_menu_item == MENU_ITEM_SET_CLOCK) {
        memcpy(buffer, "set-clock", 10);
    } else if (page_menu_item == MENU_ITEM_VFD_OPEN) {
        sprintf(buffer, "vfd:%s", vfd_open ? "open" : "close");
    } else if (page_menu_item == MENU_ITEM_VFD_BRIGHTNESS) {
        memcpy(buffer, "vfd-blk", 8);
    } else if (page_menu_item == MENU_ITEM_OPEN_ACG) {
        sprintf(buffer, "acg:%s", acg_open ? "open" : "close");
    } else if (page_menu_item == MENU_ITEM_OPEN_VFD_SAVER) {
        sprintf(buffer, "saver:%s", vfd_open ? "open" : "close");
    }
}

/**
 * 设定VFD亮度的页面触发
 */
void on_click_vfd_blk_handle(button_io_t io) {
    memset(buffer, 0x00, sizeof(buffer));
    if (io == KEY1) {
        if (vfd_brightness > 1) {
            vfd_brightness--;
        }
    } else if (io == KEY2) {
        if (vfd_brightness < 7) {
            vfd_brightness++;
        }
    }
    if (io == KEY3) {
        // 退出设定
        page_display_flag = PAGE_FLAG_MENU;
    } else {
        sprintf(buffer, "blk:%bd", map(vfd_brightness, 1, 7, 1, 100));
    }
}

/**
 * 触发设置时间刷新值并改变显示内容的操作
 */
void on_click_click_handle(button_io_t io) {
    static u8 select_i = 1, set_prefix[8];
    // select_i 分为设置年月日，时分秒 年为1,以此累加到秒为6
    u8* action_num;
    u8 max, min;
    memset(set_prefix, 0x00, sizeof(set_prefix));
    switch (select_i) {
        case 1:
            action_num = &set_timeinfo_cache.year;
            max = 50;
            min = 23;
            strcpy(set_prefix, "year:");
            break;
        case 2:
            action_num = &set_timeinfo_cache.month;
            max = 12;
            min = 1;
            strcpy(set_prefix, "month:");
            break;
        case 3:
            action_num = &set_timeinfo_cache.day;
            max = 31;
            min = 1;
            strcpy(set_prefix, "day:");
            break;
        case 4:
            action_num = &set_timeinfo_cache.hour;
            max = 23;
            min = 0;
            strcpy(set_prefix, "hour:");
            break;
        case 5:
            action_num = &set_timeinfo_cache.min;
            max = 59;
            min = 0;
            strcpy(set_prefix, "minute:");
            break;
        case 6:
            action_num = &set_timeinfo_cache.sec;
            max = 59;
            min = 0;
            strcpy(set_prefix, "second:");
            break;
    }
    if (io == KEY1) {
        if (*action_num == 0 || (*action_num - 1) < min) {
            *action_num = max;
        } else {
            *action_num -= 1;
        }
    } else if (io == KEY2) {
        if ((*action_num + 1) > max) {
            *action_num = min;
        } else {
            *action_num += 1;
        }
    } else if (io == KEY3) {
        select_i++;
        if (select_i > 6) {
            select_i = 1;
        }
    }
    memset(buffer, 0x00, sizeof(buffer));
    sprintf(buffer, "%s%02bd", set_prefix, *action_num);
}

void on_click_key1(button_state_t state) {
    if (state == BTN_RELEASE) {
        if (page_display_flag == PAGE_FLAG_CLOCK_TIME ||
            page_display_flag == PAGE_FLAG_CLOCK_DATE) {
            // 切换Home显示内容
            page_display_flag = (page_display_flag == PAGE_FLAG_CLOCK_TIME)
                                    ? PAGE_FLAG_CLOCK_DATE
                                    : PAGE_FLAG_CLOCK_TIME;
        } else if (page_display_flag == PAGE_FLAG_SET_CLOCK) {
            // 如果是设置时间的子页面触发子页面自己的逻辑
            on_click_click_handle(KEY1);
        } else if (page_display_flag == PAGE_FLAG_MENU) {
            // 菜单选项切换
            if (page_menu_item - 1 < 1) {
                page_menu_item = MENU_ITEM_LAST;
            } else {
                page_menu_item--;
            }
        } else if (page_display_flag == PAGE_FLAG_SET_VFD_BRIGHTNESS) {
            // 如果是vfd亮度调整交给亮度调整的函数去处理
            on_click_vfd_blk_handle(KEY1);
        }
    }
}
void on_click_key2(button_state_t state) {
    if (state == BTN_RELEASE) {
        if (page_display_flag == PAGE_FLAG_CLOCK_TIME ||
            page_display_flag == PAGE_FLAG_CLOCK_DATE) {
            // 切换Home显示内容
            page_display_flag = (page_display_flag == PAGE_FLAG_CLOCK_TIME)
                                    ? PAGE_FLAG_CLOCK_DATE
                                    : PAGE_FLAG_CLOCK_TIME;
        } else if (page_display_flag == PAGE_FLAG_SET_CLOCK) {
            // 如果是设置时间的子页面触发子页面自己的逻辑
            on_click_click_handle(KEY2);
        } else if (page_display_flag == PAGE_FLAG_MENU) {
            // 菜单选项切换
            if (page_menu_item + 1 > MENU_ITEM_LAST) {
                page_menu_item = 1;
            } else {
                page_menu_item++;
            }
        } else if (page_display_flag == PAGE_FLAG_SET_VFD_BRIGHTNESS) {
            // 如果是vfd亮度调整交给亮度调整的函数去处理
            on_click_vfd_blk_handle(KEY2);
        }
    }
}
void on_click_key3(button_state_t state) {
    if (state == BTN_LONG_PRESS) {
        if (page_display_flag == PAGE_FLAG_CLOCK_DATE ||
            page_display_flag == PAGE_FLAG_CLOCK_TIME) {
            // 切换到MENU菜单
            page_display_flag = PAGE_FLAG_MENU;
        } else if (page_display_flag == PAGE_FLAG_MENU) {
            // 退出menu菜单
            page_display_flag = PAGE_FLAG_CLOCK_TIME;
        } else if (page_display_flag == PAGE_FLAG_SET_CLOCK) {
            // 保存日期设置设置并退出到菜单menu
            save_timeinfo_flag = true;
            page_display_flag = PAGE_FLAG_MENU;
        }
    }
    if (state == BTN_RELEASE) {
        if (page_display_flag == PAGE_FLAG_SET_CLOCK) {
            // 如果是设置时间的子页面触发子页面自己的逻辑
            on_click_click_handle(KEY3);
        } else if (page_display_flag == PAGE_FLAG_MENU) {
            // 如果是菜单就翻转开关对应的设置选项或者进入页面
            if (page_menu_item == MENU_ITEM_SET_CLOCK) {
                // 1.进入设置时间页面
                memcpy(&set_timeinfo_cache, &timeinfo,
                       sizeof(set_timeinfo_cache));
                page_display_flag = PAGE_FLAG_SET_CLOCK;
                on_click_click_handle(255);
            } else if (page_menu_item == MENU_ITEM_VFD_OPEN) {
                // 2. 开关VFD显示
                vfd_open = !vfd_open;
            } else if (page_menu_item == MENU_ITEM_VFD_BRIGHTNESS) {
                // 3. 设置VFD的亮度
                page_display_flag = PAGE_FLAG_SET_VFD_BRIGHTNESS;
                on_click_vfd_blk_handle(255);
            } else if (page_menu_item == MENU_ITEM_OPEN_ACG) {
                // 4. 开关vfd动画
                acg_open = !acg_open;
            } else if (page_menu_item == MENU_ITEM_OPEN_VFD_SAVER) {
                // 5. 开关屏幕保护程序
                vfd_saver_open = !vfd_saver_open;
            }
        } else if (page_display_flag == PAGE_FLAG_SET_VFD_BRIGHTNESS) {
            // 如果是vfd亮度设置调整交给对应的功能处理函数去处理
            on_click_vfd_blk_handle(KEY3);
        }
    }
}

void button_callback_fun(button_t* btn) {
    if (!vfd_open) {
        // 按键检测中如何发现vfd是关闭的状态，此时有按键按下就自动开启vfd显示
        vfd_open = true;
    }
    switch (btn->key) {
        case KEY1:
            on_click_key1(btn->state);
            break;
        case KEY2:
            on_click_key2(btn->state);
            break;
        case KEY3:
            on_click_key3(btn->state);
            break;
    }
}
