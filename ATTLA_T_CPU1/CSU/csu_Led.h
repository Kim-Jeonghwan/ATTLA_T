/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Led.h
    Version          : 00.06
    Description      : 시스템 상태 표시 LED 제어 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 30. (eLED_nG 할당 핀 번호 변경: 31 -> 145)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 30. - eLED_nG 매핑 핀 번호를 145로 변경 (기존 31)
 * 2026. 06. 23. - main.h -> main_cpu1.h 인클루드 명칭 리팩토링
 * 2026. 06. 15. - stLed 구조체의 비트필드(총 24비트로 C2000 16비트 경계 초과 버그 유발) 제거 및 일반 자료형 변경
 * 2026. 06. 15. - PinMux 설정(GPIO_30_GPIO30)도 GPIO_LED_nG_PIN_CONFIG 매크로로 래핑하여 하드웨어 의존성 분리
 * 2026. 06. 15. - enum 정의 시 하드코딩된 30u 대신 매크로 상수(GPIO_LED_nG)를 활용하도록 수정
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */


#ifndef CSU_LED_H
#define CSU_LED_H

/* ************************** [[   include  ]]  *********************************************************** */
#include "main_cpu1.h"



/* ************************** [[   define   ]]  *********************************************************** */
#define LED_OFF     1u
#define LED_ON      0u

#define LED_NONE    0u
#define LED_TOGGLE  1u


/* ************************** [[   enum or struct   ]]  **************************************************** */

/**
 * @brief LED 인덱스 정의 (GPIO 번호 매핑)
 */
typedef enum
{
	eLED_nG			            = 145u, // 추후 30u 로 변경 예정

} eLed;

/**
 * @brief 개별 LED 제어 속성 구조체
 */
typedef struct
{
    uint16_t Index;       // GPIO Index (eLed 타입 저장)
    uint16_t Time;        // Toggle 주기 설정
    uint16_t Temp;        // 카운트 다운용 임시 변수
    uint16_t State;       // 현재 점등 상태 (LED_ON: 0, LED_OFF: 1)
    uint16_t Toggle;      // 토글 모드 활성 (LED_NONE: 0, LED_TOGGLE: 1)
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
void setLedStatus(stLed *pLed, uint16_t State);

/**
 * @brief      LED 토글 모드 활성화 및 주기 설정
 * @param      pLed : 대상 LED 구조체 포인터
 * @param      State : LED_TOGGLE(1) 또는 LED_NONE(0)
 * @param      Time : 토글 유지 카운트 (100ms 단위)
 * @return     void
 */
void setLedModeToggle(stLed *pLed, uint16_t State, uint16_t Time);

#endif	// #ifndef CSU_LED_H

