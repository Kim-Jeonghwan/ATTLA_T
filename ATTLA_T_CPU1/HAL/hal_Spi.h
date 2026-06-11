/**********************************************************************
 Nexcom Co., Ltd.
 Filename         : hal_Spi.h
 Version          : 00.02
 Description      : SPI 하드웨어 제어 헤더
 Programmer       : Kim Jeonghwan
 Last Updated     : 2026. 06. 11. (주석 표준화 및 레거시 코드 정리)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 09. - 함수명에서 hal_ 접두어 제거
 */


#ifndef HAL_SPI_H
#define HAL_SPI_H

/* ************************** [[   include  ]]  *********************************************************** */
#include "main.h"


/* ************************** [[   define   ]]  *********************************************************** */
#define FRAM_CS_GPIO        94u // SPID CS
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
uint8_t spi_read_byte(void);

/**
 * @brief      W6100 외부 SPI 데이터 1바이트 쓰기 콜백
 * @param      wb: 전송할 8비트 데이터
 * @return     void
 */
void spi_write_byte(uint8_t wb);

/**
 * @brief      W6100 칩 선택 (Chip Select Low) 외부 SPI 콜백
 * @param      void
 * @return     void
 */
void cs_sel(void);

/**
 * @brief      W6100 칩 해제 (Chip Select High) 외부 SPI 콜백
 * @param      void
 * @return     void
 */
void cs_desel(void);

#endif	// #ifndef HAL_SPI_H

