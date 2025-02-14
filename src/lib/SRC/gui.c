#include "gui.h"

u8 lightLevel = 1;  // 亮度级别
static u8 data send_buf[3 * VFD_DIG_LEN] = {0};
static u8 data send_buf_cache[3 * VFD_DIG_LEN] = {0};
const u32 xdata fonts[38] = {
    0x333300,  // ASCII:0,ASCII_N:48 index:0
    0x201000,  // ASCII:1,ASCII_N:49 index:1
    0xe12100,  // ASCII:2,ASCII_N:50 index:2
    0xe13000,  // ASCII:3,ASCII_N:51 index:3
    0xe21000,  // ASCII:4,ASCII_N:52 index:4
    0xc33000,  // ASCII:5,ASCII_N:53 index:5
    0xc33100,  // ASCII:6,ASCII_N:54 index:6
    0x211000,  // ASCII:7,ASCII_N:55 index:7
    0xe33100,  // ASCII:8,ASCII_N:56 index:8
    0xe33000,  // ASCII:9,ASCII_N:57 index:9
    0x080400,  // ASCII::,ASCII_N:58 index:10
    0xe31100,  // ASCII:A,ASCII_N:65 index:11
    0xa93400,  // ASCII:B,ASCII_N:66 index:12
    0x032100,  // ASCII:C,ASCII_N:67 index:13
    0x293400,  // ASCII:D,ASCII_N:68 index:14
    0xc32100,  // ASCII:E,ASCII_N:69 index:15
    0xc30100,  // ASCII:F,ASCII_N:70 index:16
    0xc33100,  // ASCII:G,ASCII_N:71 index:17
    0xe21100,  // ASCII:H,ASCII_N:72 index:18
    0x092400,  // ASCII:I,ASCII_N:73 index:19
    0x082400,  // ASCII:J,ASCII_N:74 index:20
    0x520900,  // ASCII:K,ASCII_N:75 index:21
    0x022100,  // ASCII:L,ASCII_N:76 index:22
    0x361100,  // ASCII:M,ASCII_N:77 index:23
    0x261900,  // ASCII:N,ASCII_N:78 index:24
    0x233100,  // ASCII:O,ASCII_N:79 index:25
    0xe10100,  // ASCII:P,ASCII_N:80 index:26
    0x233900,  // ASCII:Q,ASCII_N:81 index:27
    0xe30900,  // ASCII:R,ASCII_N:82 index:28
    0xc33000,  // ASCII:S,ASCII_N:83 index:29
    0x090400,  // ASCII:T,ASCII_N:84 index:30
    0x223100,  // ASCII:U,ASCII_N:85 index:31
    0x241800,  // ASCII:V,ASCII_N:86 index:32
    0x221b00,  // ASCII:W,ASCII_N:87 index:33
    0x140a00,  // ASCII:X,ASCII_N:88 index:34
    0x140400,  // ASCII:Y,ASCII_N:89 index:35
    0x112004,  // ASCII:Z,ASCII_N:90 index:36
    0xc00000,  // ASCII:-,ASCII_N:45 index:37
};

u32* gui_get_font(char c);

void start_pwm() {
    PWMA_CCER1 = 0x00;    // 写CCMRx前必须先清零CCERx关闭通道
    PWMA_CCMR2 = 0x68;    // 设置CC2为PWMA输出模式
    PWMA_CCER1 = 0x10;    // 使能CC2通道
    PWMA_CCR2 = PWM_CCR;  // 设置占空比时间
    PWMA_ARR = PWM_ARR;   // 设置周期时间
    PWMA_ENO = 0x04;      // 使能PWM2P端口输出
    PWMA_BKR = 0x80;      // 使能主输出
    PWMA_CR1 |= 0x81;     // 开始计时
}

void stop_pwm() {
    PWMA_CR1 &= 0xFE;  // 关闭定时器
    PWMA_ENO = 0x00;   // 禁止使能
}

void vfd_gui_init() {
    VFD_EN = 1;
    start_pwm();
    // VFD Setting
    setDisplayMode(6);
    setModeWirteDisplayMode(0);
    vfd_gui_set_blk_level(7);
    vfd_gui_clear();
}

void vfd_gui_stop() {
    VFD_EN = 0;
    stop_pwm();
    vfd_gui_clear();
}

void vfd_gui_clear() {
    memset(send_buf, 0x00, sizeof(send_buf));
    sendDigAndData(0, send_buf, sizeof(send_buf));
}

void vfd_gui_set_icon(u32 buf) {
    uint8_t arr[3] = {0};
    memset(arr, 0x00, sizeof(arr));
    if (buf) {
        arr[0] = (buf >> 16) & 0xFF;
        arr[1] = (buf >> 8) & 0xFF;
        arr[2] = buf & 0xFF;
    }
    sendDigAndData(0x1b, arr, 3);
}

void vfd_gui_set_text(const char* string,
                      const u8 colon,
                      const u8 left_first_conlon) {
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
    if (left_first_conlon) {
        send_buf[7] |= 0x40;
    }
    if (colon) {
        send_buf[13] |= 0x40;
        send_buf[19] |= 0x40;
    }
    sendDigAndData(0, send_buf, sizeof(send_buf));
}

/**
 * 亮度调节 1~7
 */
void vfd_gui_set_blk_level(size_t level) {
    if (level == lightLevel) {
        return;
    }
    lightLevel = level;
    ptSetDisplayLight(1, lightLevel);
}

long map(long x, long in_min, long in_max, long out_min, long out_max) {
    const long dividend = out_max - out_min;
    const long divisor = in_max - in_min;
    const long delta = x - in_min;
    return (delta * dividend + (divisor / 2)) / divisor + out_min;
}

u32* gui_get_font(char c) {
    if (c == ' ') {
        return 0;
    }
    if (c == '-') {
        return &fonts[37];
    }
    if (c >= 48 && c <= 58) {
        return &fonts[map(c, 48, 58, 0, 10)];
    } else if (c >= 65 && c <= 90) {
        return &fonts[map(c, 65, 90, 11, 36)];
    } else if (c >= 97 && c <= 122) {
        return gui_get_font(c - 32);
    } else {
        return 0;
    }
}

/**
 * acg动画
 */
void vfd_gui_acg_update() {
    static u8 acf_i = 9;
    if (acf_i == 9) {
        static u32 icon = 0x040000;
        static u8 sec = 0;
        vfd_gui_set_icon(icon);
        sec++;
        if (sec == 3) {
            icon = 0x008000;
        } else if (sec < 3) {
            icon = (0x040000 >> sec);
        } else {
            icon = (0x040000 << (sec - 3));
        }
        if (sec == 4) {
            acf_i = 0;
        }
        if (sec == 7) {
            sec = 0;
            icon = 0x040000;
        }
    } else {
        u8 bi = acf_i == 0 ? 1 : (acf_i + 1) * 3 - 2;
        memcpy(send_buf_cache, send_buf, sizeof(send_buf));
        if (acf_i == 2 || acf_i == 4 || acf_i == 6) {
            send_buf_cache[bi] |= 0x80;
        } else {
            send_buf_cache[bi] |= 0xC0;
        }
        if (acf_i == 0) {
            vfd_gui_set_icon(0);
        }
        sendDigAndData(0, send_buf_cache, sizeof(send_buf_cache));
        acf_i++;
        if (acf_i == 9) {
            sendDigAndData(0, send_buf, sizeof(send_buf));
        }
    }
}

/**
 * 屏幕保护程序
 */
void vfd_gui_display_protect_exec() {
    u8 i, j;
    u8 data buf[12];
    for (i = 1; i <= 10; i++) {
        memset(buf, 0x00, sizeof(buf));
        for (j = 0; j < 10; j++) {
            u8 rn = (hal_systick_get() << i) % 10;
            sprintf((&buf) + j, "%bd", rn);
            delay_ms(1);
        }
        vfd_gui_set_text(buf, 0, 0);
        delay_ms(10);
    }
}
