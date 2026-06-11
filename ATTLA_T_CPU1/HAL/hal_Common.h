/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : hal_Common.h
    Version          : 00.00
    Description      : 공통 유틸리티 하드웨어 제어 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 11. (주석 표준화 및 레거시 코드 정리)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 */


#ifndef HAL_COMMON_H
#define HAL_COMMON_H

/* ************************** [[   include  ]]  *********************************************************** */
#include "main.h"


/* ************************** [[   define   ]]  *********************************************************** */



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



