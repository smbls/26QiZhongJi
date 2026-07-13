#include "board.h"
#include "delay.h"
#include "usart.h"
#include "Emm_V5.h"

int main(void)
{
	uint8_t cmd2[8] = {0};

	board_init();

	delay_ms(500);

	cmd2[0] = 2;						
	cmd2[1] = 0xF6;						
	cmd2[2] = 0;						
	cmd2[3] = (uint8_t)(1000 >> 8);		
	cmd2[4] = (uint8_t)(1000 & 0xFF);	
	cmd2[5] = 100;						
	cmd2[6] = 0;						
	cmd2[7] = 0x6B;						

	Emm_V5_Vel_Control(1, 0, 1000, 100, 0);	// USART1 -> ���1
	usart2_SendCmd(cmd2, 8);					// USART2 -> ���2

	WAIT_RESPONSE();			// ��USART1��Ӧ
	WAIT_RESPONSE2();			// ��USART2��Ӧ

	while(1)
	{
	}
}
