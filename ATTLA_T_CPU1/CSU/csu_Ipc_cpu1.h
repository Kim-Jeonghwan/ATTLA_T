/**********************************************************************
   Nexcom Co., Ltd.
   Filename         : csu_Ipc_cpu1.h
   Version          : 00.01
   Description      : CPU1 IPC 및 공유 메모리 통신 프로토콜 정의
   Programmer       : Kim Jeonghwan
   Last Updated     : 2026. 06. 23. (main.h -> main_cpu1.h 인클루드 명칭 리팩토링)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 23. - main.h -> main_cpu1.h 인클루드 명칭 리팩토링
 * 2026. 06. 23. - CM 코어 IPC 통신용 구조체 및 버퍼 정의
 */

#ifndef CSU_IPC_CPU1_H
#define CSU_IPC_CPU1_H

#include "main_cpu1.h"

/* IPC 통신용 공용 데이터 구조체 (32비트 정렬) */
typedef struct {
    uint32_t seqCount;      /* 동기화용 Seqlock 카운터 (짝수=완료, 홀수=쓰기중) */
    uint32_t Command;       /* 명령어 */
    uint32_t Status;        /* 상태 플래그 */
    uint32_t Address;       /* 메모리 주소 */
    union {
        uint32_t PayloadRaw[16];   /* 실제 데이터 배열 */
        struct {
            uint32_t bitInformAll;     /* xBit.informAll (결함 비트맵) 전송용 */
            float32_t adcTemperature;  /* 보드 온도 센서값 */
        } TxData;
        struct {
            uint32_t reserved1;       /* 예약 필드 (구 270V 전원 상태) */
            uint32_t ibitClearReq;    /* IBIT 수행 시작 시 에러 초기화 요청 플래그 (1=요청) */
        } RxData;
    } Payload;
} stIpcDataPacket;

/* Message RAM 정의 (CPU1 View)
 * CPU1 -> CM: 0x39000
 * CM -> CPU1: 0x38000 */
#define IPC_CPU1_TO_CM_MSGRAM_ADDR    (0x39000U)
#define IPC_CM_TO_CPU1_MSGRAM_ADDR    (0x38000U)

/* IPC 명령어 정의 */
#define IPC_CMD_CPU1_ETH_TX_DATA      (0x2001U)  /* CPU1 -> CM: 모니터링 데이터 전달 */
#define IPC_CMD_CM_ETH_RX_DATA        (0x2002U)  /* CM -> CPU1: 수신 시퀀스/상태 전달 */
#define IPC_CMD_CM_BOOT_READY         (0x3001U)  /* CM -> CPU1: CM 기동 및 이더넷/인터럽트 초기화 완료 */

extern volatile stIpcDataPacket *pxDataCpu1ToCm;
extern volatile stIpcDataPacket *pxDataCmToCpu1;

/* CM 코어로부터 수신된 체계 이더넷 수신 데이터 구조체 */
typedef struct {
    uint8_t seqNum;
    uint8_t reserved1;
} stEthRxData;

extern volatile stEthRxData xEthRxData;

#endif // CSU_IPC_CPU1_H
