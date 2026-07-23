#ifndef __BUTTON_H
#define __BUTTON_H

#include "stm32f10x.h"

/* 按键引脚定义：PA0，外部上拉，按下为低电平 */
#define BUTTON_GPIO_PORT     GPIOA
#define BUTTON_GPIO_PIN      GPIO_Pin_0
#define BUTTON_GPIO_CLK      RCC_APB2Periph_GPIOA

/* 按下为低电平有效 */
#define BUTTON_PRESS_LEVEL   0

/* 按键事件枚举 */
typedef enum {
    BUTTON_EVENT_NONE = 0,      // 无事件
    BUTTON_EVENT_PRESSED,       // 按下事件（单次单击）
} ButtonEvent_t;

/**
 * @brief  初始化按键GPIO（PA0，上拉输入）
 * @param  无
 * @retval 无
 */
void button_Init(void);

/**
 * @brief  非阻塞按键扫描，供主循环周期调用
 * @param  无
 * @retval ButtonEvent_t 返回按键事件
 * @note   连续读取消抖，无需定时器，要求主循环调用间隔 < 20ms
 */
ButtonEvent_t button_Scan(void);

#endif
