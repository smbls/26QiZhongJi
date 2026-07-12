#ifndef __USART_H
#define __USART_H

#include "board.h"
#include "fifo.h"

/**********************************************************
***	Emm_V5.0�����ջ���������
***	��д���ߣ�ZHANGDATOU
***	����֧�֣��Ŵ�ͷ�ջ��ŷ�
***	�Ա����̣�https://zhangdatou.taobao.com
***	CSDN���ͣ�http s://blog.csdn.net/zhangdatou666
***	qq����Ⱥ��262438510
**********************************************************/

extern __IO bool rxFrameFlag;
extern __IO uint8_t rxCmd[FIFO_SIZE];
extern __IO uint8_t rxCount;

extern __IO bool rxFrameFlag2;
extern __IO uint8_t rxCmd2[FIFO_SIZE];
extern __IO uint8_t rxCount2;

void usart_SendCmd(__IO uint8_t *cmd, uint8_t len);
void usart_SendByte(uint16_t data);
void usart2_SendCmd(__IO uint8_t *cmd, uint8_t len);
void usart2_SendByte(uint16_t data);

/**********************************************************
***	��Ӧ�ȴ���(����ʱ�������ֹ����)
**********************************************************/
#define WAIT_RESPONSE()		do { \
	uint32_t __to = 0; \
	while(rxFrameFlag == false) { \
		if(++__to > 1000000) { break; } \
	} \
	rxFrameFlag = false; \
} while(0)

#define WAIT_RESPONSE2()	do { \
	uint32_t __to = 0; \
	while(rxFrameFlag2 == false) { \
		if(++__to > 1000000) { break; } \
	} \
	rxFrameFlag2 = false; \
} while(0)

#endif
