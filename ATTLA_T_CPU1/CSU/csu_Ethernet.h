/**********************************************************************
 Nexcom Co., Ltd.
 Filename         : csu_Ethernet.h
 Version          : 00.00
 Description      : 이더넷(W6100) 연동통제안 프로토콜 및 상태 머신 정의
 Programmer       : Kim Jeonghwan
 Last Updated     : 2026. 06. 16. (이더넷 통신 프로토콜 및 상태 머신 신규 작성)
**********************************************************************/

/*
 * Modification History
 * 2026. 06. 16. - 체계 연동 통제안(Heartbeat 전송, 270V 파싱, IBIT) 상세 구현
 * 2026. 06. 16. - 이더넷 통신 프로토콜 규격 구조체 및 상태 머신 신규 작성
 */

#ifndef CSU_ETHERNET_H
#define CSU_ETHERNET_H

#include "main.h"

/* ************************** [[   define   ]] *********************************************************** */

/* --- ID 매크로 설정 --- */
#define ETH_FC_ID               0x01    // 화포통제컴퓨터 ID (임시)
#define ETH_MY_ID               0x02    // ATTLA_T ID (임시)

/* --- 메시지 Code 정의 --- */
#define ETH_CODE_BOOT_DONE      0x10    // 망 가입 요청 (Boot Done)
#define ETH_CODE_HEARTBEAT      0x11    // 상태 정보 (Heartbeat)
#define ETH_CODE_PBIT_REQ       0x12    // PBIT 요청
#define ETH_CODE_PBIT_REP       0x13    // PBIT 결과
#define ETH_CODE_IBIT_REQ       0x14    // IBIT 요청
#define ETH_CODE_IBIT_REP       0x15    // IBIT 결과
#define ETH_CODE_CBIT_SET       0x16    // CBIT 전송주기 설정
#define ETH_CODE_CBIT_REP       0x17    // CBIT 주기 송신 (Reflect)
#define ETH_CODE_POWER_270V     0x18    // 270V 구동전원 인가 메시지
#define ETH_CODE_ACK            0xFF    // ACK 메시지

/* --- Request ACK 설정 --- */
#define ETH_ACK_NOT_REQ         0xFF    // ACK 미요청
#define ETH_ACK_REQ             0x01    // ACK 요청함
#define ETH_ACK_NORMAL          0x10    // 정상 ACK 응답
#define ETH_ACK_NACK            0x11    // NACK 응답

/* --- NACK 사유 (Ack_Info) --- */
#define ETH_ACK_INFO_OK         0x00    // 정상
#define ETH_ACK_INFO_CS_ERR     0x01    // 체크섬 오류

/* --- 타임아웃 및 주기 제한 (100ms Task 기준 카운트) --- */
#define ETH_HEARTBEAT_PERIOD    1       // 100ms 주기
#define ETH_BOOTDONE_PERIOD     5       // 500ms 주기
#define ETH_ACK_TIMEOUT         1       // ACK 응답 대기시간 (100ms)
#define ETH_DISCONNECT_LIMIT    50      // 50회(5초) 미응답 시 통신 두절

#define ETH_MAX_RETRY_COUNT     4       // 최초 1회 + 재전송 3회 = 4회

/* ************************** [[  structure ]] *********************************************************** */

/* 패킷 통신 상태 머신 (Enum) */
typedef enum {
    ETH_STATE_INIT = 0,     // W6100 소켓 초기화 후 망 가입 대기 (Step 1)
    ETH_STATE_BOOT_DONE,    // 망 가입 요청 (Boot Done) 전송 중 (Step 2)
    ETH_STATE_LINKED        // 망 가입 완료, Heartbeat 정상 교환 상태 (Step 3, 4)
} EthState_e;

/* 패킷 헤더 (12 Bytes) */
typedef struct {
    uint32_t Timestamp;     // 화포통제컴퓨터 Tickcount 또는 0x00000000
    uint8_t Source_ID;      // 송신 장치 ID
    uint8_t Dest_ID;        // 수신 목적지 장치 ID
    uint8_t Code;           // 메시지 명령어 코드
    uint8_t Request_ACK;    // 0xFF: ACK 미요청, 0x01: ACK 요청함 / (ACK 시 0x10, 0x11)
    uint8_t Priority;       // 우선순위
    uint8_t Send_Count;     // 전송 횟수 (1 ~ 4)
    uint16_t Data_Length;   // Data 필드의 크기 (Byte)
} stEthHeader;

/* 일반 메시지 구조체 (최대 1024 Bytes 구성) */
typedef struct {
    stEthHeader Header;
    uint8_t Payload[1010];  // 가변 길이 페이로드 공간
    uint16_t Checksum;      // (실제 사용할 때는 가변 길이 뒤에 붙음)
} stEthMessage;

/* ACK 응답 메시지 구조체 (18 Bytes 고정) */
typedef struct {
    stEthHeader Header;
    uint16_t Code_Info;     // 대상 메시지 Code
    uint16_t Ack_Info;      // 정상(0x00), 체크섬 오류(0x01) 등
    uint16_t Checksum;      // 체크섬
} stEthAckMessage;

/* 상태 머신 관리 구조체 */
typedef struct {
    EthState_e  State;              // 현재 망 가입 상태
    uint32_t    LastRecvTimestamp;  // 화포통제컴퓨터가 보낸 가장 최근 Timestamp 유지
    uint16_t    TickCount100ms;     // 100ms 단위로 증가하는 타이머 틱
    uint16_t    TimeoutCount;       // 통신 두절(Heartbeat 미수신) 100ms 카운트 (최대 50)
    uint16_t    RetryCount;         // 패킷 재전송 횟수 (1 ~ 4)
    uint16_t    WaitAckTimer;       // ACK 대기 타이머
    
    uint16_t    CbitPeriodSec;      // CBIT 전송 주기(초 단위)
    uint16_t    CbitTimer100ms;     // CBIT 송신 타이머 카운트
    uint8_t     IbitInProgress;     // IBIT 수행 중 플래그 (1: 진행중, 0: 대기)
    uint8_t     Power270VStatus;    // 270V 구동 전원 상태 (0: 미인가, 1: 인가)
    
    // 재전송을 위한 패킷 버퍼 백업 (간단하게 송신 구조체 유지)
    uint8_t     WaitAckCode;        // 현재 ACK를 기다리고 있는 Code
    uint8_t     TxBuffer[256];      // 재전송용 패킷 버퍼 보관
    uint16_t    TxSize;             // 재전송 패킷 크기
} stEthControl;

extern stEthControl xEthCtrl;

/* ************************** [[  function  ]] *********************************************************** */

/**
 * @brief  이더넷 파이프라인(상태머신) 초기화
 */
void csu_Ethernet_Init(void);

/**
 * @brief  체크섬 계산 함수
 * @param  pData    계산할 데이터 버퍼 포인터
 * @param  length   전체 길이 (마지막 체크섬 2바이트 포함 길이)
 * @return uint16_t 계산된 하위 2바이트 체크섬 반환
 */
uint16_t csu_Ethernet_CalculateChecksum(const uint8_t *pData, uint16_t length);

/**
 * @brief  이더넷 100ms 주기 상태 머신 (망 가입 및 Heartbeat)
 */
void csu_Ethernet_StateMachine(void);

/**
 * @brief  수신된 UDP 패킷 파싱
 * @param  pRxBuf   수신된 패킷 버퍼 포인터
 * @param  length   수신된 패킷 전체 길이
 */
void csu_Ethernet_ParsePacket(uint8_t *pRxBuf, uint16_t length);

/**
 * @brief  기본 메시지 전송 (ACK 재전송 연동)
 */
void csu_Ethernet_SendMessage(uint8_t code, uint8_t reqAck, uint8_t *pData, uint16_t dataLen);


#endif // CSU_ETHERNET_H
