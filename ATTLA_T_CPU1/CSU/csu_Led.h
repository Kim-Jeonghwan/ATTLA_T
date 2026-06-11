/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Led.h
    Version          : 00.01
    Description      : 시스템 상태 표시 LED 제어 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 11. (주석 표준화 및 레거시 코드 정리)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */


#ifndef CSU_LED_H
#define CSU_LED_H

/* ************************** [[   include  ]]  *********************************************************** */
#include "main.h"



/* ************************** [[   define   ]]  *********************************************************** */
#define LED_OFF		true
#define LED_ON		false

#define LED_NONE	false
#define LED_TOGGLE	true

/* nG 상태 표시용 LED(GPIO 30)*/
#define GPIO_LED_nG     30u


/* ************************** [[   enum or struct   ]]  **************************************************** */

/**
 * @brief LED 인덱스 정의 (GPIO 번호 매핑)
 */
typedef enum
{
	eLED_nG			            = 30u,

} eLed;

/**
 * @brief 개별 LED 제어 속성 구조체
 */
typedef struct
{
    uint16_t Index:8u;    // GPIO Index (eLed 타입 저장)
    uint16_t Time:8u;     // Toggle 주기 설정
    uint16_t Temp:8u;     // 카운트 다운용 임시 변수
    bool     State:1u;    // 현재 점등 상태 (false: On, true: Off)
    bool     Toggle:1u;   // 토글 모드 활성 (false: None, true: Toggle)
    uint16_t Reserved:14u;
} stLed;

/**
 * @brief 시스템 전체 LED 상태 관리 구조체
 */
typedef struct
{
	stLed	lednG;

} stLedStatus;



/* ************************** [[   global   ]]  *********************************************************** */
extern stLedStatus xLed;



/* ************************** [[  function  ]] *********************************************************** */


/**
 * @brief      LED 변수 초기화 및 기본 동작 설정
 * @param      void
 * @return     void
 */
void Initial_Led(void);

/**
 * @brief      LED 동작 상태 업데이트 (Main Loop 호출)
 * @param      void
 * @return     void
 */
void updateLedStatus(void);

/**
 * @brief      LED의 On/Off 상태를 직접 설정 (토글 중단)
 * @param      pLed : 대상 LED 구조체 포인터
 * @param      State : LED_ON(1) 또는 LED_OFF(0)
 * @return     void
 */
void setLedStatus(stLed *pLed, bool State);

/**
 * @brief      LED 토글 모드 활성화 및 주기 설정
 * @param      pLed : 대상 LED 구조체 포인터
 * @param      State : LED_TOGGLE(1) 또는 LED_NONE(0)
 * @param      Time : 토글 유지 카운트 (100ms 단위)
 * @return     void
 */
void setLedModeToggle(stLed *pLed, bool State, uint16_t Time);

#endif	// #ifndef CSU_LED_H

