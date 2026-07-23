#include "board.h"
#include "delay.h"
#include "usart.h"
#include "Emm_V5.h"
#include "button.h"
#include "vision.h"
//该工程用于对X1,X2,Y1,Y2,Y3位置的测量，是Y1与Y2之间的增量
//已经为非阻塞式工程
/* X 轴运动参数（电机1&2） */
#define POS_PULSESX1		    3200*10.05				// 位置脉冲数（默认16细分下，3200脉冲=1圈）
#define POS_PULSESX2		    3200*1.28			// 位置脉冲数（默认16细分下，3200脉冲=1圈）
#define POS_X1_1err 		    500					// 位置脉冲数（默认16细分下，3200脉冲=1圈）

//
#define POS_PULSESX3		    3200*5.3+100	// 位置脉冲数（默认16细分下，3200脉冲=1圈）
#define POS_X3_2err 		    500				// 位置脉冲数（默认16细分下，3200脉冲=1圈）
#define POS_PULSESX4		    3200*0.4		// 位置脉冲数（默认16细分下，3200脉冲=1圈）
#define VEL_RPMX				1000			// 速度（RPM）
#define ACC_VALX				100				// 加速度

/* Y 轴运动参数（电机3&4） */
#define POS_PULSESY1			3200*3.7			// 位置脉冲数（默认16细分下，3200脉冲=1圈）
#define POS_PULSESY2			3200*3.6+200		// 位置脉冲数（默认16细分下，3200脉冲=1圈）
#define POS_PULSESY3			3200*3.6+300		// 位置脉冲数（默认16细分下，3200脉冲=1圈）
//此处的Y4到Y8仅为置货区各个箱子的Y坐标位置，并非假象位置
#define POS_PULSESY4			3200*1			// 位置脉冲数（默认16细分下，3200脉冲=1圈）
#define POS_PULSESY5			3200*1			// 位置脉冲数（默认16细分下，3200脉冲=1圈）
#define POS_PULSESY6			3200*2.9		// 位置脉冲数（默认16细分下，3200脉冲=1圈）
#define POS_PULSESY7			3200*3		// 位置脉冲数（默认16细分下，3200脉冲=1圈）
#define POS_PULSESY8			3200*1.1+300		// 位置脉冲数（默认16细分下，3200脉冲=1圈）
#define VEL_RPMY				500			// 速度（RPM）
#define ACC_VALY				100			// 加速度

volatile uint32_t system_ms = 0;    // 1ms 递增一次

typedef enum {
      MOTION_IDLE,
      /* test2 轨迹状态 */
      PHASE_X3,       PHASE_X3_WAIT,
      PHASE_Y4,       PHASE_Y4_WAIT,
      PHASE_Y5,       PHASE_Y5_WAIT,
      PHASE_HOME,     PHASE_HOME_WAIT,
      /* OneCycle 轨迹状态 */
      OC_BOX1,        OC_BOX1_WAIT,
      OC_BOX2,        OC_BOX2_WAIT,
      OC_BOX3,        OC_BOX3_WAIT,
      OC_HOME,        OC_HOME_WAIT,
  } MotionState_t;
static MotionState_t motion_state = MOTION_IDLE;
static uint32_t motion_timeout = 0;

void motion_start_test2(void)
{
      motion_state = PHASE_X3;
}
void motion_start_OneCycle(void)
{
      motion_state = OC_BOX1;
}

/**
 * @brief  OneCycle 轨迹状态机（抓第1箱→第2箱→第3箱→归位）
 * @param  无
 * @retval 无
 */
static void motion_OneCycle(void)
{
    switch (motion_state)
    {
    /* --- 抓第1箱 --- */
    case OC_BOX1:
        rxFrameFlag = false;
        Emm_V5_Pos_Control(1, 1, VEL_RPMX, ACC_VALX, POS_PULSESX1+POS_X1_1err, 0, 1);   WAIT_RESPONSE();
        rxFrameFlag = false;
        Emm_V5_Pos_Control(2, 0, VEL_RPMX, ACC_VALX, POS_PULSESX1, 0, 1);   WAIT_RESPONSE();
		//Y先向左到Y1
		rxFrameFlag = false;
        Emm_V5_Pos_Control(3, 1, VEL_RPMY, ACC_VALY, POS_PULSESY1, 0, 1);   WAIT_RESPONSE();
        rxFrameFlag = false;
        Emm_V5_Pos_Control(4, 0, VEL_RPMY, ACC_VALY, POS_PULSESY1, 0, 1);   WAIT_RESPONSE();
        Emm_V5_Synchronous_motion(0);
        motion_timeout = system_ms + 10000;
        motion_state = OC_BOX1_WAIT;
        break;

    case OC_BOX1_WAIT:
        if (system_ms >= motion_timeout) motion_state = OC_BOX2;
        break;

    /* --- 抓第2箱 --- */
    case OC_BOX2:
        rxFrameFlag = false;
        Emm_V5_Pos_Control(1, 0, VEL_RPMX, ACC_VALX, POS_PULSESX2, 0, 1);   WAIT_RESPONSE();
        rxFrameFlag = false;
        Emm_V5_Pos_Control(2, 1, VEL_RPMX, ACC_VALX, POS_PULSESX2, 0, 1);   WAIT_RESPONSE();
	//Y再向右到Y2
        rxFrameFlag = false;
        Emm_V5_Pos_Control(3, 0, VEL_RPMY, ACC_VALY, POS_PULSESY2, 0, 1);   WAIT_RESPONSE();
        rxFrameFlag = false;
        Emm_V5_Pos_Control(4, 1, VEL_RPMY, ACC_VALY, POS_PULSESY2, 0, 1);   WAIT_RESPONSE();
        Emm_V5_Synchronous_motion(0);
        motion_timeout = system_ms + 10000;
        motion_state = OC_BOX2_WAIT;
        break;

    case OC_BOX2_WAIT:
        if (system_ms >= motion_timeout) motion_state = OC_BOX3;
        break;

    /* --- 抓第3箱 --- */
    case OC_BOX3:
        rxFrameFlag = false;
        Emm_V5_Pos_Control(1, 1, VEL_RPMX, ACC_VALX, POS_PULSESX2, 0, 1);   WAIT_RESPONSE();
        rxFrameFlag = false;
        Emm_V5_Pos_Control(2, 0, VEL_RPMX, ACC_VALX, POS_PULSESX2, 0, 1);   WAIT_RESPONSE();
	//Y向右到Y3
        rxFrameFlag = false;
        Emm_V5_Pos_Control(3, 0, VEL_RPMY, ACC_VALY, POS_PULSESY3, 0, 1);   WAIT_RESPONSE();
        rxFrameFlag = false;
        Emm_V5_Pos_Control(4, 1, VEL_RPMY, ACC_VALY, POS_PULSESY3, 0, 1);   WAIT_RESPONSE();
        Emm_V5_Synchronous_motion(0);
        motion_timeout = system_ms + 10000;
        motion_state = OC_BOX3_WAIT;
        break;

    case OC_BOX3_WAIT:
        if (system_ms >= motion_timeout) motion_state = OC_HOME;
        break;

    /* --- 归位 --- */
    case OC_HOME:
        
		
		//Y向左到中心（Y2中心）
		rxFrameFlag = false;
        Emm_V5_Pos_Control(3, 1, VEL_RPMY, ACC_VALY, POS_PULSESY3+POS_PULSESY2-POS_PULSESY1, 0, 1);   WAIT_RESPONSE();
        rxFrameFlag = false;
        Emm_V5_Pos_Control(4, 0, VEL_RPMY, ACC_VALY, POS_PULSESY3+POS_PULSESY2-POS_PULSESY1, 0, 1);   WAIT_RESPONSE();
		
		rxFrameFlag = false;
        Emm_V5_Pos_Control(1, 0, VEL_RPMX, ACC_VALX, POS_PULSESX1+POS_X1_1err, 0, 1);   WAIT_RESPONSE();
        rxFrameFlag = false;
        Emm_V5_Pos_Control(2, 1, VEL_RPMX, ACC_VALX, POS_PULSESX1, 0, 1);   WAIT_RESPONSE();
		Emm_V5_Synchronous_motion(0);
        motion_timeout = system_ms + 10000;
        motion_state = OC_HOME_WAIT;
        break;

    case OC_HOME_WAIT:
        if (system_ms >= motion_timeout) motion_state = MOTION_IDLE;
        break;

    default:
        break;
    }
}
void motion_state_machine(void)
{
	switch(motion_state)
	{
		case MOTION_IDLE:
          // 啥也不做
          break;

      /* --- Phase 1: X3 运动 --- */
      case PHASE_X3:
          // 阻塞发送（实际 ~2ms，可接受）
          rxFrameFlag = false;
          Emm_V5_Pos_Control(1, 0, VEL_RPMX, ACC_VALX, POS_PULSESX3, 0, 1);
          WAIT_RESPONSE();
          rxFrameFlag = false;
          Emm_V5_Pos_Control(2, 1, VEL_RPMX, ACC_VALX, POS_PULSESX3+POS_X3_2err, 0, 1);
          WAIT_RESPONSE();
          Emm_V5_Synchronous_motion(0);
          // 设超时，切等待
          motion_timeout = system_ms + 5000;
          motion_state = PHASE_X3_WAIT;
          break;

      case PHASE_X3_WAIT:
          if (system_ms >= motion_timeout) {
              motion_state = PHASE_Y4;
          }
          break;

      /* --- Phase 2: 抓第1箱 Y4 --- */
      case PHASE_Y4:
          rxFrameFlag = false;
          Emm_V5_Pos_Control(3, 0, VEL_RPMY, ACC_VALY, POS_PULSESY4, 0, 1);
          WAIT_RESPONSE();
          rxFrameFlag = false;
          Emm_V5_Pos_Control(4, 1, VEL_RPMY, ACC_VALY, POS_PULSESY4, 0, 1);
          WAIT_RESPONSE();
          Emm_V5_Synchronous_motion(0);
          motion_timeout = system_ms + 5000;
          motion_state = PHASE_Y4_WAIT;
          break;

      case PHASE_Y4_WAIT:
          if (system_ms >= motion_timeout) {
              motion_state = PHASE_Y5;
          }
          break;

      /* --- Phase 3: 抓第2箱 Y5 --- */
      case PHASE_Y5:
          rxFrameFlag = false;
          Emm_V5_Pos_Control(3, 0, VEL_RPMY, ACC_VALY, POS_PULSESY5, 0, 1);
          WAIT_RESPONSE();
          rxFrameFlag = false;
          Emm_V5_Pos_Control(4, 1, VEL_RPMY, ACC_VALY, POS_PULSESY5, 0, 1);
          WAIT_RESPONSE();
          Emm_V5_Synchronous_motion(0);
          motion_timeout = system_ms + 5000;
          motion_state = PHASE_Y5_WAIT;
          break;

      case PHASE_Y5_WAIT:
          if (system_ms >= motion_timeout) {
              motion_state = PHASE_HOME;
          }
          break;

      /* --- Phase 4: 归位 --- */
      case PHASE_HOME:
          rxFrameFlag = false;
          Emm_V5_Pos_Control(3, 1, VEL_RPMY, ACC_VALY, POS_PULSESY4+POS_PULSESY5, 0, 1);
          WAIT_RESPONSE();
          rxFrameFlag = false;
          Emm_V5_Pos_Control(4, 0, VEL_RPMY, ACC_VALY, POS_PULSESY4+POS_PULSESY5, 0, 1);
          WAIT_RESPONSE();
          rxFrameFlag = false;
          Emm_V5_Pos_Control(1, 1, VEL_RPMX, ACC_VALX, POS_PULSESX3, 0, 1);
          WAIT_RESPONSE();
          rxFrameFlag = false;
          Emm_V5_Pos_Control(2, 0, VEL_RPMX, ACC_VALX, POS_PULSESX3+POS_X3_2err, 0, 1);
          WAIT_RESPONSE();
          Emm_V5_Synchronous_motion(0);
          motion_timeout = system_ms + 10000;
          motion_state = PHASE_HOME_WAIT;
          break;

      case PHASE_HOME_WAIT:
          if (system_ms >= motion_timeout) {
              motion_state = MOTION_IDLE;
          }
          break;
      }
}

int main(void)
{
	board_init();
	button_Init();
	delay_ms(500);	// 等待电机初始化完成

	/* 使能四台电机 */
	Emm_V5_En_Control(1, 1, 0);	WAIT_RESPONSE();
	Emm_V5_En_Control(2, 1, 0);	WAIT_RESPONSE();
	Emm_V5_En_Control(3, 1, 0);	WAIT_RESPONSE();
	Emm_V5_En_Control(4, 1, 0);	WAIT_RESPONSE();
	delay_ms(100);

	/* 获取树莓派视觉数据 */
	vision_Init();

	while(1)
	{

		if (button_Scan() == BUTTON_EVENT_PRESSED && motion_state == MOTION_IDLE)
			motion_start_OneCycle();	// 改为 motion_start_test2() 可切回原来的4箱轨迹

		/* OC_ 状态 → OneCycle 状态机，其他 → test2 状态机 */
		if (motion_state >= OC_BOX1 && motion_state <= OC_HOME_WAIT)
			motion_OneCycle();
		else
			motion_state_machine();
	}
}
