#ifndef __VFD_GUI_
#define __VFD_GUI_

#include "pt6315.h"

// VFD位数
#define VFD_DIG_LEN 6

#define PWM_ARR 1000  // 重载值
#define PWM_CCR 500   // 比较值


/**
 * 初始化
 */
void vfd_gui_init();

/**
 * 停止关闭显示、灯丝将停止驱动
 */
void vfd_gui_stop();

/**
 * 清空VFD屏幕显示,循环刷新如果使用vfd_gui_set_text方法不需要使用它。
 */
void vfd_gui_clear();

/**
 * 在指定位置显示一个char字符,index从1~6
 */
void vfd_gui_set_one_text(size_t index, char oneChar);

/**
 * 显示一串文字，从0位开始。
 * (自动清空覆盖显示，方便每次不用调用clear防止闪屏出现)
 * @param colon 是否显示冒号
 */
u8 vfd_gui_set_text(const char* string, const u8 colon);

/**
 * 要点亮的ICON图标，宏定义传参
 */
void vfd_gui_set_icon(u32 buf);

/**
 * 背光开关
 */
void vfd_gui_set_bck(u8 onOff);

/**
 * 设置亮度等级 1~7
 */
void vfd_gui_set_blk_level(size_t level);

#endif