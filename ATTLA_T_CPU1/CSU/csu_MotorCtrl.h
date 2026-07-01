/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_MotorCtrl.h
    Version          : 00.11
    Description      : 1x PWM 모드 기반 모터 제어 모듈 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 07. 01. (구조체 변수 상세 한글 주석 추가)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 07. 01. - 구조체 변수 상세 한글 주석 추가 (코딩 규칙 적용)
 * 2026. 06. 30. - 엔코더 영점 설정 안전 인터락 타이머 변수 및 함수 추가
 * 2026. 06. 30. - 전자식 브레이크 시퀀스 상태 및 매크로 상수 추가
 * 2026. 06. 30. - 리미트 스위치 감지 시 전류 제한 비율(LIMIT_CURRENT_RATIO) 매크로 추가
 * 2026. 06. 23. - main.h -> main_cpu1.h 인클루드 명칭 리팩토링
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

#include "main_cpu1.h"

// 모터 제어 디버깅 튜닝 파라미터 구조체
typedef struct {
    float32_t posMin;                 // 기구부 최소 허용 동작 각도 (단위: Degree, 기본 0도)
    float32_t posMax;                 // 기구부 최대 허용 동작 각도 (단위: Degree, 예: 44회전 * 360도 = 15840도)
    float32_t speedMax;               // 정방향(Positive) 최대 허용 동작 속도 (단위: RPM)
    float32_t speedMin;               // 역방향(Negative) 최대 허용 동작 속도 (단위: RPM, 음수값)
    float32_t currentMax;             // 정방향(Positive) 최대 허용 동작 전류 (단위: A)
    float32_t currentMin;             // 역방향(Negative) 최대 허용 동작 전류 (단위: A, 음수값)
    float32_t currentRatio;           // 리미트 영역 진입 시 충돌 방지를 위해 제한할 구속 전류 비율 (예: Max 대비 25%)
    uint16_t brakeReleaseDelayMs;     // 브레이크 물리적 해제(Brake OFF) 완료까지 대기할 지연 시간 (단위: ms)
    uint16_t brakeEngageDelayMs;      // 모터 정지 상태 판단 후 물리적 브레이크 체결(Brake ON)까지 대기할 지연 시간 (단위: ms)
    uint16_t brakeReleaseTick100us;   // 브레이크 해제 지연 시간을 100us 단위 루프 틱 수로 환산한 값 (예: 150ms = 1500틱)
    uint16_t brakeEngageTick100us;    // 브레이크 체결 지연 시간을 100us 단위 루프 틱 수로 환산한 값 (예: 100ms = 1000틱)
    uint16_t safeZeroSetTick100us;    // 엔코더 영점 설정 전, 완전 정지 상태가 500ms(5000틱) 유지되었는지 검증하기 위한 안전 인터락 타이머 한도
} stMotorCtrlLimit;

extern stMotorCtrlLimit xMotorCtrlLimit;

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
    float32_t Kp;       // 비례(Proportional) 제어 이득(Gain) 계수
    float32_t Ki;       // 적분(Integral) 제어 이득(Gain) 계수
    float32_t Kd;       // 미분(Derivative) 제어 이득(Gain) 계수
    float32_t Ks;       // PI-IP 제어 등 특수 혼합 제어용 비례-적분 분배 계수(Scale)
} stPidParam;

typedef struct {
    stPidParam pos;     // 위치 제어 루프용 PID 파라미터 (주로 PD 제어 적용)
    stPidParam spd;     // 속도 제어 루프용 PID 파라미터 (주로 PI-IP 혼합 제어 적용)
    stPidParam curr;    // 전류 제어 루프용 PID 파라미터 (주로 표준 PI 제어 적용)
} stPidGain;

extern stPidGain xPidGain;


// 목표 구동 모드
typedef enum {
    MOTOR_MODE_STOP = 0,
    MOTOR_MODE_SPEED_CTRL,
    MOTOR_MODE_POS_CTRL,
    MOTOR_MODE_FAULT_STOP
} MotorControlMode_t;

// 브레이크 시퀀스 상태 열거형
typedef enum {
    BRAKE_STATE_LOCKED = 0,    // 완전히 잠김 상태 (Servo Off, 브레이크 ON)
    BRAKE_STATE_RELEASING,     // 해제 중 (토크 인가, 딜레이 대기 중)
    BRAKE_STATE_FREE,          // 해제 완료 (정상 제어 가능)
    BRAKE_STATE_ENGAGING       // 체결 중 (토크 유지, 딜레이 대기 중)
} BrakeState_t;

// 상태 변수 구조체
typedef struct {
    MotorControlMode_t mode;          // 현재 설정된 모터 제어 운용 모드 (정지, 속도, 위치, 고장정지 등)
    float32_t targetSpeedRpm;         // 목표(Target) 구동 속도 지령 (단위: RPM)
    float32_t targetPosition;         // 목표(Target) 구동 위치 지령 (단위: Degree)
    float32_t currentSpeedRpm;        // 엔코더 피드백으로부터 계산된 현재 실제 모터 속도 (단위: RPM)
    float32_t currentPosition;        // 엔코더 피드백으로부터 환산된 현재 실제 모터 기계각 위치 (단위: Degree)
    
    // 전자식 브레이크 타이머용 변수
    BrakeState_t brakeState;          // 전자식 브레이크 현재 시퀀스 상태 (LOCKED, RELEASING, FREE, ENGAGING)
    uint16_t brakeTimerTick;          // 브레이크 상태 천이(해제/체결) 시 딜레이 대기를 위한 100us 누적 틱 카운터
    
    // 영점 설정 안전 타이머 (500ms 유지)
    uint16_t safeToZeroTimerTick;     // 모터 완전 정지 상태를 누적 카운트하여 영점 설정 안전 여부를 확인하는 타이머 (100us 틱)
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

/**
 * @brief      엔코더 영점(Zero) 설정 안전 조건 충족 여부 확인
 * @param      void
 * @return     bool : 안전 조건 충족 시 true, 아니면 false
 */
bool MotorCtrl_IsSafeToZeroSet(void);

#endif // CSU_MOTORCTRL_H
