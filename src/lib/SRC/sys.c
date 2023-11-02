#include "sys.h"

u32 data _systick_ccr = 0;

#ifdef DEV_PLATFROM
u8 rx_index = 0;
const char* xdata STCISPCMD = "@STCISP#";
void hal_init_uart(void) {
    // 115200
    SCON = 0x50;   // 8位数据,可变波特率
    AUXR |= 0x40;  // 定时器时钟1T模式
    AUXR &= 0xFE;  // 串口1选择定时器1为波特率发生器
    TMOD &= 0x0F;  // 设置定时器模式
    TL1 = 0xD0;    // 设置定时初始值
    TH1 = 0xFF;    // 设置定时初始值
    ET1 = 0;       // 禁止定时器中断
    TR1 = 1;       // 定时器1开始计时
}

void uart_isr(void) interrupt 4 {
    char dat;
    if (TI) {
        TI = 0;
    }
    if (RI) {
        RI = 0;
        dat = SBUF;
        if (dat == STCISPCMD[rx_index]) {
            rx_index++;
            if (STCISPCMD[rx_index] == '\0') {
                IAP_CONTR = 0x60;  // 软复位到ISP进行下载
            }
        } else {
            // 不匹配重新开始
            rx_index++;
            if (dat == STCISPCMD[rx_index]) {
                rx_index++;
            }
        }
    }
}

char putchar(char ch) {
    SBUF = ch;  // 串口1数据寄存器
    while (TI == 0)
        ;  // 串口1中断发送请求标志
    TI = 0;
    return ch;
}
#endif

void hal_init_systick() {
    // 1毫秒@22.1184MHz
    AUXR |= 0x80;  // 定时器时钟1T模式
    TMOD = 0xF3;   // 设置定时器模式
    TL0 = 0x9A;    // 设置定时初始值
    TH0 = 0xA9;    // 设置定时初始值
    TF0 = 0;       // 清除TF0标志
    TR0 = 1;       // 定时器0开始计时
    ET0 = 1;       // 使能定时器0中断
}
u32 hal_systick_get() {
    return _systick_ccr;
}

void timer0_Isr(void) interrupt 1 {
    _systick_ccr++;
}

void hal_init_all_gpio(void) {
    P3M0 = 0x00;
    P3M1 = 0x00;
    P1M0 = 0x00;
    P1M1 = 0x00;
    // I2C内部上拉
    P1PU = 0xE0;
    // Key按键高阻输入+内部上拉
    P3M0 |= 0x78;
    P3M1 |= 0x78;
    P3PU = 0x78;
    // RGB 配置推挽输出+高速模式
    P1M0 |= 0x08;
    P1SR &= 0xf7;
}

void delay_ms(u32 ms) {
    unsigned char data i, j;
    do {
        i = 29;
        j = 183;
        do {
            while (--j)
                ;
        } while (--i);
    } while (--ms);
}

void delay_us(u32 us) {
    unsigned char data i;

    do {
        _nop_();
        i = 5;
        while (--i)
            ;
    } while (--us);
}