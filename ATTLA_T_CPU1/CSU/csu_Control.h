/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Control.h
    Version          : 00.01
    Description      : 시스템 제어 모듈 (PBIT, CBIT, 오프셋 조정 등) 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 11. (함수명 명명 규칙 위반 접두어 제거)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 * 2026. 06. 11. - 함수명 접두어(csu_, hal_) 제거 리팩토링
 */

#ifndef CSU_CONTROL_H
#define CSU_CONTROL_H

#include "main.h"

extern volatile uint16_t isOffsetCalibrated;
extern volatile uint16_t isPbitComplete;

void Control_CalibrateCurrentOffset(void);
void Bit_RunPBIT(void);
void Bit_RunCBIT(void);
void Control_SystemOperation(void);

#endif // CSU_CONTROL_H
