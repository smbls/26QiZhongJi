#include "vision.h"
#include "usart.h"

/* 静态全局：存储接收到的视觉数据 */
static VisionData_t s_visionData = {0};

/**
 * @brief  发送请求命令给树莓派
 * @param  无
 * @retval 无
 * @note   发送: 0xAA 0x01 0x01 0xFF
 *         包头 长度 命令 包尾
 */
static void vision_SendRequest(void)
{
    uint8_t req[] = {VISION_HEADER, 0x01, VISION_CMD_GET, VISION_TAIL};
    usart2_SendCmd(req, sizeof(req));
}

/**
 * @brief  解析树莓派回复的二进制数据包
 * @param  buf: 接收缓冲区指针
 * @param  len: 接收数据长度
 * @retval true  — 解析成功
 *         false — 校验失败
 * @note   期望格式: 0xAA 0x08 D1 D2 D3 D4 D5 D6 D7 D8 0xFF
 *         D1~D3: 摄像头1 左→右 豆子编码
 *         D4~D8: 摄像头2 右→左 数字分布
 */
static bool vision_Parse(uint8_t* buf, uint8_t len)
{
    uint8_t i;

    /* 1. 基本长度校验 */
    if (len != VISION_PKT_LEN) return false;

    /* 2. 校验包头 */
    if (buf[0] != VISION_HEADER) return false;

    /* 3. 校验数据长度字段 */
    if (buf[1] != VISION_DATA_LEN) return false;

    /* 4. 校验包尾 */
    if (buf[VISION_PKT_LEN - 1] != VISION_TAIL) return false;

    /* 5. 提取摄像头1数据（buf[2..4]）: 从左到右 */
    for (i = 0; i < CAM1_POS_COUNT; i++)
    {
        s_visionData.cam1_beans[i] = buf[2 + i];
    }

    /* 6. 提取摄像头2数据（buf[5..9]）: 从右到左 */
    for (i = 0; i < CAM2_POS_COUNT; i++)
    {
        s_visionData.cam2_nums[i] = buf[5 + i];
    }

    return true;
}

/* 公开接口 ---------------------------------------------------------------- */

void vision_Init(void)
{
    /* 重置状态 */
    s_visionData.valid = false;

    /* 发送请求命令 */
    vision_SendRequest();

    /* 阻塞等待树莓派回复（超时由宏内循环保护） */
    WAIT_RESPONSE2();

    /* 解析接收到的数据 */
    s_visionData.valid = vision_Parse(rxCmd2, rxCount2);
}

const VisionData_t* vision_GetData(void)
{
    return &s_visionData;
}
