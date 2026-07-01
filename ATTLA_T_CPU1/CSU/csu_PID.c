/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Pid.c
    Version          : 00.03
    Description      : 표준 PID 제어기 (안티와인드업 및 PI-IP 혼합 제어 포함)
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 07. 01. (초기화 구문 상세 한글 주석 추가)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 07. 01. - 초기화 구문 상세 한글 주석 추가 (코딩 규칙 적용)
 * 2026. 06. 23. - 코딩 규칙 및 구조 불일치 사항 리팩토링 반영
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
    if (pid != NULL)
    {
        pid->Kp = kp;                  // 비례 게인 설정
        pid->Ki = ki;                  // 적분 게인 설정
        pid->Kd = kd;                  // 미분 게인 설정
        pid->Ks = ks;                  // PI-IP 가중치 게인 설정
        pid->dt = dt;                  // 루프 샘플링 주기 설정
        pid->maxOutput = max_out;      // 출력값 상한 클램핑 설정
        pid->minOutput = min_out;      // 출력값 하한 클램핑 설정
        
        pid->integral = 0.0f;          // 누적 오차 0.0으로 초기화
        pid->prevError = 0.0f;         // 이전 오차 0.0으로 초기화
        pid->output = 0.0f;            // 초기 출력 0.0으로 초기화
    }
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
    float32_t retVal = 0.0f;

    if (pid != NULL)
    {
        float32_t error = setpoint - feedback;
        float32_t pOut = 0.0f;
        float32_t iOut = 0.0f;
        float32_t dOut = 0.0f;

        // P Control (PI-IP 혼합식 적용)
        // Ks = 1.0 (PI 제어): Kp * (setpoint - feedback) = Kp * error
        // Ks = 0.0 (IP 제어): Kp * (0 - feedback) = -Kp * feedback
        pOut = (error * pid->Kp * pid->Ks) - (feedback * pid->Kp * (1.0f - pid->Ks));

        // I Control (Anti-windup 적용)
        pid->integral += error * pid->dt;
        iOut = pid->Ki * pid->integral;

        // D Control (Divide by Zero 방어 적용)
        if (pid->dt > 1.0e-6f)
        {
            dOut = pid->Kd * ((error - pid->prevError) / pid->dt);
        }
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
        else
        {
            // 방어 코드
        }

        retVal = pid->output;
    }

    return retVal;
}
