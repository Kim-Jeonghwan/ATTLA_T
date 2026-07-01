/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Bit.h
    Version          : 00.09
    Description      : 1x PWM 구조용 간소화된 BIT 로직 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 07. 01. (구조체 변수 상세 한글 주석 추가)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 07. 01. - 구조체 변수 상세 한글 주석 추가 (코딩 규칙 적용)
 * 2026. 06. 23. - main.h -> main_cpu1.h 인클루드 명칭 리팩토링
 * 2026. 06. 12. - 과속 판단 기준을 모터 정격과 동일한 3240 RPM으로 하향 조정
 * 2026. 06. 12. - 매크로 상수명 추상화: BIT_CNT_FILTER_REF 반영
 * 2026. 06. 12. - csu_Bit.c 내부의 매크로 상수를 통합
 * 2026. 06. 11. - BIT 임계값 매크로 집중화 및 신규 결함 구조체/함수 추가
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 상태 변수들을 stBitState 구조체(xBit)로 통합
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 * 2026. 06. 11. - 함수명 접두어(csu_, hal_) 제거 리팩토링
 */

#ifndef CSU_BIT_H
#define CSU_BIT_H

#include "main_cpu1.h"

// BIT 디버깅 튜닝 파라미터 구조체
typedef struct {
    float32_t ovcMotMax;         // 모터 과전류 판단 임계값 (단위: A)
    float32_t ovcBrkMax;         // 브레이크 과전류 판단 임계값 (단위: A)
    float32_t ovtBdMax;          // 보드 과열 판단 임계값 (단위: °C)
    float32_t ovv28VMax;         // 28V 과전압 판단 임계값 (단위: V)
    float32_t stallCurrMin;      // 스톨 판단을 위한 모터 전류 하한치 (단위: A)
    float32_t stallRpmLimit;     // 스톨 판단을 위한 모터 속도 상한치 (단위: RPM)
    Uint16 stallTimeCnt;         // 스톨 상태 유지 판단 시간 (단위: 100us 틱 카운트)
    float32_t speedMotMax;       // 모터 정방향 과속 제한 임계값 (단위: RPM)
    float32_t speedMotMin;       // 모터 역방향 과속 제한 임계값 (단위: RPM)
    Uint16 ovsTimeCnt;           // 과속 감지 판단 지연시간 (단위: 100us 틱 카운트)
    Uint16 ovvBrkTimeCnt;        // 브레이크 전압 감지 판단 지연시간 (단위: 100us 틱 카운트)
    Uint16 cntFilterRef;         // 100ms 누적 필터링 판단 기준값 (단위: 100us 틱 카운트)
} stBitLimit;

extern stBitLimit xBitLimit;

typedef struct {
    Uint32 informAll;            // 전체 결함 상태를 비트맵 형태로 모아둔 변수 (상위 통신용)
    Uint16 startFlagSet;         // 시스템 시작 정상 플래그 (1: 정상)
    Uint16 faultFlagSet;         // 전체 결함 통합 플래그 (1: 하나 이상의 결함 발생)
    Uint16 faultOvCurrMot;       // 모터 과전류 결함 플래그 (1: 결함)
    Uint16 faultOvCurrBrk;       // 브레이크 과전류 결함 플래그 (1: 결함)
    Uint16 faultOvTempBd;        // 보드 과열 결함 플래그 (1: 결함)
    Uint16 faultOvVolt28V;       // 28V 전원 과전압 결함 플래그 (1: 결함)
    Uint16 faultOvVoltBrk;       // 브레이크 구동 전압 이상 결함 플래그 (1: 결함)
    Uint16 faultDrv8343nFault;   // DRV8343 모터 드라이버 자체 하드웨어 결함 플래그 (1: 결함)
    Uint16 faultStall;           // 모터 구속(Stall) 결함 플래그 (1: 결함)
    Uint16 faultOverSpeed;       // 모터 과속 결함 플래그 (1: 결함)
    Uint16 faultEncError;        // 엔코더 통신/데이터 에러 결함 플래그 (1: 결함)
    Uint16 warnEncWarning;       // 엔코더 상태 워닝 플래그 (1: 경고)
    Uint16 stallCheckCnt;        // 구속(Stall) 상태 카운팅 변수 (단위: 100us 틱)
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
 * @brief      모터의 과속 상태 감지 (절대 속도 3240 RPM 초과가 100ms 지속)
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
