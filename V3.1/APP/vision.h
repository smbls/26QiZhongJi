#ifndef __VISION_H
#define __VISION_H

#include <stdint.h>
#include "stdbool.h"

/* 豆子类型编码 */
#define BEAN_YELLOW     1   // 黄豆
#define BEAN_GREEN      2   // 绿豆
#define BEAN_WHITE      3   // 白芸豆

/* 摄像头位置数量 */
#define CAM1_POS_COUNT  3
#define CAM2_POS_COUNT  5

/* 二进制数据包协议常量 */
#define VISION_HEADER   0xAA
#define VISION_TAIL     0xFF
#define VISION_CMD_GET  0x01    // 请求视觉数据命令码
#define VISION_DATA_LEN 8       // 回复数据段长度
#define VISION_PKT_LEN  11      // 完整包长: 1+1+8+1

/**
 * @brief  视觉数据结构体
 * @note   cam1_beans: 摄像头1, 3个位置从左到右的豆子类型
 *         cam2_nums:  摄像头2, 5个位置从右到左的数字分布（位置1=最右）
 */
typedef struct {
    uint8_t cam1_beans[CAM1_POS_COUNT];  // 1=黄豆, 2=绿豆, 3=白芸豆
    uint8_t cam2_nums[CAM2_POS_COUNT];   // 1-5
    bool    valid;                        // true=数据已成功接收解析
} VisionData_t;

/**
 * @brief  初始化视觉通讯：发送请求 → 等待回复 → 解析数据
 * @param  无
 * @retval 无
 * @note   阻塞等待树莓派回复，超时由 WAIT_RESPONSE2 的 1000000 周期保护
 */
void vision_Init(void);

/**
 * @brief  获取视觉数据指针
 * @param  无
 * @retval const VisionData_t* 指向视觉数据的只读指针
 */
const VisionData_t* vision_GetData(void);

#endif
