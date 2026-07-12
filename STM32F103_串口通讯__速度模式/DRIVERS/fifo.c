#include "fifo.h"

/**********************************************************
***	Emm_V5.0�����ջ���������
***	��д���ߣ�ZHANGDATOU
***	����֧�֣��Ŵ�ͷ�ջ��ŷ�
***	�Ա����̣�https://zhangdatou.taobao.com
***	CSDN���ͣ�http s://blog.csdn.net/zhangdatou666
***	qq����Ⱥ��262438510
**********************************************************/

__IO FIFO_t rxFIFO = {0};
__IO FIFO_t rxFIFO2 = {0};

/**
	* @brief   ��ʼ������
	* @param   ��
	* @retval  ��
	*/
void initQueue(void)
{
	rxFIFO.ptrRead  = 0;
	rxFIFO.ptrWrite = 0;
}

/**
	* @brief   ���
	* @param   ��
	* @retval  ��
	*/
void fifo_enQueue(uint16_t data)
{
	rxFIFO.buffer[rxFIFO.ptrWrite] = data;
	
	++rxFIFO.ptrWrite;
	
	if(rxFIFO.ptrWrite >= FIFO_SIZE)
	{
		rxFIFO.ptrWrite = 0;
	}
}

/**
	* @brief   ����
	* @param   ��
	* @retval  ��
	*/
uint16_t fifo_deQueue(void)
{
	uint16_t element = 0;

	element = rxFIFO.buffer[rxFIFO.ptrRead];

	++rxFIFO.ptrRead;

	if(rxFIFO.ptrRead >= FIFO_SIZE)
	{
		rxFIFO.ptrRead = 0;
	}

	return element;
}

/**
	* @brief   �жϿն���
	* @param   ��
	* @retval  ��
	*/
bool fifo_isEmpty(void)
{
	if(rxFIFO.ptrRead == rxFIFO.ptrWrite)
	{
		return true;
	}

	return false;
}

/**
	* @brief   ������г���
	* @param   ��
	* @retval  ��
	*/
uint16_t fifo_queueLength(void)
{
	if(rxFIFO.ptrRead <= rxFIFO.ptrWrite)
	{
		return (rxFIFO.ptrWrite - rxFIFO.ptrRead);
	}
	else
	{
		return (FIFO_SIZE - rxFIFO.ptrRead + rxFIFO.ptrWrite);
	}
}

/**********************************************************
*** FIFO2���в���
**********************************************************/
void fifo_enQueue2(uint16_t data)
{
	rxFIFO2.buffer[rxFIFO2.ptrWrite] = data;

	++rxFIFO2.ptrWrite;

	if(rxFIFO2.ptrWrite >= FIFO_SIZE)
	{
		rxFIFO2.ptrWrite = 0;
	}
}

uint16_t fifo_deQueue2(void)
{
	uint16_t element = 0;

	element = rxFIFO2.buffer[rxFIFO2.ptrRead];

	++rxFIFO2.ptrRead;

	if(rxFIFO2.ptrRead >= FIFO_SIZE)
	{
		rxFIFO2.ptrRead = 0;
	}

	return element;
}

bool fifo_isEmpty2(void)
{
	if(rxFIFO2.ptrRead == rxFIFO2.ptrWrite)
	{
		return true;
	}

	return false;
}

uint16_t fifo_queueLength2(void)
{
	if(rxFIFO2.ptrRead <= rxFIFO2.ptrWrite)
	{
		return (rxFIFO2.ptrWrite - rxFIFO2.ptrRead);
	}
	else
	{
		return (FIFO_SIZE - rxFIFO2.ptrRead + rxFIFO2.ptrWrite);
	}
}
