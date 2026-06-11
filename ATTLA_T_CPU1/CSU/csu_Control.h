/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Control.h
    Version          : 00.02
    Description      : 시스템 제어 모듈 (PBIT, CBIT, 오프셋 조정 등) 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 11. (전역 변수 구조체화 마이그레이션)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 11. - 상태 변수들을 stControlState 구조체(xSysCtrl)로 통합
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 * 2026. 06. 11. - 함수명 접두어(csu_, hal_) 제거 리팩토링
 */

#ifndef CSU_CONTROL_H
#define CSU_CONTROL_H

#include "main.h"

typedef struct {
    uint16_t isOffsetCalibrated;
    uint16_t isPbitComplete;
} stControlState;

extern volatile stControlState xSysCtrl;

void Control_Init(void);
void Control_CalibrateCurrentOffset(void);
void Bit_RunPBIT(void);
void Bit_RunCBIT(void);
void Control_SystemOperation(void);

#endif // CSU_CONTROL_H
