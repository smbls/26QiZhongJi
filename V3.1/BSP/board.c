#include "board.h"

/* 初始化 TIM4 为 1ms 自由运行计数器（非阻塞用） */
void timer4_init(void)
  {
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

      TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
      TIM_TimeBaseStructure.TIM_Prescaler = 36 - 1;       // 36MHz / 36 = 1MHz
      TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
      TIM_TimeBaseStructure.TIM_Period = 1000 - 1;         // 1MHz / 1000 = 1kHz = 1ms
      TIM_TimeBaseStructure.TIM_ClockDivision = 0;
      TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

      TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
      TIM_Cmd(TIM4, ENABLE);

      NVIC_InitTypeDef NVIC_InitStructure;
      NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
      NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
      NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
      NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
      NVIC_Init(&NVIC_InitStructure);
  }

/**
	* @brief   ����NVIC������
	* @param   ��
	* @retval  ��
	*/
void nvic_init(void)
{
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_Init(&NVIC_InitStructure);
}

/**
	*	@brief		����ʱ�ӳ�ʼ��
	*	@param		��
	*	@retval		��
	*/
void clock_init(void)
{
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
}

/**
	* @brief   ��ʼ��USART
	* @param   ��
	* @retval  ��
	*/
void usart_init(void)
{
/**********************************************************
***	��ʼ��USART1����
**********************************************************/
	// PA9 - USART1_TX
	GPIO_InitTypeDef  GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;				/* ����������� */
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	// PA10 - USART1_RX
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;					/* �������� */
	GPIO_Init(GPIOA, &GPIO_InitStructure);

/**********************************************************
***	��ʼ��USART1
**********************************************************/
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(USART1, &USART_InitStructure);

/**********************************************************
***	���USART1�ж�
**********************************************************/
	USART1->SR; USART1->DR;
	USART_ClearITPendingBit(USART1, USART_IT_RXNE);

/**********************************************************
***	ʹ��USART1�ж�
**********************************************************/	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);

/**********************************************************
***	ʹ��USART1
**********************************************************/
	USART_Cmd(USART1, ENABLE);

/**********************************************************
***	��ʼ��USART2����(PA2-TX, PA3-RX)
**********************************************************/
	// PA2 - USART2_TX
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;				/* ����������� */
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	// PA3 - USART2_RX
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;					/* �������� */
	GPIO_Init(GPIOA, &GPIO_InitStructure);

/**********************************************************
***	��ʼ��USART2
**********************************************************/
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(USART2, &USART_InitStructure);

/**********************************************************
***	���USART2�ж�
**********************************************************/
	USART2->SR; USART2->DR;
	USART_ClearITPendingBit(USART2, USART_IT_RXNE);

/**********************************************************
***	ʹ��USART2�ж�
**********************************************************/
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);

/**********************************************************
***	ʹ��USART2
**********************************************************/
	USART_Cmd(USART2, ENABLE);
}

/**
	*	@brief		���س�ʼ��
	*	@param		��
	*	@retval		��
	*/
void board_init(void)
{
	nvic_init();
	clock_init();
	usart_init();
	timer4_init();
}
