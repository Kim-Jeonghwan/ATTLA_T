/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_LimitSwitch.h
    Version          : 00.00
    Description      : 리미트 스위치 상태 감지 및 고장 진단 모듈 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 22. (최초 작성)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 22. - 파일 생성 및 기본 구조 작성
 */

#ifndef CSU_LIMITSWITCH_H
#define CSU_LIMITSWITCH_H

#include "main.h"

// 리미트 스위치 고장 코드 열거형
typedef enum {
    LS_FAULT_NONE = 0,
    LS_FAULT_SW1_BROKEN,        // 스위치1 단선 또는 하드웨어 고장 (NO == NC)
    LS_FAULT_SW2_BROKEN,        // 스위치2 단선 또는 하드웨어 고장 (NO == NC)
    LS_FAULT_POS1_MISMATCH,     // 목표 1 위치 조건과 스위치 상태 불일치
    LS_FAULT_POS2_MISMATCH      // 목표 2 위치 조건과 스위치 상태 불일치
} LimitSwitchFaultCode_t;

// 리미트 스위치 설정 데이터 구조체
typedef struct {
    float32_t targetPos1;             // 목표 1 위치 (단위: 디그리 또는 mm 등 시스템 기준, 나중에 사용자가 수정할 값)
    float32_t targetPos2;             // 목표 2 위치 (단위 동일)
    float32_t nearTolerance1;         // 목표 1 근접 판정 오차 허용치
    float32_t nearTolerance2;         // 목표 2 근접 판정 오차 허용치
    uint16_t mappedSwitchForTarget1;  // 목표 1에 매핑될 스위치 번호 (1 또는 2)
    uint16_t mappedSwitchForTarget2;  // 목표 2에 매핑될 스위치 번호 (1 또는 2)
} stLimitSwitchConfig;

// 리미트 스위치 상태 진단 구조체
typedef struct {
    bool isFaultActive;                   // 현재 리미트 스위치 고장 발생 여부 (true: 에러 발생, 제어 반영 필요)
    LimitSwitchFaultCode_t faultCode;     // 구체적인 에러 원인 코드
} stLimitSwitchState;

extern stLimitSwitchConfig xLimitSwitchConfig;
extern stLimitSwitchState xLimitSwitch;

/**
 * @brief      리미트 스위치 관리 모듈 초기화 (임시 기본값 셋업)
 * @param      void
 * @return     void
 */
void LimitSwitch_Init(void);

/**
 * @brief      리미트 스위치 상태 주기적 점검 및 고장 판단 로직
 *             이 함수는 모터 제어 루프 내부에서 주기적으로 호출되어 즉각적인 고장을 판별합니다.
 * @param      void
 * @return     void
 */
void LimitSwitch_CheckFaults(void);

#endif // CSU_LIMITSWITCH_H
