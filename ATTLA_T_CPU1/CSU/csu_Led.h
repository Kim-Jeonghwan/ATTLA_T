/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Led.h
    Version          : 00.00
    Description      : 시스템 상태 표시 LED 제어 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 08. (주석 템플릿 일괄 적용)
**********************************************************************/

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
// /* ERROR 상태 표시용 LED(GPIO 146)*/
// #define GPIO_LED_ERROR       146u


/* ************************** [[   enum or struct   ]]  **************************************************** */

/**
 * @brief LED 인덱스 정의 (GPIO 번호 매핑)
 */
typedef enum
{
	eLED_nG			            = 30u,
//	eLED_ERROR				    = 146u,

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
//	stLed	ledError;

} stLedStatus;



/* ************************** [[   global   ]]  *********************************************************** */
extern stLedStatus xLed;



/* ************************** [[  function  ]]  *********************************************************** */

/**
 * @brief LED 제어를 위한 GPIO 방향 및 Mux 설정
 */
void initGpioDoutLed(void);

/**
 * @brief LED 변수 초기화 및 기본 동작 설정
 */
void Initial_Led(void);

/**
 * @brief LED 동작 상태 업데이트 (Main Loop 호출)
 */
void updateLedStatus(void);

/**
 * @brief LED의 On/Off 상태를 직접 설정 (토글 중단)

 */
void setLedStatus(stLed *pLed, bool State);

/**
 * @brief LED 토글 모드 활성화 및 주기 설정
 */
void setLedModeToggle(stLed *pLed, bool State, uint16_t Time);

/**
 * @brief IPC 커맨드에 따른 GPIO LED 직접 제어
 */
void updateGpioLed(void);



#endif	// #ifndef CSU_LED_H

