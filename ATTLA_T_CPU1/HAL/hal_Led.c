/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : hal_Led.c
    Version          : 00.02
    Description      : 시스템 상태 표시 LED 하드웨어 제어 로직
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 11. (함수명 명명 규칙 위반 접두어 제거)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 * 2026. 06. 11. - 함수명 접두어(csu_, hal_) 제거 리팩토링
 */


#include "hal_Led.h"

void Led_InitGpio(void)
{
    EALLOW;
    
    // nG LED (GPIO30)
    GPIO_setPinConfig(GPIO_30_GPIO30);
    GPIO_setPadConfig(30u, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(30u, GPIO_DIR_MODE_OUT);
    GPIO_setMasterCore(30u, GPIO_CORE_CPU1);

    EDIS;
}

void Led_WritePin(uint16_t Index, bool State)
{
	switch(Index)
	{
	case eLED_nG:
		GPIO_writePin(eLED_nG, (uint32_t)State);
		break;

	// case eLED_ERROR:
	// 	GPIO_writePin(eLED_ERROR, (uint32_t)State);
	// 	break;

	default:
		// MISRA
		break;
	}
}

void Led_TogglePin(uint16_t Index)
{
	switch(Index)
	{
	case eLED_nG:
		GPIO_togglePin(eLED_nG);
		break;

	// case eLED_ERROR:
	// 	GPIO_togglePin(eLED_ERROR);
	// 	break;

	default:
		// MISRA
		break;
	}
}
