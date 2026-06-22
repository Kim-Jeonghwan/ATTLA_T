/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_MotorCtrl.h
    Version          : 00.07
    Description      : 1x PWM 모드 기반 모터 제어 모듈 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 22. (리미트 스위치 고장 정지 모드 추가)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 22. - 리미트 스위치 감지를 위한 MOTOR_MODE_FAULT_STOP 열거형 추가
 * 2026. 06. 22. - PID 계수를 xPidGain 구조체로 묶어 관리하도록 변경
 * 2026. 06. 22. - PID 계수 하드코딩 매크로 제거 및 전역 변수화 적용
 * 2026. 06. 22. - 위치 제어 분주비를 5ms 로 갱신
 * 2026. 06. 12. - 전류 제어 한계치를 모터 정격과 동일한 9.34A로 하향 조정 (Fault 임계값 10A와 차별화)
 * 2026. 06. 12. - 최대 동작 속도 제한(Soft Limit)을 정격과 동일한 3240 RPM으로 수정
 * 2026. 06. 12. - 제어 루프 분주비 매크로 추가 (이름에 숫자를 배제하고 개념적 명칭 적용)
 * 2026. 06. 12. - PID 상수, 모터 출력 한도, 변환 스케일 상수 헤더(.h)로 이동 (글로벌 룰 적용)
 * 2026. 06. 11. - 모터 제어 소프트 리미트(위치, 속도, 전류) 상수 및 매크로 추가
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 상태 변수들을 stMotorCtrlState 구조체(xMotorCtrl)로 통합
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */


#ifndef CSU_MOTORCTRL_H
#define CSU_MOTORCTRL_H

#include "main.h"

// --- 모터 제어 소프트 리미트 (Soft Limits) ---
#define LIMIT_POS_MIN       0.0f        // 기구부 최소 각도 (0도)
#define LIMIT_POS_MAX       15840.0f    // 기구부 최대 각도 (44바퀴 * 360도)

#define LIMIT_SPEED_MAX     3240.0f     // 최대 동작 속도 (RPM)
#define LIMIT_SPEED_MIN     -3240.0f    // 최소 동작 속도 (RPM)

#define LIMIT_CURRENT_MAX   9.34f       // 최대 동작 전류 (A)
#define LIMIT_CURRENT_MIN   -9.34f      // 최소 동작 전류 (A)

// 클램핑 매크로 유틸리티
#define CLAMP_F32(x, min, max)  (((x) < (min)) ? (min) : (((x) > (max)) ? (max) : (x)))

// --- 모터 제어 연산 스케일 및 듀티 한도 ---
#define MOTOR_SCALE_POS_DEG    0.001373291f
#define MOTOR_SCALE_SPEED_RPM  166.6667f
#define MOTOR_DUTY_MAX         100.0f

// --- 제어 루프 분주비 (Decimation Ratios) ---
#define DECIMATION_SPEED_CTRL  10U     // 100us -> 속도 제어기용 분주 (1ms 루프)
#define DECIMATION_POS_CTRL    5U      // 속도 루프 -> 위치 제어기용 분주 (5ms 루프)

// --- PID 제어 주기 파라미터 ---
// 전류 제어기 (100us)
#define PID_CURR_DT 0.0001f

// 속도 제어기 (1ms)
#define PID_SPD_DT  0.001f

// 위치 제어기 (5ms)
#define PID_POS_DT  0.005f

// --- PID 파라미터 전역 변수 구조체 (디버거 실시간 튜닝용) ---
typedef struct {
    float32_t Kp;
    float32_t Ki;
    float32_t Kd;
    float32_t Ks;
} stPidParam;

typedef struct {
    stPidParam pos;     // 위치 제어 (PD)
    stPidParam spd;     // 속도 제어 (PI-IP)
    stPidParam curr;    // 전류 제어 (PI)
} stPidGain;

extern stPidGain xPidGain;


// 목표 구동 모드
typedef enum {
    MOTOR_MODE_STOP = 0,
    MOTOR_MODE_SPEED_CTRL,
    MOTOR_MODE_POS_CTRL,
    MOTOR_MODE_FAULT_STOP
} MotorControlMode_t;

// 상태 변수 구조체
typedef struct {
    MotorControlMode_t mode;
    float32_t targetSpeedRpm;
    float32_t targetPosition;
    float32_t currentSpeedRpm;
    float32_t currentPosition;
} stMotorCtrlState;

extern stMotorCtrlState xMotorCtrl;

/**
 * @brief      모터 제어기 초기화
 * @param      void
 * @return     void
 */
void MotorCtrl_Init(void);

/**
 * @brief      엔코더 위치 및 속도 피드백 갱신
 * @param      void
 * @return     void
 */
void MotorCtrl_UpdateFeedback(void);

/**
 * @brief      100us 주기 모터 제어 메인 루틴
 * @param      void
 * @return     void
 */
void MotorCtrl_Run(void);

/**
 * @brief      1x PWM Duty 및 방향 설정
 * @param      outputDuty : 모터 인가 듀티 비 (-100.0 ~ 100.0 %)
 * @return     void
 */
void MotorCtrl_SetOutput(float32_t outputDuty);

#endif // CSU_MOTORCTRL_H
