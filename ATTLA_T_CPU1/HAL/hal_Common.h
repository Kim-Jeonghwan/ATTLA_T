/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : hal_Common.h
    Version          : 00.04
    Description      : 공통 유틸리티 하드웨어 제어 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 07. 01. (통신 확인용 LED 핀 추가 및 모터 브레이크 매크로 이동)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 07. 01. - 통신 확인용 LED 핀(GPIO 146) 추가 및 모터 브레이크 매크로 csu_Dio.h로 이동
 * 2026. 07. 01. - 구조체 변수 상세 한글 주석 추가 (코딩 규칙 적용)
 * 2026. 06. 23. - main.h -> main_cpu1.h 인클루드 명칭 리팩토링
 * 2026. 06. 23. - 코딩 규칙 및 구조 불일치 사항 리팩토링 반영
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 */


#ifndef HAL_COMMON_H
#define HAL_COMMON_H

/* ************************** [[   include  ]]  *********************************************************** */
#include "main_cpu1.h"


/* ************************** [[   define   ]]  *********************************************************** */
#define GPIO_PIN_ALIVE_LED    34U   // 메인컨트롤 ISR 동작 상태 모니터링 LED 핀
#define GPIO_PIN_COMM_LED     146U  // 통신 동작(Tx/Rx) 깜빡임 상태 모니터링 LED 핀



/* ************************** [[   enum or struct   ]]  *************************************************** */
/**
 * @brief      32비트 데이터(uint32_t, float32_t)를 8비트 바이트 단위로 분할하여 전송/수신할 때 사용하는 공용체
 */
typedef union
{
    uint32_t 		u32;            // 32비트 부호 없는 정수형 원본 데이터 접근용
	float32_t f32;                  // 32비트 단정밀도 부동소수점(실수) 원본 데이터 접근용

    struct
    {
	    uint16_t B0:8u;             // LSB (가장 낮은 자리 8비트, Byte 0)
	    uint16_t B1:8u;             // Byte 1
	    uint16_t B2:8u;             // Byte 2
	    uint16_t B3:8u;             // MSB (가장 높은 자리 8비트, Byte 3)
    } byte;
}onConv32;


/**
 * @brief      16비트 데이터(uint16_t)를 8비트 바이트 혹은 개별 비트 단위로 접근할 때 사용하는 공용체
 */
typedef union
{
    uint16_t u16;                   // 16비트 부호 없는 정수형 원본 데이터 접근용

    struct
    {
	    uint16_t B0:8u;             // LSB (가장 낮은 자리 8비트, Byte 0)
	    uint16_t B1:8u;             // MSB (가장 높은 자리 8비트, Byte 1)
    } byte;

	struct
	{
		uint16_t b00:1u;            // Bit 0 (LSB)
		uint16_t b01:1u;            // Bit 1
		uint16_t b02:1u;            // Bit 2
		uint16_t b03:1u;            // Bit 3
		uint16_t b04:1u;            // Bit 4
		uint16_t b05:1u;            // Bit 5
		uint16_t b06:1u;            // Bit 6
		uint16_t b07:1u;            // Bit 7
		uint16_t b08:1u;            // Bit 8
		uint16_t b09:1u;            // Bit 9
		uint16_t b10:1u;            // Bit 10
		uint16_t b11:1u;            // Bit 11
		uint16_t b12:1u;            // Bit 12
		uint16_t b13:1u;            // Bit 13
		uint16_t b14:1u;            // Bit 14
		uint16_t b15:1u;            // Bit 15 (MSB)
	} bit;
}onConv16;


/* ************************** [[   global   ]]  *********************************************************** */


/* ************************** [[  function  ]]  *********************************************************** */
extern void hal_Common_InitTempGpio(void);


#endif	// #ifndef HAL_COMMON_H



