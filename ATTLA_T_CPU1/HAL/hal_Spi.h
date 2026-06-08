/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : hal_Spi.h
    Version          : 00.00
    Description      : SPI 하드웨어 제어 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 08. (주석 템플릿 일괄 적용)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 
 * 
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
void Initial_SPI(void);

// FRAM (SPI-D) 용 함수 선언
void hal_Spid_CsLow(void);
void hal_Spid_CsHigh(void);
uint16_t hal_Spid_Transmit(uint16_t data);

// WIZnet W6100용 SPI 콜백 래퍼 함수
uint8_t spi_read_byte(void);
void spi_write_byte(uint8_t wb);
void cs_sel(void);
void cs_desel(void);

#endif	// #ifndef HAL_SPI_H

