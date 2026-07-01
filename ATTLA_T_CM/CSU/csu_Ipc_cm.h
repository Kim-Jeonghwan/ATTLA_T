/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Ipc_cm.h
    Version          : 00.07
    Description      : CM IPC 및 공유 메모리 통신 프로토콜 정의
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 07. 01. (구조체 변수 상세 한글 주석 추가 및 헤더 버전 동기화)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 07. 01. - 구조체 변수 상세 한글 주석 추가 및 헤더 버전 동기화 (코딩 규칙 적용)
 * 2026. 06. 22. - GSRAM 잔재 주석을 MSGRAM 기준으로 수정
 * 2026. 06. 22. - 파형 선택 기능 지원을 위해 구조체 내부 필드명 변경 (status -> waveType, sineValue -> waveValue)
 * 2026. 06. 22. - MSGRAM 기반 공유 메모리 사용으로 stIpcDataPacket에 seqCount 추가
 * 2026. 06. 22. - 포인터 변수명을 pxDataCpu1ToCm / pxDataCmToCpu1 로 변경
 * 2026. 06. 22. - CM 코어는 GSRAM에 쓰기 권한이 없으므로(Hard Fault 발생), 통신 수단을 MSGRAM으로 원복
 */

#ifndef CSU_IPC_CM_H
#define CSU_IPC_CM_H

#include "main_cm.h"

// IPC 통신용 공용 데이터 구조체 (32비트 정렬)
typedef struct {
    uint32_t seqCount;      // 동기화용 Seqlock 카운터 (짝수=완료, 홀수=쓰기중)
    uint32_t Command;       // IPC 명령어 코드 (예: 송신 요청, 수신 알림 등)
    uint32_t Status;        // 시스템 상태 또는 에러 플래그 레지스터
    uint32_t Address;       // 메모리 복사 또는 접근을 위한 기준 포인터 주소
    union {
        uint32_t PayloadRaw[16];   // 32비트 단위 16단어(64바이트) 길이의 순수 원시 데이터 배열
        struct {
            uint32_t bitInformAll;    // 전체 모듈 자체 점검(BIT) 결과 플래그 모음 (CPU1에서 수집)
            float32_t adcTemperature; // ADC로 측정한 시스템 온도 (섭씨)
        } TxData;                   // CPU1 코어에서 CM 코어로 송신하는 데이터 포맷
        struct {
            uint32_t reserved1;       /* 예약 필드 (구 270V 전원 상태) */
            uint32_t ibitClearReq;    /* IBIT 수행 시작 시 에러 초기화 요청 플래그 (1=요청) */
        } RxData;                   // CM 코어에서 CPU1 코어로 송신하는(CPU1 입장에서는 수신) 데이터 포맷
    } Payload;
} stIpcDataPacket;

// Message RAM 정의 (CM View)
#define IPC_CPU1_TO_CM_MSGRAM_ADDR    0x20080000U
#define IPC_CM_TO_CPU1_MSGRAM_ADDR    0x20082000U

// --- 이더넷 패킷 전용 공유 메모리 설정 ---
// 1. 이더넷 패킷 데이터 구조 (MSGRAM에 배치될 데이터)
typedef struct {
    uint32_t Length;        // 패킷 길이 (Bytes)
    uint32_t Reserved;      // 64비트 정렬(Alignment)을 맞추기 위한 여분 필드
    uint8_t  Data[1514];    // 실제 이더넷 프레임 데이터 버퍼 (표준 이더넷 MTU 크기 기준)
} stEthPacketBuffer;

// IPC 명령어 정의
#define IPC_CMD_ETH_RCV_NOTIFY    0x1001U  /* CM -> CPU1: 패킷 수신 알림 */
#define IPC_CMD_ETH_XMT_REQUEST   0x1002U  /* CPU1 -> CM: 패킷 송신 요청 */
#define IPC_CMD_CPU1_ETH_TX_DATA  0x2001U  /* CPU1 -> CM: 온도+시퀀스 전달 */
#define IPC_CMD_CM_ETH_RX_DATA    0x2002U  /* CM -> CPU1: 수신 SeqNum/Cmd 전달 */
#define IPC_CMD_CM_BOOT_READY     0x3001U  /* CM -> CPU1: CM 기동 및 주변기기 초기화 완료 */

extern volatile stIpcDataPacket *pxDataCpu1ToCm;
extern volatile stIpcDataPacket *pxDataCmToCpu1;

// 제거됨: void recvIpcCpu1Message(uint32_t command, uint32_t addr, uint32_t data);
// 제거됨: void processBulkDataFromCPU1(void);

#endif // CSU_IPC_CM_H
