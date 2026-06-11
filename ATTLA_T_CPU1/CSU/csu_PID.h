/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_PID.h
    Version          : 00.00
    Description      : 표준 PID 제어기 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 11. (신규 생성)
**********************************************************************/

/*
 * Modification History
 * --------------------
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

void PID_Init(PID_Controller_t* pid, float32_t kp, float32_t ki, float32_t kd, float32_t dt, float32_t max_out, float32_t min_out);
float32_t PID_Calculate(PID_Controller_t* pid, float32_t setpoint, float32_t feedback);

#endif // CSU_PID_H
