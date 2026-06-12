/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Bit.h
    Version          : 00.05
    Description      : 1x PWM 구조용 간소화된 BIT 로직 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 12. (csu_Bit.c 내부의 매크로 상수를 통합)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 12. - csu_Bit.c 내부의 매크로 상수를 통합
 * 2026. 06. 11. - BIT 임계값 매크로 집중화 및 신규 결함 구조체/함수 추가
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 상태 변수들을 stBitState 구조체(xBit)로 통합
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 * 2026. 06. 11. - 함수명 접두어(csu_, hal_) 제거 리팩토링
 */

#ifndef CSU_BIT_H
#define CSU_BIT_H

#include "main.h"

// 기존 임계값 및 신규 임계값 정의
#define BIT_LIMIT_OVC_MOT_MAX       10.0f    // 모터 과전류 (10A)
#define BIT_LIMIT_OVC_BRK_MAX       1.5f     // 브레이크 과전류 (1.5A)
#define BIT_LIMIT_OVT_BD_MAX        80.0f    // 보드 과열 (80도)
#define BIT_LIMIT_OVV_28V_MAX       32.0f    // 28V 과전압 (32V)

// 신규 스톨 및 과속, 전원 감시 임계값 (헤더 집중화)
#define BIT_LIMIT_STALL_CURR_MIN    5.0f     // 스톨 판단 전류 하한치 (5.0A)
#define BIT_LIMIT_STALL_RPM_LIMIT   10.0f    // 스톨 판단 속도 상한치 (10.0 RPM)
#define BIT_LIMIT_STALL_TIME_CNT    10000U   // 스톨 반응 시간 (TDU 기준 1.0초, 100us * 10000)

#define BIT_LIMIT_SPEED_MOT_MAX     3500.0f  // 모터 과속 제한 (3500 RPM)
#define BIT_LIMIT_SPEED_MOT_MIN     -3500.0f

#define BIT_LIMIT_OVS_TIME_CNT      1000U    // 과속 감지 지연시간 (100ms, 100us * 1000)
#define BIT_LIMIT_OVV_BRK_TIME_CNT  1000U    // 브레이크 전압 감지 지연시간 (100ms, 100us * 1000)
#define BIT_CNT_REF_100MS           1000U    // 공통 100ms 누적 필터 카운트 (100us * 1000)

typedef struct {
    Uint32 informAll;
    Uint16 startFlagSet;
    Uint16 faultFlagSet;
    Uint16 faultOvCurrMot;
    Uint16 faultOvCurrBrk;
    Uint16 faultOvTempBd;
    Uint16 faultOvVolt28V;
    Uint16 faultOvVoltBrk;
    Uint16 faultDrv8343nFault;
    Uint16 faultStall;
    Uint16 faultOverSpeed;
    Uint16 faultEncError;
    Uint16 warnEncWarning;
    Uint16 stallCheckCnt;
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
 * @brief      모터의 스톨 상태 감지 (전류 5A 초과 및 속도 10 RPM 미만이 1.0초 지속)
 * @param      void
 * @return     void
 */
void Bit_MotorStall_Check(void);

/**
 * @brief      모터의 과속 상태 감지 (절대 속도 3500 RPM 초과가 100ms 지속)
 * @param      void
 * @return     void
 */
void Bit_MotorOverSpeed_Check(void);

/**
 * @brief      엔코더 상태 이상 감지 (에러 및 워닝 비트 검출)
 * @param      void
 * @return     void
 */
void Bit_Encoder_Check(void);

/**
 * @brief      시스템의 전체 BIT 결함 플래그 및 레지스터 리셋
 * @param      Data : 1U 인 경우 리셋 실행
 * @return     void
 */
void Bit_FaultReset(Uint16 Data);

#endif // CSU_BIT_H
