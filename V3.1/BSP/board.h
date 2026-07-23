#ifndef __BOARD_H
#define __BOARD_H

#include "stm32f10x.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_it.h"

/**********************************************************
***	Emm_V5.0步进闭环控制例程
***	编写作者：ZHANGDATOU
***	技术支持：张大头闭环伺服
***	淘宝店铺：https://zhangdatou.taobao.com
***	CSDN博客：http s://blog.csdn.net/zhangdatou666
***	qq交流群：262438510
**********************************************************/
extern volatile uint32_t system_ms;

void nvic_init(void);
void clock_init(void);
void usart_init(void);
void board_init(void);

#endif
