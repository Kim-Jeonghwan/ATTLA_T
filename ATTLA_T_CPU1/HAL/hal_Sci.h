/**********************************************************************
 Nexcom Co., Ltd.
 Filename         : hal_Sci.h
 Version          : 00.03
 Description      : SCI (시리얼 통신) 하드웨어 헤더
 Programmer       : Kim Jeonghwan
 Last Updated     : 2026. 07. 01. (구조체 변수 상세 주석 및 헤더 버전 동기화)
 **********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 07. 01. - 구조체 변수 상세 주석 및 헤더 버전 동기화 (코딩 규칙 적용)
 * 2026. 06. 23. - main.h -> main_cpu1.h 인클루드 명칭 리팩토링
 * 2026. 06. 12. - 통신 포트/핀 관련 매크로 및 xQueSCI_PC 선언 헤더(.h)로 이동 (글로벌 룰 적용)
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 08. - 주석 템일 일괄 적용
 */


#ifndef HAL_SCI_H
#define HAL_SCI_H

/* ************************** [[   include  ]]  *********************************************************** */
#include "main_cpu1.h"


/* ************************** [[   define   ]]  *********************************************************** */
#define QUEUE_MAX_SCI 200u

#define SCI_PC_GPIO_PIN_SCIA_RXD	28u             // SCI RX용 GPIO 핀 번호
#define SCI_PC_GPIO_PIN_SCIA_TXD	29u             // SCI TX용 GPIO 핀 번호
#define SCI_PC_GPIO_CFG_SCIA_RXD	GPIO_28_SCIA_RX	// SCI RX용 pinConfig
#define SCI_PC_GPIO_CFG_SCIA_TXD	GPIO_29_SCIA_TX	// SCI TX용 pinConfig


/* ************************** [[   enum or struct   ]]  *************************************************** */
typedef enum
{
	eSciA_SOF = 0,
	eSciA_MSGID,
	eSciA_LEN,
	eSciA_DATA,
	eSciA_CRC,
	eSciA_EOT
}eSciA;

typedef struct
{
    eSciA           Frame;      /* 프레임 수신 위치 (SOF, ID, DATA 등 상태 제어) */
    uint16_t          MSGID;      /* 메시지 식별 ID (예: 0x10) */
    uint16_t          LEN;        /* 메시지 데이터 길이 (Payload Length) */
    uint16_t          DATA[50u];  /* 실제 수신 데이터 버퍼 */
    uint16_t          CRC;        /* 수신된 체크섬/CRC 값 */
    uint16_t 			POS;        /* 현재 데이터 수신/파싱 인덱스 위치 */
} stSciA;



typedef struct
{
    uint16_t front; // 큐의 데이터를 꺼낼 위치를 가리키는 포인터 인덱스
    uint16_t rear;  // 큐에 새로운 데이터를 저장할 위치를 가리키는 포인터 인덱스
    uint16_t Data[QUEUE_MAX_SCI]; // 실제 송신 데이터를 저장하는 원형 큐 버퍼 배열
} stQsci;






/* ************************** [[   global   ]]  *********************************************************** */
extern stQsci xQueSCI_PC;


/* ************************** [[  function  ]]  *********************************************************** */

/**
 * @brief      SCI PC 통신 드라이버 초기화
 * @param      void
 * @return     void
 */
void Initial_SCI(void);

/**
 * @brief      SCIA 수신 인터럽트 서비스 루틴 (ISR)
 * @param      void
 * @return     void (__interrupt)
 */
__interrupt void isrScia_SCI_PC(void);

/**
 * @brief      SCI 송신 큐에 데이터 삽입
 * @param      data[]: 송신할 데이터 배열
 * @param      len: 전송 길이
 * @return     void
 */
void xmtScia_SCI_PC(uint16_t data[], uint16_t len);

/**
 * @brief      비동기 SCI 데이터 송신 처리 (백그라운드 루프 폴링)
 * @param      void
 * @return     void
 */
void sendScia_SCI_PC(void);

#endif	// #ifndef HAL_SCI_H
