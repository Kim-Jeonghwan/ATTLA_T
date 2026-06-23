/**********************************************************************
 Nexcom Co., Ltd.
 Filename         : hal_Fram.h
 Version          : 00.02
 Description      : FRAM (CY15B256Q) 제어 모듈 헤더
 Programmer       : Kim Jeonghwan
 Last Updated     : 2026. 06. 23. (main.h -> main_cpu1.h 인클루드 명칭 리팩토링)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 23. - main.h -> main_cpu1.h 인클루드 명칭 리팩토링
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */


#ifndef FRAM_H
#define FRAM_H

/* ************************** [[   include  ]]  *********************************************************** */
#include "main_cpu1.h"

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

/**
 * @brief      FRAM 내부 설정 초기화 (보호 영역 BP0, BP1 해제)
 * @param      void
 * @return     void
 */
void Fram_Init(void);

/**
 * @brief      FRAM에서 1바이트 읽기
 * @param      address : 읽어올 메모리 주소
 * @return     읽어온 1바이트 데이터
 */
uint16_t Fram_ReadByte(uint16_t address);

/**
 * @brief      FRAM에 1바이트 쓰기
 * @param      address : 데이터를 기록할 주소
 * @param      writeData : 기록할 8비트 데이터
 * @return     void
 */
void Fram_WriteByte(uint16_t address, uint16_t writeData);

/**
 * @brief      FRAM에 256바이트 페이지 쓰기
 * @param      address : 시작 주소
 * @param      data : 기록할 256바이트 데이터 배열
 * @return     void
 */
void Fram_PageWrite(uint16_t address, const uint16_t* data);

#endif // FRAM_H
