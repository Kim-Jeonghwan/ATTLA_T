/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : hal_Spi.h
    Version          : 00.01
    Description      : SPI 하드웨어 제어 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 09. (함수명에서 hal_ 접두어 제거)
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
void Spid_CsLow(void);
void Spid_CsHigh(void);
uint16_t Spid_Transmit(uint16_t data);

// WIZnet W6100용 SPI 콜백 래퍼 함수
uint8_t spi_read_byte(void);
void spi_write_byte(uint8_t wb);
void cs_sel(void);
void cs_desel(void);

#endif	// #ifndef HAL_SPI_H

