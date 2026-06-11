/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Control.h
    Version          : 00.00
    Description      : 시스템 제어 모듈 (PBIT, CBIT, 오프셋 조정 등) 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 11. (신규 생성)
**********************************************************************/
#ifndef CSU_CONTROL_H
#define CSU_CONTROL_H

#include "main.h"

extern volatile uint16_t isOffsetCalibrated;
extern volatile uint16_t isPbitComplete;

void csu_Control_CalibrateCurrentOffset(void);
void csu_Bit_RunPBIT(void);
void csu_Bit_RunCBIT(void);
void csu_Control_SystemOperation(void);

#endif // CSU_CONTROL_H
