#include "usart.h"

/**********************************************************
***	Emm_V5.0�����ջ���������
***	��д���ߣ�ZHANGDATOU
***	����֧�֣��Ŵ�ͷ�ջ��ŷ�
***	�Ա����̣�https://zhangdatou.taobao.com
***	CSDN���ͣ�http s://blog.csdn.net/zhangdatou666
***	qq����Ⱥ��262438510
**********************************************************/

__IO bool rxFrameFlag = false;
__IO uint8_t rxCmd[FIFO_SIZE] = {0};
__IO uint8_t rxCount = 0;

__IO bool rxFrameFlag2 = false;
__IO uint8_t rxCmd2[FIFO_SIZE] = {0};
__IO uint8_t rxCount2 = 0;

/**
	* @brief   USART1�жϺ���
	* @param   ��
	* @retval  ��
	*/
void USART1_IRQHandler(void)
{
	__IO uint16_t i = 0;

/**********************************************************
***	���ڽ����ж�
**********************************************************/
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		// δ���һ֡���ݽ��գ����ݽ��뻺�����
		fifo_enQueue((uint8_t)USART1->DR);

		// ������ڽ����ж�
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}

/**********************************************************
***	���ڿ����ж�
**********************************************************/
	else if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
	{
		// �ȶ�SR�ٶ�DR�����IDLE�ж�
		USART1->SR; USART1->DR;

		// ��ȡһ֡��������
		rxCount = fifo_queueLength(); for(i=0; i < rxCount; i++) { rxCmd[i] = fifo_deQueue(); }

		// һ֡���ݽ�����ɣ���λ֡��־λ
		rxFrameFlag = true;
	}
}

/**
	* @brief   USART2�жϺ���
	* @param   ��
	* @retval  ��
	*/
void USART2_IRQHandler(void)
{
	__IO uint16_t i = 0;

/**********************************************************
***	���ڽ����ж�
**********************************************************/
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
	{
		// δ���һ֡���ݽ��գ����ݽ��뻺�����
		fifo_enQueue2((uint8_t)USART2->DR);

		// ������ڽ����ж�
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
	}

/**********************************************************
***	���ڿ����ж�
**********************************************************/
	else if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
	{
		// �ȶ�SR�ٶ�DR�����IDLE�ж�
		USART2->SR; USART2->DR;

		// ��ȡһ֡��������
		rxCount2 = fifo_queueLength2(); for(i=0; i < rxCount2; i++) { rxCmd2[i] = fifo_deQueue2(); }

		// һ֡���ݽ�����ɣ���λ֡��־λ
		rxFrameFlag2 = true;
	}
}

/**
	* @brief   USART���Ͷ���ֽ�
	* @param   ��
	* @retval  ��
	*/
void usart_SendCmd(__IO uint8_t *cmd, uint8_t len)
{
	__IO uint8_t i = 0;
	
	for(i=0; i < len; i++) { usart_SendByte(cmd[i]); }
}

/**
	* @brief   USART����һ���ֽ�
	* @param   ��
	* @retval  ��
	*/
void usart_SendByte(uint16_t data)
{
	__IO uint16_t t0 = 0;

	USART1->DR = (data & (uint16_t)0x01FF);

	while(!(USART1->SR & USART_FLAG_TXE))
	{
		++t0; if(t0 > 8000)	{	return; }
	}
}

/**
	* @brief   USART2���Ͷ���ֽ�
	* @param   ��
	* @retval  ��
	*/
void usart2_SendCmd(__IO uint8_t *cmd, uint8_t len)
{
	__IO uint8_t i = 0;

	for(i=0; i < len; i++) { usart2_SendByte(cmd[i]); }
}

/**
	* @brief   USART2����һ���ֽ�
	* @param   ��
	* @retval  ��
	*/
void usart2_SendByte(uint16_t data)
{
	__IO uint16_t t0 = 0;

	USART2->DR = (data & (uint16_t)0x01FF);

	while(!(USART2->SR & USART_FLAG_TXE))
	{
		++t0; if(t0 > 8000)	{	return; }
	}
}


