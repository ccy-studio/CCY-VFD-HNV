#include "gui.h"

u8 lightOff = 1;    // 背光开关
u8 lightLevel = 5;  // 亮度级别
static u8 xdata send_buf[24] = {0};
const u32 xdata fonts[1];

u32* gui_get_font(char c);

void start_pwm() {
    PWMA_CCER1 = 0x00;  // 在设置前先清零
    PWMA_PSCRH = 0x00;
    PWMA_PSCRL = 0x00;  // 1分频
    PWMA_ARRH = (u8)(PWM_ARR >> 8);
    PWMA_ARRL = (u8)PWM_ARR;
    PWMA_CCR1H = (u8)(PWM_CCR >> 8);
    PWMA_CCR1L = (u8)PWM_CCR;
    PWMA_CCMR1 = 0x68;  //
    PWMA_CCER1 = 0x01;  // 开启CC1
    // 输出使能
    PWMA_ENO = 0x04;   // 使能PWM2P输出
    PWMA_BKR = 0x80;   // 使能主输出
    PWMA_PS = 0x00;    // P1.2
    PWMA_CR1 |= 0x81;  // 开始计时
}

void stop_pwm() {
    PWMA_CR1 &= 0xFE;  // 关闭定时器
    PWMA_ENO = 0x00;   // 禁止使能
}

void vfd_gui_init() {
    VFD_EN = 1;
    start_pwm();
    // VFD Setting
    setDisplayMode(3);
    setModeWirteDisplayMode(0);
    vfd_gui_set_blk_level(lightLevel);
    vfd_gui_clear();
}

void vfd_gui_stop() {
    VFD_EN = 0;
    stop_pwm();
    vfd_gui_clear();
}

void vfd_gui_clear() {
    memset(send_buf, 0x00, sizeof(send_buf));
    sendDigAndData(0, send_buf, 24);
}

void vfd_gui_set_one_text(size_t index, char oneChar) {
    uint8_t arr[3];
    u32* buf = gui_get_font(oneChar);
    arr[0] = (*buf >> 16) & 0xFF;
    arr[1] = (*buf >> 8) & 0xFF;
    arr[2] = *buf & 0xFF;
    sendDigAndData(index * 3, arr, 3);
}

void vfd_gui_set_icon(u32 buf) {
    uint8_t arr[3];
    arr[0] = (buf >> 16) & 0xFF;
    arr[1] = (buf >> 8) & 0xFF;
    arr[2] = buf & 0xFF;
    sendDigAndData(0, arr, 3);
}

u8 vfd_gui_set_text(const char* string, const u8 colon) {
    size_t str_len = strlen(string);
    size_t index = 0, i = 0;
    size_t len = str_len > VFD_DIG_LEN ? VFD_DIG_LEN : str_len;
    memset(send_buf, 0x00, sizeof(send_buf));
    for (i = 0; i < len; i++) {
        if (string[i] && string[i] != '\0') {
            u32* buf = gui_get_font(string[i]);
            send_buf[index++] = (*buf >> 16) & 0xFF;
            send_buf[index++] = (*buf >> 8) & 0xFF;
            send_buf[index++] = *buf & 0xFF;
        }
    }
    if (colon) {
        send_buf[5] |= 0x10;
        send_buf[11] |= 0x10;
    }
    sendDigAndData(3, send_buf, 24);
    return 1;
}

void vfd_gui_set_bck(u8 onOff) {
    lightOff = onOff;
    ptSetDisplayLight(lightOff, lightLevel);
}

/**
 * 亮度调节 1~7
 */
void vfd_gui_set_blk_level(size_t level) {
    lightLevel = level;
    ptSetDisplayLight(lightOff, lightLevel);
}

long map(long x, long in_min, long in_max, long out_min, long out_max) {
    const long dividend = out_max - out_min;
    const long divisor = in_max - in_min;
    const long delta = x - in_min;
    return (delta * dividend + (divisor / 2)) / divisor + out_min;
}

u32* gui_get_font(char c) {
    if (c == ' ') {
        return 0x00;
    }
    if (c >= 33 && c <= 96) {
        // ! ~ `
        return &fonts[map(c, 33, 96, 0, 63)];
    } else if (c >= 97 && c <= 122) {
        // a~z
        return gui_get_font(c - 32);
    } else {
        return 0;
    }
}