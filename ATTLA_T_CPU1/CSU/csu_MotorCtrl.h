/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_MotorCtrl.h
    Version          : 00.01
    Description      : 1x PWM 모드 기반 모터 제어 모듈 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 11. (주석 표준화 및 레거시 코드 정리)
**********************************************************************/

/*
 * Modification History
 * --------------------
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

#define LIMIT_SPEED_MAX     3500.0f     // 최대 동작 속도 (RPM)
#define LIMIT_SPEED_MIN     -3500.0f    // 최소 동작 속도 (RPM)

#define LIMIT_CURRENT_MAX   10.0f       // 최대 동작 전류 (A)
#define LIMIT_CURRENT_MIN   -10.0f      // 최소 동작 전류 (A)

// 클램핑 매크로 유틸리티
#define CLAMP_F32(x, min, max)  (((x) < (min)) ? (min) : (((x) > (max)) ? (max) : (x)))


// 목표 구동 모드
typedef enum {
    MOTOR_MODE_STOP = 0,
    MOTOR_MODE_SPEED_CTRL,
    MOTOR_MODE_POS_CTRL
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
