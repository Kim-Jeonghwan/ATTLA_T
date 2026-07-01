/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_LimitSwitch.h
    Version          : 00.02
    Description      : 리미트 스위치 상태 감지 및 고장 진단 모듈 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 30. (리미트 스위치 데드존 및 오프셋 거리 제어 로직 추가)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 30. - 리미트 스위치 데드존 및 오프셋 거리 제어 로직 추가
 * 2026. 06. 23. - main.h -> main_cpu1.h 인클루드 명칭 리팩토링
 * 2026. 06. 22. - 파일 생성 및 기본 구조 작성
 */

#ifndef CSU_LIMITSWITCH_H
#define CSU_LIMITSWITCH_H

#include "main_cpu1.h"

// 리미트 스위치 디버깅 튜닝 파라미터 구조체
typedef struct {
    float32_t offsetCount;           // 리미트 감지 후 진입 허용 제한 거리
    float32_t deadzoneCount;         // 오프셋 재설정을 방지하기 위한 데드존 최소 이동 거리
    uint16_t sensorErrorTimeMs;      // 센서 이상 상태 판단 유지 시간 (ms)
    uint16_t sensorErrorTick100us;   // 센서 이상 상태 판단 100us 기준 틱 수 (500틱)
} stLimitSwitchLimit;

extern stLimitSwitchLimit xLimitSwitchLimit;

// 리미트 스위치 고장 코드 열거형
typedef enum {
    LS_FAULT_NONE = 0,
    LS_FAULT_SW1_BROKEN,        // 스위치1 단선 또는 하드웨어 고장 (NO == NC)
    LS_FAULT_SW2_BROKEN,        // 스위치2 단선 또는 하드웨어 고장 (NO == NC)
    LS_FAULT_SIMULTANEOUS       // 양방향 스위치 동시 감지 고장
} LimitSwitchFaultCode_t;

// 리미트 스위치 설정 데이터 구조체
typedef struct {
    uint16_t mappedSwitchForPosDir;  // 양의 방향(Positive) 매핑 스위치 (기본 1)
    uint16_t mappedSwitchForNegDir;  // 음의 방향(Negative) 매핑 스위치 (기본 2)
} stLimitSwitchConfig;

// 리미트 스위치 상태 진단 구조체
typedef struct {
    bool isFaultActive;                   // 현재 리미트 스위치 고장 발생 여부
    LimitSwitchFaultCode_t faultCode;     // 구체적인 에러 원인 코드
    
    // 오프셋 및 데드존 차단 로직용 상태
    uint16_t activeDirection;             // 감지된 방향 (0: 없음, 1: Positive, 2: Negative)
    bool isLimitReached;                  // 제한 거리(LIMIT_OFFSET_COUNT) 초과 도달 여부
    float32_t limitBasePos;               // 스위치가 최초 감지된 시점의 기준 위치
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
