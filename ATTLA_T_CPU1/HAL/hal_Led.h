/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : hal_Led.h
    Version          : 00.01
    Description      : 시스템 상태 표시 LED 하드웨어 제어 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 09. (신규 생성)
**********************************************************************/

#ifndef HAL_LED_H
#define HAL_LED_H

#include "main.h"

void hal_Led_InitGpio(void);
void hal_Led_WritePin(uint16_t Index, bool State);
void hal_Led_TogglePin(uint16_t Index);

#endif // HAL_LED_H
