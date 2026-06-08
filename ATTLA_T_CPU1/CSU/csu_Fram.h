/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Fram.h
    Version          : 00.00
    Description      : FRAM (CY15B256Q) 제어 모듈 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 08. (최초 작성)
**********************************************************************/

#ifndef CSU_FRAM_H
#define CSU_FRAM_H

/* ************************** [[   include  ]]  *********************************************************** */
#include "main.h"

/* ************************** [[   define   ]]  *********************************************************** */
// CY15B256Q 명령어 정의
#define FRAM_WREN           0x06u   // Set write enable latch
#define FRAM_WRDI           0x04u   // Write disable
#define FRAM_RDSR           0x05u   // Read status register
#define FRAM_WRSR           0x01u   // Write status register
#define FRAM_READ           0x03u   // Read memory data
#define FRAM_WRITE          0x02u   // Write memory data
#define STATUS_REG_EX_FRAM  0x00u
#define DUMMY_EX_FRAM       0xFFu

/* ************************** [[   enum or struct   ]]  *************************************************** */

/* ************************** [[   global   ]]  *********************************************************** */

/* ************************** [[  function  ]]  *********************************************************** */

void csu_Fram_Init(void);
uint16_t csu_Fram_ReadByte(uint16_t address);
void csu_Fram_WriteByte(uint16_t address, uint16_t writeData);
void csu_Fram_PageWrite(uint16_t address, const uint16_t* data);

#endif // CSU_FRAM_H
