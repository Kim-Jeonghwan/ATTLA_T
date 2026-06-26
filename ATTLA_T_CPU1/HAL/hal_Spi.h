/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : hal_Spi.h
    Version          : 00.08
    Description      : SPI 하드웨어 제어 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 26. (SPI-B 및 SPI-C 핀맵 재할당)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 26. - SPI-B 및 SPI-C 통신 핀맵 재할당 (GPIO 63~66, 70, 71)
 * 2026. 06. 23. - main.h -> main_cpu1.h 인클루드 명칭 리팩토링
 * 2026. 06. 15. - cs_sel, spi_read_byte 등 W6100 전용 래퍼 함수에 spia_ 접두어 추가
 * 2026. 06. 15. - LSPCLK 초과 에러 해결을 위해 W6100 SPI-A 통신 속도 10MHz로 하향 조정
 * 2026. 06. 15. - hal_DspInit.c에 있던 W6100 SPI-A 통신 핀 설정 이관
 * 2026. 06. 15. - SPI 설정 매직넘버 상수화 및 핀 네이밍 규칙 통일
 * 2026. 06. 12. - 엔코더 SPI 통신용 핀 매크로를 헤더(.h)로 이동 (글로벌 룰 적용)
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 09. - 함수명에서 hal_ 접두어 제거
 */


#ifndef HAL_SPI_H
#define HAL_SPI_H

/* ************************** [[   include  ]]  *********************************************************** */
#include "main_cpu1.h"


/* ************************** [[   define   ]]  *********************************************************** */
// [ SPI-A : W6100 ]
#define SPIA_W6100_BAUDRATE         10000000u   // 10MHz
#define SPIA_W6100_DATA_WIDTH       8u
#define SPIA_W6100_SIMO_PIN         16u
#define SPIA_W6100_SOMI_PIN         17u
#define SPIA_W6100_CLK_PIN          18u
#define SPIA_W6100_CS_PIN           19u

// [ SPI-B : Motor Driver ]
#define SPIB_MOTOR_BAUDRATE         1000000u    // 1MHz
#define SPIB_MOTOR_DATA_WIDTH       16u
#define SPIB_MOTOR_CLK_PIN          65u
#define SPIB_MOTOR_STE_PIN          66u
#define SPIB_MOTOR_SIMO_PIN         63u
#define SPIB_MOTOR_SOMI_PIN         64u

// [ SPI-C : SSI Encoder ]
#define SPIC_SSI_BAUDRATE           2500000u    // 2.5MHz
#define SPIC_SSI_DATA_WIDTH         16u
#define SPIC_SSI_SOMI_PIN           70u
#define SPIC_SSI_CLK_PIN            71u

// [ SPI-D : FRAM ]
#define SPID_FRAM_BAUDRATE          1000000u    // 1MHz
#define SPID_FRAM_DATA_WIDTH        8u
#define SPID_FRAM_SIMO_PIN          91u
#define SPID_FRAM_SOMI_PIN          92u
#define SPID_FRAM_CLK_PIN           93u
#define SPID_FRAM_CS_PIN            94u
/* ************************** [[   enum or struct   ]]  *************************************************** */




/* ************************** [[   global   ]]  *********************************************************** */



/* ************************** [[  function  ]]  *********************************************************** */

/**
 * @brief      SPI 드라이버 초기화 (SPI-A, SPI-C, SPI-D)
 * @param      void
 * @return     void
 */
void Initial_SPI(void);

/**
 * @brief      FRAM CS 핀을 Low 상태로 만듭니다. (Select)
 * @param      void
 * @return     void
 */
void Spid_CsLow(void);

/**
 * @brief      FRAM CS 핀을 High 상태로 만듭니다. (Deselect)
 * @param      void
 * @return     void
 */
void Spid_CsHigh(void);

/**
 * @brief      SPI-D 모듈을 사용하여 1바이트 데이터를 송수신합니다.
 * @param      data : 전송할 8비트 데이터
 * @return     수신된 8비트 데이터
 */
uint16_t Spid_Transmit(uint16_t data);

/**
 * @brief      W6100 외부 SPI 데이터 1바이트 읽기 콜백
 * @param      void
 * @return     uint8_t (수신된 데이터)
 */
uint8_t spia_read_byte(void);

/**
 * @brief      W6100 외부 SPI 데이터 1바이트 쓰기 콜백
 * @param      wb: 전송할 8비트 데이터
 * @return     void
 */
void spia_write_byte(uint8_t wb);

/**
 * @brief      W6100 칩 선택 (Chip Select Low) 외부 SPI 콜백
 * @param      void
 * @return     void
 */
void spia_cs_sel(void);

/**
 * @brief      W6100 칩 해제 (Chip Select High) 외부 SPI 콜백
 * @param      void
 * @return     void
 */
void spia_cs_desel(void);

#endif	// #ifndef HAL_SPI_H

