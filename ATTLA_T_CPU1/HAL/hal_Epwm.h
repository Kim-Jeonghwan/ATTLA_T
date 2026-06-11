/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : hal_Epwm.h
    Version          : 00.02
    Description      : EPWM 제어 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 11. (모터 1x PWM Duty 제어 함수 추가)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 11. - 모터 1x PWM Duty 제어용 Epwm_SetMotorDuty_1x() 원형 추가
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */


#ifndef HAL_EPWM_H
#define HAL_EPWM_H

/* ************************** [[   include  ]]  *********************************************************** */
#include "main.h"

/* ************************** [[   define   ]]  *********************************************************** */

/* 100us (10kHz) 주기 설정 */
#define EPWM_TIMER1_BASE       EPWM1_BASE
#define EPWM_TIMER1_CLK_DIV    EPWM_CLOCK_DIVIDER_1
#define EPWM_TIMER1_HCLK_DIV   EPWM_HSCLOCK_DIVIDER_1
#define EPWM_TIMER1_PERIOD     (10000U)       /* 200MHz, UP-DOWN, 100us (10kHz) */
/* ************************** [[   enum or struct   ]]  *************************************************** */


/* ************************** [[   global   ]]  *********************************************************** */

/* ************************** [[  function  ]]  *********************************************************** */
void Initial_EpwmTimer(void);
void Epwm_SetMotorDuty_1x(float32_t dutyPercent);

#endif // HAL_EPWM_H
