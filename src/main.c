/*
 * @Description:
 * @Blog: saisaiwa.com
 * @Author: ccy
 * @Date: 2023-11-02 11:19:34
 * @LastEditTime: 2023-11-02 16:10:26
 */

#include "gui.h"
#include "rx8025.h"
#include "ws2812b.h"

void main() {
    hal_init_all_gpio();
    hal_init_systick();
    hal_init_uart();
    rx8025t_init();
    vfd_gui_init();

    rgb_timer_start();

    while (1) {
    }
}