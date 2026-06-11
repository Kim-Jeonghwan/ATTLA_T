/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Bit.h
    Version          : 00.02
    Description      : 1x PWM 구조용 간소화된 BIT 로직 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 11. (함수명 명명 규칙 위반 접두어 제거)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 * 2026. 06. 11. - 함수명 접두어(csu_, hal_) 제거 리팩토링
 */

#ifndef CSU_BIT_H
#define CSU_BIT_H

#include "main.h"

extern Uint32  Bit_Inform_all;
extern Uint16  BitStartFlag_Set;
extern Uint16  BitFaultFlag_Set;

extern Uint16  BitFault_OvCurr_Mot;
extern Uint16  BitFault_OvCurr_Brk;
extern Uint16  BitFault_OvTemp_Bd;
extern Uint16  BitFault_OvVolt_28V;
extern Uint16  BitFault_Drv8343_nFault;

void Bit_OvCurrent_Check(void);
void Bit_OvTemperature_Check(void);
void Bit_OvVoltage_Check(void);
void Bit_GateFault_Check(void);
void Bit_FaultReset(Uint16 Data);

#endif // CSU_BIT_H
