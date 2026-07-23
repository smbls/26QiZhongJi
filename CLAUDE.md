# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述

基于 STM32F103C8 控制 ZDT XS 系列闭环步进电机（"张大头闭环伺服"），通过 **单串口（USART1）** 控制 **4 台电机**（地址 1~4），实现 **X 轴 + Y 轴双轴多点定位运动**。使用 Keil MDK 开发，基于 STM32F10x 标准外设库 V3.5.0。

电机配对方式：
- **电机 1 & 2**（X 轴）— 两侧安装，方向相反实现同向移动
- **电机 3 & 4**（Y 轴）— 两侧安装，方向相反实现同向移动

## 版本说明

| 版本 | 路径 | 状态 |
|------|------|------|
| **V3.1（当前）** | `V3.1/` | 活跃开发版本，非阻塞状态机架构 |
| V1.3.1（旧版） | `V1.3.1/` | 已从工作区删除（`git rm`），历史记录可见 |

> 仓库目录名为 `V3.0状态机调参+视觉通讯`，但实际代码已演进到 **V3.1**（非阻塞状态机架构）。所有工作都在 `V3.1/` 目录下进行。

**V3.1 核心改进**：使用 `MotionState_t` 状态机 + `system_ms` 1ms 定时器替代阻塞式 `delay_ms()`。

## 构建方式

使用 **Keil MDK**（µVision）编译，调试器为 JLink：
- 工程文件：`V3.1/PRJ/STM32_UART_CMD.uvprojx`
- 无命令行构建工具，用 Keil µVision 打开编译、下载、调试

## 代码架构（V3.1）

所有用户代码位于 `V3.1/` 目录下：

```
V3.1/
├── APP/              # 应用层
│   ├── main.c          # 主程序：状态机调度 + 位置常量 + 轨迹逻辑
│   ├── vision.c/h      # 树莓派视觉通讯（USART2）：请求/解析二进制数据包
│   ├── stm32f10x_it.c  # 异常中断 + TIM4_IRQHandler (system_ms++)
│   ├── stm32f10x_it.h
│   └── stm32f10x_conf.h
├── BSP/              # 板级支持包
│   ├── board.c/h        # NVIC → 时钟(72MHz) → USART1+USART2 → TIM4 初始化
│   ├── usart.c/h        # USART1+USART2 收发驱动、中断处理、WAIT_RESPONSE 宏
│   └── Emm_V5.c/h       # ZDT Emm V5 电机控制 API（1733 行，含 MMCL 多机命令列表）
├── DRIVERS/          # 底层驱动
│   ├── button.c/h       # 非阻塞按键（PA0，连续采样消抖）
│   ├── delay.c/h        # SysTick 阻塞延时（仅上电初始化使用）
│   └── fifo.c/h         # uint16_t 环形缓冲区（FIFO_SIZE=128），USART1+USART2 各一
├── CMSIS/            # Cortex-M3 内核 + 启动文件
├── LIB/              # STM32F10x 标准外设库 V3.5.0
└── PRJ/              # Keil 工程 + JLink 配置 + 编译输出
```

### 核心架构：非阻塞状态机

**V3.1 最大变化**：用 `MotionState_t` 状态机 + `system_ms` 硬件定时器替代了 V1.3.1 中的阻塞式 `delay_ms()`。

#### 1ms 时基
- `TIM4` 配置为 APB1(36MHz) /36/1000 = 1ms 中断（`board.c:timer4_init`）
- `TIM4_IRQHandler` 递增全局 `system_ms`（`stm32f10x_it.c:2-9`）

#### MotionState_t 枚举（main.c:36-48）
```c
typedef enum {
    MOTION_IDLE,
    /* test2 轨迹（4箱）*/
    PHASE_X3,  PHASE_X3_WAIT,
    PHASE_Y4,  PHASE_Y4_WAIT,
    PHASE_Y5,  PHASE_Y5_WAIT,
    PHASE_HOME,PHASE_HOME_WAIT,
    /* OneCycle 轨迹（3箱循环）*/
    OC_BOX1,   OC_BOX1_WAIT,
    OC_BOX2,   OC_BOX2_WAIT,
    OC_BOX3,   OC_BOX3_WAIT,
    OC_HOME,   OC_HOME_WAIT,
} MotionState_t;
```

#### 主循环调度（main.c:267-278）
```c
while(1) {
    if (button_Scan() == BUTTON_EVENT_PRESSED && motion_state == MOTION_IDLE)
        motion_start_OneCycle();  // 可改为 motion_start_test2()
    if (motion_state >= OC_BOX1 && motion_state <= OC_HOME_WAIT)
        motion_OneCycle();        // 3箱循环路径
    else
        motion_state_machine();   // test2 4箱路径
}
```

#### 状态机模式
- 每相位分 `ACTION` + `WAIT` 两步
- **ACTION 阶段**：发送 2~4 条串口命令（MMCL 缓存）→ `Emm_V5_Synchronous_motion(0)` 广播触发同步启动 → 设超时切 WAIT
- **WAIT 阶段**：轮询 `system_ms >= motion_timeout`，主循环自由运行（按键持续响应）
- `WAIT_RESPONSE()` 宏仍是阻塞的（轮询 `rxFrameFlag`，超时 1000000 周期），但每个 ACTION 阶段仅 ~2ms

### 运动参数（V3.1 值，与 V1.3.1 不同）

| 常量 | 值 | 说明 |
|------|-----|------|
| X1 | 3200*10.05 | X 轴取箱位 |
| X2 | 3200*1.28 | X 轴箱间距 |
| X3 | 3200*5.3+100 | X 轴 test2 目标 |
| X4 | 3200*0.4 | X 轴微调 |
| Y1~Y3 | 3200*3.7 / +200 / +300 | Y 轴取箱位 |
| Y4~Y5 | 3200*1 | Y 轴置货位（两箱）|
| Y6~Y8 | 3200*2.9 / 3 / 1.1+300 | Y 轴保留位置 |

速度：X轴 VEL_RPMX=1000, ACC_VALX=100；Y轴 VEL_RPMY=500, ACC_VALY=100

## 双串口数据流

### USART1：电机控制总线（PA9-TX, PA10-RX, 115200 8N1）
1. `Emm_V5_Pos_Control()` 组装串口帧 → `usart_SendCmd()` 逐字节发送（8000 周期超时）
2. 电机响应 → RXNE 中断 → `fifo_enQueue()` 入队 `rxFIFO`
3. 总线空闲 → IDLE 中断 → FIFO 批量读入 `rxCmd[]` → `rxFrameFlag = true`
4. `WAIT_RESPONSE()` 轮询 `rxFrameFlag`（超时 1000000 周期）

**关键**：每发一个 MMCL 缓存命令前必须 `rxFrameFlag = false`，否则 `WAIT_RESPONSE()` 因残留标志立即返回

### USART2：树莓派视觉通讯（PA2-TX, PA3-RX, 115200 8N1）
- 独立于 USART1，互不干扰
- `vision_Init()` 发送 `0xAA 0x01 0x01 0xFF` 请求，阻塞等待回复
- 回复格式：`0xAA 0x08 D1~D8 0xFF`（11 字节二进制包）
  - D1~D3：摄像头1 左→右 豆子编码（1=黄豆，2=绿豆，3=白芸豆）
  - D4~D8：摄像头2 右→左 数字分布（1-5）
- 解析结果存入 `VisionData_t` 结构体，通过 `vision_GetData()` 只读访问

### 同步触发机制（MMCL）
- `snF=1`：命令缓存到 `MMCL_cmd[]`，不立即发送
- 4 个命令均缓存后，`Emm_V5_Synchronous_motion(0)` 广播触发同步启动
- V3.1 用 `motion_timeout = system_ms + 5000/10000` 代替 `delay_ms()` 等待运动完成

## Emm_V5 API 要点

所有 API 使用 USART1，内部调用 `usart_SendCmd()`。

| 函数 | 说明 |
|------|------|
| `Emm_V5_Pos_Control(addr, dir, vel, acc, clk, raF, snF)` | 位置模式控制 |
| `Emm_V5_En_Control(addr, state, snF)` | 使能/失能 |
| `Emm_V5_Stop_Now(addr, snF)` | 立即停止 |
| `Emm_V5_Synchronous_motion(addr)` | 多机同步启动（addr=0 广播） |

- `snF`: 0=立即发送，1=缓存到 MMCL
- `raF`: 0=相对位置，1=绝对位置

## 硬件资源

| 外设 | 引脚 | 用途 |
|------|------|------|
| USART1 | PA9(TX), PA10(RX) | 4 台电机共用总线（115200 8N1） |
| USART2 | PA2(TX), PA3(RX) | **树莓派视觉通讯**（二进制数据包协议） |
| GPIOA | PA0 | 按键输入（内部上拉，低电平按下） |
| TIM4 | - | 1ms 时基（非阻塞状态机核心） |
| SysTick | - | 仅 `delay_ms()` 阻塞延时用（上电初始化） |

- HSE 8MHz → PLL 72MHz, NVIC 4bit 抢占优先级
- USART1: APB2(72MHz), USART2: APB1(36MHz), TIM4: APB1(36MHz)

## Git 提交约定

从历史提交看，使用中文 Conventional Commits 格式：
- `修复: ...` — bug 修复
- `功能: ...` — 新功能
- 提交信息应简要说明改动内容和原因

## 已知问题

1. **FIFO 函数名不匹配**：`fifo.h` 声明 `fifo_initQueue()`，`fifo.c` 定义 `initQueue()`（缺前缀）。未调用（全局零初始化），如需重置 FIFO 需注意。
2. **FIFO 数据类型**：元素 `uint16_t`，USART 接收为 `uint8_t`，高字节始终为 0。
3. **响应未解析**：`main.c` 仅等待响应但不解析 `rxCmd[]` 内容。`rxCmd2[]` 由 vision.c 正确解析。
4. **死循环风险**：HardFault/异常中断中 `while(1)` 无恢复机制，需手动复位。
5. **按键调用间隔**：`button.h` 注释建议 < 20ms，`button.c` 内部 `CONSISTENT_CNT=5` 建议 < 5ms，两者不一致。
6. **文件编码 GBK**：所有中文注释文件为 GBK 编码，非 Windows 环境显示乱码，不影响逻辑。

## 注意事项

- **文件编码 GBK**（非 UTF-8），注释中文，非 Windows 环境显示乱码
- **上电顺序**：`delay_ms(500)` 等待电机初始化 → 使能 4 台电机 → `vision_Init()` 获取视觉数据
- **默认轨迹**：按键触发 `motion_OneCycle()`（3箱循环），注释说明可改为 `motion_start_test2()`（4箱轨迹）
- **修改轨迹**：只需编辑 `V3.1/APP/main.c`——修改位置常量、状态机顺序或超时时间
- **USART 中断**在 `BSP/usart.c`，**TIM4_IRQHandler** 在 `APP/stm32f10x_it.c`
- **4 台电机共用 USART1 总线**，不支持同时独立通信
- **电机供应商**：张大头闭环伺服（淘宝 zhangdatou.taobao.com，QQ 群 262438510）
