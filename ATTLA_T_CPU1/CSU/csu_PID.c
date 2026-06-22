/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Pid.c
    Version          : 00.01
    Description      : 표준 PID 제어기 (안티와인드업 및 PI-IP 혼합 제어 포함)
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 22. (PI-IP 혼합 제어 지원 로직 추가)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 22. - PI-IP 혼합 제어 지원 로직 추가 (Ks 파라미터 적용)
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */


#include "csu_Pid.h"

/*
@function    void PID_Init(PID_Controller_t* pid, float32_t kp, float32_t ki, float32_t kd, float32_t ks, float32_t dt, float32_t max_out, float32_t min_out)
@brief      PID 제어기 초기화
@param      pid: 제어기 구조체 포인터
@param      kp, ki, kd: PID 게인
@param      ks: PI-IP 혼합 게인 (0.0=IP, 1.0=PI)
@param      dt: 제어 주기 (초 단위, 예: 0.0001f = 100us)
@param      max_out, min_out: 출력 제한
@return     void
*/
void PID_Init(PID_Controller_t* pid, float32_t kp, float32_t ki, float32_t kd, float32_t ks, float32_t dt, float32_t max_out, float32_t min_out)
{
    pid->Kp = kp;
    pid->Ki = ki;
    pid->Kd = kd;
    pid->Ks = ks;
    pid->dt = dt;
    pid->maxOutput = max_out;
    pid->minOutput = min_out;
    
    pid->integral = 0.0f;
    pid->prevError = 0.0f;
    pid->output = 0.0f;
}

/*
@function    float32_t PID_Calculate(PID_Controller_t* pid, float32_t setpoint, float32_t feedback)
@brief      PID 제어 연산 수행
@param      pid: 제어기 구조체 포인터
@param      setpoint: 목표값
@param      feedback: 현재 측정값 (피드백)
@return     float32_t 제어 출력값
*/
float32_t PID_Calculate(PID_Controller_t* pid, float32_t setpoint, float32_t feedback)
{
    float32_t error = setpoint - feedback;
    float32_t pOut, iOut, dOut;

    // P Control (PI-IP 혼합식 적용)
    // Ks = 1.0 (PI 제어): Kp * (setpoint - feedback) = Kp * error
    // Ks = 0.0 (IP 제어): Kp * (0 - feedback) = -Kp * feedback
    pOut = (error * pid->Kp * pid->Ks) - (feedback * pid->Kp * (1.0f - pid->Ks));

    // I Control (Anti-windup 적용)
    pid->integral += error * pid->dt;
    iOut = pid->Ki * pid->integral;

    // D Control
    dOut = pid->Kd * ((error - pid->prevError) / pid->dt);
    pid->prevError = error;

    // Total Output
    pid->output = pOut + iOut + dOut;

    // 출력 제한 (Saturation) & Anti-windup
    if (pid->output > pid->maxOutput)
    {
        pid->output = pid->maxOutput;
        // Anti-windup (Clamping)
        pid->integral -= error * pid->dt;
    }
    else if (pid->output < pid->minOutput)
    {
        pid->output = pid->minOutput;
        // Anti-windup (Clamping)
        pid->integral -= error * pid->dt;
    }

    return pid->output;
}
