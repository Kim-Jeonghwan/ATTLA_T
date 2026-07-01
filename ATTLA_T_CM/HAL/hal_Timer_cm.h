/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : hal_Timer_cm.h
    Version          : 00.03
    Description      : CM Core SysTick 타이머 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 07. 01. (구조체 변수 상세 한글 주석 추가 및 헤더 버전 동기화)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 07. 01. - 구조체 변수 상세 한글 주석 추가 및 헤더 버전 동기화 (코딩 규칙 적용)
 * 2026. 06. 26. - hal_Timer_cm 으로 파일명 리팩토링
 * 2026. 06. 23. - 코딩 규칙 준수 정비 (매크로 상수 이동 및 작성자 기입)
 * 2026. 06. 05. - 코드 주석 포맷팅 및 한글화
 */
#ifndef HAL_TIMER_CM_H
#define HAL_TIMER_CM_H

#include "main_cm.h"

#define CM_CLK_HZ          125000000U   /* 실제 CM 클럭: AUXPLL = 125 MHz */
#define TIMER0_PERIOD_2MS  (CM_CLK_HZ / 500U)     /* 250,000: 2ms 주기 */
#define TIMER1_PERIOD_1MS  (CM_CLK_HZ / 1000U)    /* 125,000: 1ms 주기 */
#define TIMER2_PERIOD_1S   (CM_CLK_HZ / 1U)       /* 125,000,000: 1s 주기 */


typedef struct
{
    uint16_t Cycle_2ms;    // 2ms 단위 주기를 생성하여 UDP 패킷 송신 간격을 제어하기 위한 카운터 (Timer0 전용)
    uint16_t Cycle_1ms;    // 1ms 단위 주기를 생성하여 가장 빠른 백그라운드 태스크를 제어하기 위한 카운터 (Timer1 기반)
    uint16_t Cycle_10ms;   // 10ms 단위 주기를 생성하기 위해 매 1ms마다 누적되는 카운터
    uint16_t Cycle_100ms;  // 100ms 단위 주기를 생성하기 위해 매 1ms마다 누적되는 카운터 (상태 정보 송수신 점검용)
    uint16_t Cycle_1000ms; // 1000ms(1초) 단위 주기를 생성하기 위해 매 1ms마다 누적되는 카운터

    uint16_t Hzcnt;        // 메인 루프에서 연산 속도를 계측하기 위해 자유롭게 누적되는 임시 카운터
    uint16_t Hz;           // 매 1초(Timer2)마다 Hzcnt 값을 복사하여 실시간 메인 루프 반복 주파수를 나타내는 계측값
} stTimer;

extern volatile stTimer xTimer;

// 타이머 초기화
extern void Initial_TIMER(void);

// CPU 타이머 인터럽트 핸들러
extern void isr_CpuTimer0(void); // 이더넷 송신
extern void isr_CpuTimer1(void); // 주기적 작업
extern void isr_CpuTimer2(void); // Hz 측정

#endif // HAL_TIMER_CM_H
