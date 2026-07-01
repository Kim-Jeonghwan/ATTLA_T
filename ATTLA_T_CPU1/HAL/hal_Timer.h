/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : hal_Timer.h
    Version          : 00.03
    Description      : CPU 타이머 하드웨어 제어 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 07. 01. (구조체 변수 상세 주석 및 헤더 버전 동기화)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 07. 01. - 구조체 변수 상세 주석 및 헤더 버전 동기화 (코딩 규칙 적용)
 * 2026. 06. 23. - main.h -> main_cpu1.h 인클루드 명칭 리팩토링
 * 2026. 06. 15. - 컴파일러 최적화 방지용 volatile 키워드를 xTimer 구조체 선언에 추가
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */


#ifndef HAL_TIMER_H
#define HAL_TIMER_H

/* ************************** [[   include  ]]  *********************************************************** */
#include "main_cpu1.h"


/* ************************** [[   define   ]]  *********************************************************** */


/* ************************** [[   enum or struct   ]]  *************************************************** */
typedef struct
{
	uint16_t Cycle_1ms; // 1ms 주기로 증가하는 카운터 (메인 루프에서 특정 주기를 측정할 때 사용)
	uint16_t Cycle_10ms; // 10ms 주기로 증가하는 카운터
	uint16_t Cycle_100ms; // 100ms 주기로 증가하는 카운터
	uint16_t Cycle_1000ms; // 1000ms(1s) 주기로 증가하는 카운터

	uint16_t Hzcnt; // 메인 루프 실행 주기를 카운트하여 초당 시스템 동작 주파수 계산
	uint16_t Hz;    // Timer2 인터럽트에 의해 1초마다 갱신되는 메인 루프 실행 주파수 결과값
} stTimer;


/* ************************** [[   global   ]]  *********************************************************** */
extern volatile stTimer xTimer;


/* ************************** [[  function  ]]  *********************************************************** */

/**
 * @brief      CPU1 코어의 하드웨어 타이머 초기화 (0, 1, 2)
 * @param      void
 * @return     void
 */
void Initial_TIMER(void);

/**
 * @brief      CPU 타이머 0 인터럽트 서비스 루틴 (100us)
 * @param      void
 * @return     void (__interrupt)
 */
__interrupt void isr_CpuTimer0(void);

/**
 * @brief      CPU 타이머 1 인터럽트 서비스 루틴 (1ms)
 * @param      void
 * @return     void (__interrupt)
 */
__interrupt void isr_CpuTimer1(void);

/**
 * @brief      CPU 타이머 2 인터럽트 서비스 루틴 (1000ms = 1s)
 * @param      void
 * @return     void (__interrupt)
 */
__interrupt void isr_CpuTimer2(void);

#endif	// #ifndef HAL_TIMER_H
