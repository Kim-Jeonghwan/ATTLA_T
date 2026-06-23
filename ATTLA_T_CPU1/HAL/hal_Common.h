/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : hal_Common.h
    Version          : 00.02
    Description      : 공통 유틸리티 하드웨어 제어 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 23. (main.h -> main_cpu1.h 인클루드 명칭 리팩토링)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 23. - main.h -> main_cpu1.h 인클루드 명칭 리팩토링
 * 2026. 06. 23. - 코딩 규칙 및 구조 불일치 사항 리팩토링 반영
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 */


#ifndef HAL_COMMON_H
#define HAL_COMMON_H

/* ************************** [[   include  ]]  *********************************************************** */
#include "main_cpu1.h"


/* ************************** [[   define   ]]  *********************************************************** */
#define GPIO_PIN_MOTOR_BRAKE  35U  // 모터 브레이크 제어 핀 (Active High)
#define GPIO_PIN_ALIVE_LED    34U  // 메인컨트롤 ISR 동작 상태 모니터링 LED 핀



/* ************************** [[   enum or struct   ]]  *************************************************** */
/**
 * @brief      32비트 데이터(uint32_t, float32_t)를 8비트 바이트 단위로 분할하여 전송/수신할 때 사용하는 공용체
 */
typedef union
{
    uint32_t 		u32;
	float32_t f32;

    struct
    {
	    uint16_t B0:8u;
	    uint16_t B1:8u;
	    uint16_t B2:8u;
	    uint16_t B3:8u;
    } byte;
}onConv32;


/**
 * @brief      16비트 데이터(uint16_t)를 8비트 바이트 혹은 개별 비트 단위로 접근할 때 사용하는 공용체
 */
typedef union
{
    uint16_t u16;

    struct
    {
	    uint16_t B0:8u;
	    uint16_t B1:8u;
    } byte;

	struct
	{
		uint16_t b00:1u;
		uint16_t b01:1u;
		uint16_t b02:1u;
		uint16_t b03:1u;
		uint16_t b04:1u;
		uint16_t b05:1u;
		uint16_t b06:1u;
		uint16_t b07:1u;
		uint16_t b08:1u;
		uint16_t b09:1u;
		uint16_t b10:1u;
		uint16_t b11:1u;
		uint16_t b12:1u;
		uint16_t b13:1u;
		uint16_t b14:1u;
		uint16_t b15:1u;
	} bit;
}onConv16;


/* ************************** [[   global   ]]  *********************************************************** */


/* ************************** [[  function  ]]  *********************************************************** */



#endif	// #ifndef HAL_COMMON_H



