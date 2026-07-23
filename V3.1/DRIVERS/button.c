#include "button.h"
/*
 * 非阻塞按键驱动设计说明：
 *
 * 不使用硬件定时器，采用 连续采样消抖 策略。
 * 要求主循环以较高频率调用 button_Scan()（建议间隔 < 5ms）。
 *
 * 状态机：
 *   IDLE ── 连续读到 N 次按下电平 ──> PRESSED
 *   PRESSED ── 返回事件 ──> RELEASE_WAIT
 *   RELEASE_WAIT ── 连续读到 N 次释放电平 ──> IDLE
 *
 * CONSISTENT_CNT = 5：连续 5 次采样一致才确认状态切换。
 * 若主循环调用间隔约 1ms，等效消抖时间约 5ms。
 * 若调用间隔更疏，可适当减小 CONSISTENT_CNT。
 */

#define CONSISTENT_CNT      5       // 连续一致采样次数（消抖用）

/* 按键状态机 */
typedef enum {
    STATE_IDLE,                     // 空闲，等待按下
    STATE_PRESSED,                  // 已确认按下
    STATE_RELEASE_WAIT,             // 等待释放
} ButtonState_t;

static ButtonState_t  state = STATE_IDLE;
static uint8_t        sampleCnt = 0;

/**
 * @brief  读取按键电平
 * @param  无
 * @retval 1 释放，0 按下
 */
static uint8_t button_Read(void)
{
    return (uint8_t)(GPIO_ReadInputDataBit(BUTTON_GPIO_PORT, BUTTON_GPIO_PIN));
}

/**
 * @brief  初始化按键GPIO
 * @param  无
 * @retval 无
 */
void button_Init(void)
{
    GPIO_InitTypeDef gpio;

    RCC_APB2PeriphClockCmd(BUTTON_GPIO_CLK, ENABLE);

    gpio.GPIO_Pin   = BUTTON_GPIO_PIN;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode  = GPIO_Mode_IPU;       // 上拉输入（外部已有上拉，内部再开无冲突）
    GPIO_Init(BUTTON_GPIO_PORT, &gpio);
}

/**
 * @brief  非阻塞按键扫描
 * @param  无
 * @retval ButtonEvent_t 按键事件
 */
ButtonEvent_t button_Scan(void)
{
    uint8_t level = button_Read();
    uint8_t isPressed = (level == BUTTON_PRESS_LEVEL);

    switch (state)
    {
    case STATE_IDLE:
        if (isPressed)
        {
            if (++sampleCnt >= CONSISTENT_CNT)
            {
                sampleCnt = 0;
                state = STATE_PRESSED;
                return BUTTON_EVENT_PRESSED;
            }
        }
        else
        {
            sampleCnt = 0;      // 中途弹回，计数器清零
        }
        break;

    case STATE_PRESSED:
        /* 返回事件后立即转入释放等待，避免重复触发 */
        state = STATE_RELEASE_WAIT;
        sampleCnt = 0;
        break;

    case STATE_RELEASE_WAIT:
        if (!isPressed)
        {
            if (++sampleCnt >= CONSISTENT_CNT)
            {
                sampleCnt = 0;
                state = STATE_IDLE;
            }
        }
        else
        {
            sampleCnt = 0;      // 中途按下，重新计数
        }
        break;
    }

    return BUTTON_EVENT_NONE;
}
