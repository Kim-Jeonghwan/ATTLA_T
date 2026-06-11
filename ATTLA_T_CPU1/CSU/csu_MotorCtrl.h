/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_MotorCtrl.h
    Version          : 00.00
    Description      : 1x PWM 모드 기반 모터 제어 모듈 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 11. (신규 생성)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */


#ifndef CSU_MOTORCTRL_H
#define CSU_MOTORCTRL_H

#include "main.h"

// 목표 구동 모드
typedef enum {
    MOTOR_MODE_STOP = 0,
    MOTOR_MODE_SPEED_CTRL,
    MOTOR_MODE_POS_CTRL
} MotorControlMode_t;

extern MotorControlMode_t currentMotorMode;
extern float32_t targetSpeedRpm;
extern float32_t targetPosition;
extern float32_t currentSpeedRpm;
extern float32_t currentPosition;

void MotorCtrl_Init(void);
void MotorCtrl_UpdateFeedback(void);
void MotorCtrl_Run(void);
void MotorCtrl_SetOutput(float32_t outputDuty);

#endif // CSU_MOTORCTRL_H
