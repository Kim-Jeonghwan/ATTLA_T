/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : hal_EpwmTimer.h
    Version          : 00.00
    Description      : EPWM1 기반 2ms 하드웨어 타이머 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 08. (주석 템플릿 일괄 적용)
**********************************************************************/

#ifndef HAL_EPWM_TIMER_H
#define HAL_EPWM_TIMER_H

#include "main.h"

/*
 * [EPWM1 타이머 설계]
 *   - DSP CPU 클럭: 200 MHz
 *   - 모드: UP-DOWN (대칭 PWM)
 *   - Period: 10,000 → 주기 = 10,000 × 2 / 200,000,000 = 100us (10kHz)
 *   - Zero Event (카운터 = 0) 시 ADC SOCA 발생 및 ISR 호출
 *   - ISR 내부에서 카운터를 두어 20회(2ms)마다 이더넷 송신 플래그(flag_2ms_tx) Set
 */
#define EPWM_TIMER1_BASE       EPWM1_BASE     /* EPWM1 기본 주소 */
#define EPWM_TIMER1_PERIOD     (10000U)       /* 200MHz, UP-DOWN, 100us (10kHz) */
#define EPWM_TIMER1_CLK_DIV    EPWM_CLOCK_DIVIDER_1
#define EPWM_TIMER1_HCLK_DIV   EPWM_HSCLOCK_DIVIDER_1

/* 함수 프로토타입 */
void Initial_EpwmTimer(void);

#endif /* HAL_EPWM_TIMER_H */
