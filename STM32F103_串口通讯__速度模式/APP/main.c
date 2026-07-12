#include "board.h"
#include "delay.h"
#include "usart.h"
#include "Emm_V5.h"


int main(void)
{

	board_init();

	delay_ms(500);

	Emm_V5_MMCL_Vel_Control(1, 0, 1000, 100, 0);	// ������1: CW, 1000RPM, acc=100
	Emm_V5_MMCL_Vel_Control(2, 1, 1000, 100, 0);	// ������2: CW, 1000RPM, acc=100

	Emm_V5_Multi_Motor_Cmd(0);

	WAIT_RESPONSE();

	while(1)
	{
	}
}
