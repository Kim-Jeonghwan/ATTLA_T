/**********************************************************************
 Nexcom Co., Ltd.
 Filename         : hal_Led.h
 Version          : 00.02
 Description      : 시스템 상태 표시 LED 하드웨어 제어 헤더
 Programmer       : Kim Jeonghwan
 Last Updated     : 2026. 06. 11. (주석 표준화 및 레거시 코드 정리)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 * 2026. 06. 11. - 함수명 접두어(csu_, hal_) 제거 리팩토링
 */


#ifndef HAL_LED_H
#define HAL_LED_H

#include "main.h"

/**
 * @brief      LED 출력 핀(GPIO30) 초기화
 * @param      void
 * @return     void
 */
void Led_InitGpio(void);

/**
 * @brief      특정 LED 핀의 출력 값 설정
 * @param      Index : 제어 대상 LED 인덱스 (eLED_nG)
 * @param      State : 출력 상태 (true: High, false: Low)
 * @return     void
 */
void Led_WritePin(uint16_t Index, bool State);

/**
 * @brief      특정 LED 핀의 출력 상태 반전 (Toggling)
 * @param      Index : 제어 대상 LED 인덱스 (eLED_nG)
 * @return     void
 */
void Led_TogglePin(uint16_t Index);

#endif // HAL_LED_H
