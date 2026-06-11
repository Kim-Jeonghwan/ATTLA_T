/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Bit.h
    Version          : 00.03
    Description      : 1x PWM 구조용 간소화된 BIT 로직 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 11. (주석 표준화 및 레거시 코드 정리)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
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

/**
 * @brief      Built-In Test (BIT) 상태 변수 및 에러 구조체 초기화
 * @param      void
 * @return     void
 */
void Bit_Init(void);

/**
 * @brief      모터 및 브레이크 전류의 과전류 여부 검증 (100ms 누적 필터링)
 * @param      void
 * @return     void
 */
void Bit_OvCurrent_Check(void);

/**
 * @brief      보드 온도 센서의 과열 여부 검증 (100ms 누적 필터링)
 * @param      void
 * @return     void
 */
void Bit_OvTemperature_Check(void);

/**
 * @brief      시스템 입력 28V 전압의 과전압 여부 검증 (100ms 누적 필터링)
 * @param      void
 * @return     void
 */
void Bit_OvVoltage_Check(void);

/**
 * @brief      모터 드라이버(DRV8343) 하드웨어 nFAULT 신호 상태 감지
 * @param      void
 * @return     void
 */
void Bit_GateFault_Check(void);

/**
 * @brief      시스템의 전체 BIT 결함 플래그 및 레지스터 리셋
 * @param      Data : 1U 인 경우 리셋 실행
 * @return     void
 */
void Bit_FaultReset(Uint16 Data);

#endif // CSU_BIT_H
