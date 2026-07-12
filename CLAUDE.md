# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述

本项目是 ZDT XS 系列第二代闭环步进电机的 STM32F103 串口通讯示例（速度模式），使用 Keil MDK 开发，基于 STM32F10x 标准外设库 V3.5.0。

## 构建方式

本项目使用 **Keil MDK**（µVision）进行编译，工程文件位于：
- `STM32F103_串口通讯__速度模式/PRJ/STM32_UART_CMD.uvprojx`

无命令行构建工具，需用 Keil µVision 打开 `.uvprojx` 文件后进行编译、下载和调试。调试器配置使用 JLink（见 `PRJ/JLinkSettings.ini`）。

## 代码架构

```
STM32F103_串口通讯__速度模式/
├── APP/          # 应用层
│   ├── main.c              # 主程序入口
│   ├── stm32f10x_it.c/h    # 中断服务函数（非USART中断）
│   └── stm32f10x_conf.h    # 标准库功能使能配置
├── BSP/          # 板级支持包
│   ├── board.c/h           # 硬件初始化（NVIC、时钟、USART）
│   ├── usart.c/h           # USART1 收发驱动 + USART1_IRQHandler
│   └── Emm_V5.c/h          # ZDT Emm V5 电机控制 API
├── DRIVERS/      # 底层驱动
│   ├── delay.c/h           # 延时函数
│   └── fifo.c/h            # 环形缓冲区（FIFO_SIZE=128, uint16_t 元素）
├── CMSIS/        # ARM Cortex-M3 内核文件 + 启动文件
└── LIB/          # STM32F10x 标准外设库 V3.5.0
```

## 数据流

1. `board_init()` → `nvic_init()` → `clock_init()` → `usart_init()`
   - HSE 8MHz 外部晶振，PLL 倍频至 72MHz
   - USART1: 115200-8N1, PA9-TX(推挽复用), PA10-RX(上拉输入)
   - 开启 USART1 的 RXNE 和 IDLE 中断
2. `Emm_V5_xxx()` → 组装串口命令帧 → `usart_SendCmd()` → `usart_SendByte()` 逐字节发送（含 8000 周期超时保护）
3. 电机响应：
   - 每个字节 → USART1 RXNE 中断 → `fifo_enQueue()` 入队
   - 帧结束（总线空闲）→ IDLE 中断触发 → 从 FIFO 批量读入 `rxCmd[]` → `rxFrameFlag = true`
4. `main.c` 发送速度命令后，通过 `while(rxFrameFlag == false); rxFrameFlag = false;` 同步等待电机响应

## 串口通讯协议

- **波特率**：115200，8N1
- **命令帧格式**：`[addr][功能码][数据...][0x6B]`，末尾固定校验字节 `0x6B`
- **响应帧检测**：使用 USART1 IDLE 空闲线中断判断一帧接收完毕
- **接收变量**：`rxCmd[]` 存储原始字节，`rxCount` 为字节数，`rxFrameFlag` 为帧完成标志

## Emm_V5 API 说明

`BSP/Emm_V5.h` 提供两组函数：

- **普通版**（`Emm_V5_xxx`）：直接通过 `usart_SendCmd()` 发送命令
- **MMCL 版**（`Emm_V5_MMCL_xxx`）：将命令缓存至 `MMCL_cmd[]` 数组，用于多轴同步运动，再统一触发 `Emm_V5_Synchronous_motion()`

### 常用运动控制函数

| 函数 | 说明 |
|------|------|
| `Emm_V5_Vel_Control(addr, dir, vel, acc, snF)` | 速度模式控制 |
| `Emm_V5_Pos_Control(addr, dir, vel, acc, clk, raF, snF)` | 位置模式控制（`raF`: 0=相对位置，1=绝对位置） |
| `Emm_V5_Stop_Now(addr, snF)` | 立即停止 |
| `Emm_V5_En_Control(addr, state, snF)` | 使能/失能电机（state: 1=使能，0=失能） |

- `addr`：电机地址（1字节），`snF`：同步标志位（0=立即发送，1=缓存到 MMCL）
- `dir`：方向（0=CW, 1=CCW），`vel`：速度（RPM），`acc`：加速度

### 系统参数读取

`SysParams_t` 枚举定义了一系列可读取的系统参数，通过 `Emm_V5_Read_Sys_Params(addr, param)` 读取：

- `S_VBUS(5)` — 总线电压，`S_CBUS(6)` — 总线电流
- `S_VEL(14)` — 实时转速，`S_CPOS(15)` — 实时位置
- `S_ENCO(8)` — 编码器原始值，`S_PERR(16)` — 位置误差
- `S_FLAG(19)` — 驱动状态标志位，`S_OFLAG(20)` — 电机状态标志位
- 完整列表见 `Emm_V5.h:17-36`

## 已发现的潜在问题

1. **FIFO 函数名不匹配**：`fifo.h:25` 声明 `fifo_initQueue()`，但 `fifo.c:19` 定义的是 `initQueue()`（缺少 `fifo_` 前缀）。当前代码未调用此函数（初始化时结构体为全局零初始化），如需重置 FIFO 需注意此问题。
2. **FIFO 数据类型**：FIFO 缓冲区元素为 `uint16_t`（`fifo.h:17`），但 USART 接收数据为 `uint8_t`，高字节始终为 0，使用上无影响。
3. **响应处理**：`main.c` 当前仅等待电机响应但不解析数据。如需处理电机返回的状态数据，需在收到 `rxFrameFlag` 后解析 `rxCmd[]`。

## 关键注意事项

- **文件编码**：所有源文件编码为 GBK（非 UTF-8），注释为中文，在非 Windows 环境下显示为乱码，不影响代码逻辑
- **上电延时**：上电后需等待 500ms 让电机完成初始化再发送命令（见 `main.c` 第 30 行 `delay_ms(500)`）
- **修改位置**：电机控制逻辑只需编辑 `APP/main.c`，驱动层无需改动
- **支持信息**：电机供应商为"张大头闭环伺服"，淘宝店：zhangdatou.taobao.com，QQ 群：262438510
