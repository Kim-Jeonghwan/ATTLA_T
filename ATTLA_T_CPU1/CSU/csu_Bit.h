/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Bit.h
    Version          : 00.03
    Description      : 1x PWM 구조용 간소화된 BIT 로직 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 11. (전역 변수 구조체화 마이그레이션)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 11. - 상태 변수들을 stBitState 구조체(xBit)로 통합
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 * 2026. 06. 11. - 함수명 접두어(csu_, hal_) 제거 리팩토링
 */

#ifndef CSU_BIT_H
#define CSU_BIT_H

#include "main.h"

typedef struct {
    Uint32 informAll;
    Uint16 startFlagSet;
    Uint16 faultFlagSet;
    Uint16 faultOvCurrMot;
    Uint16 faultOvCurrBrk;
    Uint16 faultOvTempBd;
    Uint16 faultOvVolt28V;
    Uint16 faultDrv8343nFault;
} stBitState;

extern stBitState xBit;

void Bit_Init(void);
void Bit_OvCurrent_Check(void);
void Bit_OvTemperature_Check(void);
void Bit_OvVoltage_Check(void);
void Bit_GateFault_Check(void);
void Bit_FaultReset(Uint16 Data);

#endif // CSU_BIT_H
