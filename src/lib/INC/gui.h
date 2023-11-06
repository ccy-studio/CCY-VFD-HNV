#ifndef __VFD_GUI_
#define __VFD_GUI_

#include "pt6315.h"

// VFDλ��
#define VFD_DIG_LEN 9

#define PWM_ARR 1000  // ����ֵ
#define PWM_CCR 150   // �Ƚ�ֵ

/**
 * ��ʼ��
 */
void vfd_gui_init();

/**
 * ֹͣ�ر���ʾ����˿��ֹͣ����
 */
void vfd_gui_stop();

/**
 * ���VFD��Ļ��ʾ,ѭ��ˢ�����ʹ��vfd_gui_set_text��������Ҫʹ������
 */
void vfd_gui_clear();

/**
 * ��ָ��λ����ʾһ��char�ַ�,index��1~6
 */
void vfd_gui_set_one_text(size_t index, char oneChar);

/**
 * ��ʾһ�����֣���0λ��ʼ��
 * (�Զ���ո�����ʾ������ÿ�β��õ���clear��ֹ��������)
 * @param colon �Ƿ���ʾð��
 */
void vfd_gui_set_text(const char* string, const u8 colon,const u8 left_first_conlon);

/**
 * Ҫ������ICONͼ�꣬�궨�崫��
 */
void vfd_gui_set_icon(u32 buf);

/**
 * ���⿪��
 */
void vfd_gui_set_bck(u8 onOff);

/**
 * �������ȵȼ� 1~7
 */
void vfd_gui_set_blk_level(size_t level);

long map(long x, long in_min, long in_max, long out_min, long out_max);
#endif