/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_PID.h
    Version          : 00.00
    Description      : 표준 PID 제어기 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 11. (주석 표준화 및 레거시 코드 정리)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */


#ifndef CSU_PID_H
#define CSU_PID_H

#include "main.h"

typedef struct {
    // Gains
    float32_t Kp;
    float32_t Ki;
    float32_t Kd;
    float32_t dt;
    
    // Limits
    float32_t maxOutput;
    float32_t minOutput;
    
    // States
    float32_t integral;
    float32_t prevError;
    
    // Output
    float32_t output;
} PID_Controller_t;

/**
 * @brief      PID 제어기 초기화
 * @param      pid : 제어기 구조체 포인터
 * @param      kp : 비례 게인 (Proportional Gain)
 * @param      ki : 적분 게인 (Integral Gain)
 * @param      kd : 미분 게인 (Derivative Gain)
 * @param      dt : 제어 주기 (초 단위, 예: 0.0001f = 100us)
 * @param      max_out : 출력 상한 제한치
 * @param      min_out : 출력 하한 제한치
 * @return     void
 */
void PID_Init(PID_Controller_t* pid, float32_t kp, float32_t ki, float32_t kd, float32_t dt, float32_t max_out, float32_t min_out);

/**
 * @brief      PID 제어 연산 수행 (Anti-windup 클램핑 적용)
 * @param      pid : 제어기 구조체 포인터
 * @param      setpoint : 목표 설정값
 * @param      feedback : 현재 측정값 (피드백)
 * @return     float32_t 연산된 제어 출력 값
 */
float32_t PID_Calculate(PID_Controller_t* pid, float32_t setpoint, float32_t feedback);

#endif // CSU_PID_H
