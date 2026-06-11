/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : hal_Led.h
    Version          : 00.02
    Description      : 시스템 상태 표시 LED 하드웨어 제어 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 11. (함수명 명명 규칙 위반 접두어 제거)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 * 2026. 06. 11. - 함수명 접두어(csu_, hal_) 제거 리팩토링
 */


#ifndef HAL_LED_H
#define HAL_LED_H

#include "main.h"

void Led_InitGpio(void);
void Led_WritePin(uint16_t Index, bool State);
void Led_TogglePin(uint16_t Index);

#endif // HAL_LED_H
