/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : hal_Epwm.h
    Version          : 00.03
    Description      : EPWM 제어 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 23. (main.h -> main_cpu1.h 인클루드 명칭 리팩토링)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 23. - main.h -> main_cpu1.h 인클루드 명칭 리팩토링
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 모터 1x PWM Duty 제어용 Epwm_SetMotorDuty_1x() 원형 추가
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */


#ifndef HAL_EPWM_H
#define HAL_EPWM_H

/* ************************** [[   include  ]]  *********************************************************** */
#include "main_cpu1.h"

/* ************************** [[   define   ]]  *********************************************************** */

/* 100us (10kHz) 주기 설정 */
#define EPWM_TIMER1_BASE       EPWM1_BASE
#define EPWM_TIMER1_CLK_DIV    EPWM_CLOCK_DIVIDER_1
#define EPWM_TIMER1_HCLK_DIV   EPWM_HSCLOCK_DIVIDER_1
#define EPWM_TIMER1_PERIOD     (10000U)       /* 200MHz, UP-DOWN, 100us (10kHz) */
/* ************************** [[   enum or struct   ]]  *************************************************** */


/* ************************** [[   global   ]]  *********************************************************** */

/* ************************** [[  function  ]]  *********************************************************** */
/**
 * @brief      EPWM1 기반 100us 타이머 및 모터 PWM (EPWM1A / GPIO0) 초기화
 * @param      void
 * @return     void
 */
void Initial_EpwmTimer(void);

/**
 * @brief      1x PWM 모드 Duty 및 SW Force 제어
 * @param      dutyPercent : 모터 인가 듀티 퍼센트 (0.0 ~ 100.0 %)
 * @return     void
 */
void Epwm_SetMotorDuty_1x(float32_t dutyPercent);

#endif // HAL_EPWM_H
