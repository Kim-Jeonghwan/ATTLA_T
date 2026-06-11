/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : hal_Epwm.h
    Version          : 00.01
    Description      : EPWM 제어 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 08. (주석 템플릿 일괄 적용)
**********************************************************************/

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
void Initial_Epwm7a(void);
void Initial_EpwmTimer(void);
#endif // HAL_EPWM_H
