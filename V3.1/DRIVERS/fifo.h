#ifndef __FIFO_H
#define __FIFO_H

#include "stm32f10x.h"

/**********************************************************
***	Emm_V5.0�����ջ���������
***	��д���ߣ�ZHANGDATOU
***	����֧�֣��Ŵ�ͷ�ջ��ŷ�
***	�Ա����̣�https://zhangdatou.taobao.com
***	CSDN���ͣ�http s://blog.csdn.net/zhangdatou666
***	qq����Ⱥ��262438510
**********************************************************/

#define 	FIFO_SIZE   128
typedef struct {
	uint16_t buffer[FIFO_SIZE];
	__IO uint8_t ptrWrite;
	__IO uint8_t ptrRead;
}FIFO_t;

extern __IO FIFO_t rxFIFO;
extern __IO FIFO_t rxFIFO2;

void fifo_initQueue(void);
void fifo_enQueue(uint16_t data);
uint16_t fifo_deQueue(void);
bool fifo_isEmpty(void);
uint16_t fifo_queueLength(void);

void fifo_enQueue2(uint16_t data);
uint16_t fifo_deQueue2(void);
bool fifo_isEmpty2(void);
uint16_t fifo_queueLength2(void);

#endif
