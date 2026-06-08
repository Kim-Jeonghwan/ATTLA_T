/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Fram.c
    Version          : 00.00
    Description      : FRAM (CY15B256Q) 제어 모듈 로직 구현부
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 08. (최초 작성)
**********************************************************************/

/* ************************** [[   include  ]]  *********************************************************** */
#include "csu_Fram.h"

/* ************************** [[   define   ]]  *********************************************************** */

/* ************************** [[   global   ]]  *********************************************************** */

/* ************************** [[  static prototype  ]]  *************************************************** */
static void csu_Fram_WriteEnable(void);
static void csu_Fram_WriteDisable(void);
static void csu_Fram_WriteStatusRegister(uint16_t statusRegister);
static uint16_t csu_Fram_ReadStatusRegister(void);

/* ************************** [[  function  ]]  *********************************************************** */

/*
@funtion    void csu_Fram_Init(void)
@brief      FRAM 내부 설정 초기화
@param      void
@return     void
@remark 
    - 상태 레지스터의 보호 영역(BP0, BP1)을 해제합니다.
*/
void csu_Fram_Init(void)
{
    // 쓰기 활성화 (WREN)
    csu_Fram_WriteEnable();

    // 상태 레지스터 쓰기 (보호 영역 해제 BP0=0, BP1=0)
    csu_Fram_WriteStatusRegister(STATUS_REG_EX_FRAM);
}

/*
@funtion    uint16_t csu_Fram_ReadByte(uint16_t address)
@brief      FRAM에서 1바이트 읽기
@param      address : 읽어올 메모리 주소
@return     읽어온 1바이트 데이터
*/
uint16_t csu_Fram_ReadByte(uint16_t address)
{
    uint16_t rdata;

    hal_Spid_CsLow();
    hal_Spid_Transmit(FRAM_READ); // READ 명령어
    hal_Spid_Transmit((address >> 8u) & 0xFFu); // 주소 상위 8비트
    hal_Spid_Transmit(address & 0xFFu);         // 주소 하위 8비트
    rdata = hal_Spid_Transmit(0xFFu);           // 더미 데이터 전송 후 데이터 수신
    hal_Spid_CsHigh();
    
    // 동작 안정성을 위한 지연시간 (기존 참고코드 유지)
    DELAY_US(1u);

    return rdata;
}

/*
@funtion    void csu_Fram_WriteByte(uint16_t address, uint16_t writeData)
@brief      FRAM에 1바이트 쓰기
@param      address : 데이터를 기록할 주소
@param      writeData : 기록할 8비트 데이터
@return     void
*/
void csu_Fram_WriteByte(uint16_t address, uint16_t writeData)
{
    // 쓰기 작업 전 WREN 명령 전송
    hal_Spid_CsLow();
    hal_Spid_Transmit(FRAM_WREN);
    hal_Spid_CsHigh();
    DELAY_US(1u);

    hal_Spid_CsLow();
    hal_Spid_Transmit(FRAM_WRITE); // WRITE 명령어
    hal_Spid_Transmit((address >> 8u) & 0xFFu);
    hal_Spid_Transmit(address & 0xFFu);
    hal_Spid_Transmit(writeData);  // 기록할 데이터 송신
    hal_Spid_CsHigh();
    
    DELAY_US(1u);
}

/*
@funtion    void csu_Fram_PageWrite(uint16_t address, const uint16_t* data)
@brief      FRAM에 256바이트 페이지 쓰기
@param      address : 시작 주소
@param      data : 기록할 256바이트 데이터 배열
@return     void
*/
void csu_Fram_PageWrite(uint16_t address, const uint16_t* data)
{
    uint16_t i = 0u;

    hal_Spid_CsLow();
    hal_Spid_Transmit(FRAM_WREN); // Write Enable
    hal_Spid_CsHigh();
    DELAY_US(1u);

    hal_Spid_CsLow();
    hal_Spid_Transmit(FRAM_WRITE);
    hal_Spid_Transmit((address >> 8u) & 0xFFu);
    hal_Spid_Transmit(address & 0xFFu);

    for(i = 0u; i < 256u; i++)
    {
        hal_Spid_Transmit(data[i]);
    }
    hal_Spid_CsHigh();
    
    // 블로킹 딜레이 (사용자 승인 내용 반영, 기존 10ms 지연시간 유지)
    DELAY_US(10000u);
}

/*
@funtion    static void csu_Fram_WriteEnable(void)
@brief      쓰기 활성화 명령 (WREN) 전송
@param      void
@return     static void
*/
static void csu_Fram_WriteEnable(void)
{
    hal_Spid_CsLow();
    hal_Spid_Transmit(FRAM_WREN);
    hal_Spid_CsHigh();
}

/*
@funtion    static void csu_Fram_WriteDisable(void)
@brief      쓰기 비활성화 명령 (WRDI) 전송
@param      void
@return     static void
*/
static void csu_Fram_WriteDisable(void)
{
    hal_Spid_CsLow();
    hal_Spid_Transmit(FRAM_WRDI);
    hal_Spid_CsHigh();
}

/*
@funtion    static void csu_Fram_WriteStatusRegister(uint16_t statusRegister)
@brief      상태 레지스터 설정 (WRSR)
@param      statusRegister : 기록할 상태 레지스터 값
@return     static void
*/
static void csu_Fram_WriteStatusRegister(uint16_t statusRegister)
{
    hal_Spid_CsLow();
    hal_Spid_Transmit(FRAM_WRSR);
    hal_Spid_Transmit(statusRegister);
    hal_Spid_CsHigh();
}

/*
@funtion    static uint16_t csu_Fram_ReadStatusRegister(void)
@brief      상태 레지스터 읽기 (RDSR)
@param      void
@return     상태 레지스터 값
*/
static uint16_t csu_Fram_ReadStatusRegister(void)
{
    uint16_t out = 0u;

    hal_Spid_CsLow();
    hal_Spid_Transmit(FRAM_RDSR);
    out = hal_Spid_Transmit(DUMMY_EX_FRAM);
    hal_Spid_CsHigh();

    return out;
}
