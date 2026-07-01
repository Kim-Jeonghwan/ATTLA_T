/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Pid.h
    Version          : 00.04
    Description      : 표준 PID 제어기 헤더 (PI-IP 혼합 제어 지원)
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 07. 01. (구조체 변수 상세 한글 주석 추가)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 07. 01. - 구조체 변수 상세 한글 주석 추가 (코딩 규칙 적용)
 * 2026. 06. 23. - main.h -> main_cpu1.h 인클루드 명칭 리팩토링
 * 2026. 06. 23. - 코딩 규칙 및 구조 불일치 사항 리팩토링 반영
 * 2026. 06. 22. - PI-IP 혼합 제어 지원을 위한 Ks 파라미터 추가
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */


#ifndef CSU_PID_H
#define CSU_PID_H

#include "main_cpu1.h"

typedef struct {
    // Gains
    float32_t Kp;                 // 비례 게인 (Proportional Gain)
    float32_t Ki;                 // 적분 게인 (Integral Gain)
    float32_t Kd;                 // 미분 게인 (Derivative Gain)
    float32_t Ks;                 // PI-IP 혼합 제어를 위한 가중치 게인 (0.0: IP 제어, 1.0: PI 제어)
    float32_t dt;                 // 제어 주기 (Delta Time, 단위: 초)
    
    // Limits
    float32_t maxOutput;          // 제어기 출력의 상한 제한치 (Anti-windup 클램핑용)
    float32_t minOutput;          // 제어기 출력의 하한 제한치 (Anti-windup 클램핑용)
    
    // States
    float32_t integral;           // 누적 오차 적분값 보관 변수
    float32_t prevError;          // 미분 제어를 위한 이전 주기 오차값 보관 변수
    
    // Output
    float32_t output;             // 최종 계산된 PID 제어기 출력값
} PID_Controller_t;

/**
 * @brief      PID 제어기 초기화
 * @param      pid : 제어기 구조체 포인터
 * @param      kp : 비례 게인 (Proportional Gain)
 * @param      ki : 적분 게인 (Integral Gain)
 * @param      kd : 미분 게인 (Derivative Gain)
 * @param      ks : PI-IP 혼합 게인 (0.0=IP, 1.0=PI)
 * @param      dt : 제어 주기 (초 단위, 예: 0.0001f = 100us)
 * @param      max_out : 출력 상한 제한치
 * @param      min_out : 출력 하한 제한치
 * @return     void
 */
void PID_Init(PID_Controller_t* pid, float32_t kp, float32_t ki, float32_t kd, float32_t ks, float32_t dt, float32_t max_out, float32_t min_out);

/**
 * @brief      PID 제어 연산 수행 (Anti-windup 클램핑 적용)
 * @param      pid : 제어기 구조체 포인터
 * @param      setpoint : 목표 설정값
 * @param      feedback : 현재 측정값 (피드백)
 * @return     float32_t 연산된 제어 출력 값
 */
float32_t PID_Calculate(PID_Controller_t* pid, float32_t setpoint, float32_t feedback);

#endif // CSU_PID_H
