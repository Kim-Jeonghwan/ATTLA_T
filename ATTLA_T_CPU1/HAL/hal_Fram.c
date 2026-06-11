/**********************************************************************
 Nexcom Co., Ltd.
 Filename         : hal_Fram.c
 Version          : 00.01
 Description      : FRAM (CY15B256Q) 제어 모듈 로직 구현부
 Programmer       : Kim Jeonghwan
 Last Updated     : 2026. 06. 11. (주석 표준화 및 레거시 코드 정리)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */


/* ************************** [[   include  ]]  *********************************************************** */
#include "hal_Fram.h"

/* ************************** [[   define   ]]  *********************************************************** */

/* ************************** [[   global   ]]  *********************************************************** */

/* ************************** [[  static prototype  ]]  *************************************************** */
static void Fram_WriteEnable(void);
static void Fram_WriteStatusRegister(uint16_t statusRegister);

/* ************************** [[  function  ]]  *********************************************************** */

/*
@function    void Fram_Init(void)
@brief      FRAM 내부 설정 초기화
@param      void
@return     void
@remark 
    - 상태 레지스터의 보호 영역(BP0, BP1)을 해제합니다.
*/
void Fram_Init(void)
{
    // 쓰기 활성화 (WREN)
    Fram_WriteEnable();

    // 상태 레지스터 쓰기 (보호 영역 해제 BP0=0, BP1=0)
    Fram_WriteStatusRegister(STATUS_REG_EX_FRAM);
}

/*
@function    uint16_t Fram_ReadByte(uint16_t address)
@brief      FRAM에서 1바이트 읽기
@param      address : 읽어올 메모리 주소
@return     읽어온 1바이트 데이터
*/
uint16_t Fram_ReadByte(uint16_t address)
{
    uint16_t rdata;

    Spid_CsLow();
    Spid_Transmit(FRAM_READ); // READ 명령어
    Spid_Transmit((address >> 8u) & 0xFFu); // 주소 상위 8비트
    Spid_Transmit(address & 0xFFu);         // 주소 하위 8비트
    rdata = Spid_Transmit(0xFFu);           // 더미 데이터 전송 후 데이터 수신
    Spid_CsHigh();
    
    // 동작 안정성을 위한 지연시간 (기존 참고코드 유지)
    DELAY_US(1u);

    return rdata;
}

/*
@function    void Fram_WriteByte(uint16_t address, uint16_t writeData)
@brief      FRAM에 1바이트 쓰기
@param      address : 데이터를 기록할 주소
@param      writeData : 기록할 8비트 데이터
@return     void
*/
void Fram_WriteByte(uint16_t address, uint16_t writeData)
{
    // 쓰기 작업 전 WREN 명령 전송
    Spid_CsLow();
    Spid_Transmit(FRAM_WREN);
    Spid_CsHigh();
    DELAY_US(1u);

    Spid_CsLow();
    Spid_Transmit(FRAM_WRITE); // WRITE 명령어
    Spid_Transmit((address >> 8u) & 0xFFu);
    Spid_Transmit(address & 0xFFu);
    Spid_Transmit(writeData);  // 기록할 데이터 송신
    Spid_CsHigh();
    
    DELAY_US(1u);
}

/*
@function    void Fram_PageWrite(uint16_t address, const uint16_t* data)
@brief      FRAM에 256바이트 페이지 쓰기
@param      address : 시작 주소
@param      data : 기록할 256바이트 데이터 배열
@return     void
*/
void Fram_PageWrite(uint16_t address, const uint16_t* data)
{
    uint16_t i = 0u;

    Spid_CsLow();
    Spid_Transmit(FRAM_WREN); // Write Enable
    Spid_CsHigh();
    DELAY_US(1u);

    Spid_CsLow();
    Spid_Transmit(FRAM_WRITE);
    Spid_Transmit((address >> 8u) & 0xFFu);
    Spid_Transmit(address & 0xFFu);

    for(i = 0u; i < 256u; i++)
    {
        Spid_Transmit(data[i]);
    }
    Spid_CsHigh();
}

/*
@function    static void Fram_WriteEnable(void)
@brief      쓰기 활성화 명령 (WREN) 전송
@param      void
@return     static void
*/
static void Fram_WriteEnable(void)
{
    Spid_CsLow();
    Spid_Transmit(FRAM_WREN);
    Spid_CsHigh();
}

/*
@function    static void Fram_WriteStatusRegister(uint16_t statusRegister)
@brief      상태 레지스터 설정 (WRSR)
@param      statusRegister : 기록할 상태 레지스터 값
@return     static void
*/
static void Fram_WriteStatusRegister(uint16_t statusRegister)
{
    Spid_CsLow();
    Spid_Transmit(FRAM_WRSR);
    Spid_Transmit(statusRegister);
    Spid_CsHigh();
}
