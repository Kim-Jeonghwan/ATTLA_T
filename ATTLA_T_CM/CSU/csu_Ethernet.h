/**********************************************************************
   Nexcom Co., Ltd.
   Filename         : csu_Ethernet.h
   Version          : 00.05
   Description      : CM 코어 체계 이더넷(Raw UDP) 연동통제안 및 프로토콜 정의
   Programmer       : Kim Jeonghwan
   Last Updated     : 2026. 06. 23. (체계 연동통제안(ICD) 상태머신 및 메시지 코드 통합)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 23. - 체계 연동통제안(ICD) 상태머신 및 메시지 코드 통합
 */

#ifndef CSU_ETHERNET_H
#define CSU_ETHERNET_H

#include "main_cm.h"

/* ---------------------------------------------------------------
 * 체계 연동통제안(ICD) ID 및 메시지 코드 정의
 * --------------------------------------------------------------- */
#define ETH_FC_ID               (0x01U)   /* 화포통제컴퓨터 ID (임시) */
#define ETH_MY_ID               (0x02U)   /* ATTLA_T ID (임시) */

/* 메시지 Code 정의 */
#define ETH_CODE_BOOT_DONE      (0x10U)   /* 망 가입 요청 (Boot Done) */
#define ETH_CODE_HEARTBEAT      (0x11U)   /* 상태 정보 (Heartbeat) */
#define ETH_CODE_PBIT_REQ       (0x12U)   /* PBIT 요청 */
#define ETH_CODE_PBIT_REP       (0x13U)   /* PBIT 결과 응답 */
#define ETH_CODE_IBIT_REQ       (0x14U)   /* IBIT 요청 */
#define ETH_CODE_IBIT_REP       (0x15U)   /* IBIT 결과 응답 */
#define ETH_CODE_CBIT_SET       (0x16U)   /* CBIT 전송주기 설정 */
#define ETH_CODE_CBIT_REP       (0x17U)   /* CBIT 주기 송신 (Reflect) */
#define ETH_CODE_POWER_270V     (0x18U)   /* 270V 구동전원 인가 메시지 */
#define ETH_CODE_ACK            (0xFFU)   /* ACK 메시지 */

/* Request ACK 설정 */
#define ETH_ACK_NOT_REQ         (0xFFU)   /* ACK 미요청 */
#define ETH_ACK_REQ             (0x01U)   /* ACK 요청함 */
#define ETH_ACK_NORMAL          (0x10U)   /* 정상 ACK 응답 */
#define ETH_ACK_NACK            (0x11U)   /* NACK 응답 */

/* NACK 사유 (Ack_Info) */
#define ETH_ACK_INFO_OK         (0x0000U) /* 정상 */
#define ETH_ACK_INFO_CS_ERR     (0x0001U) /* 체크섬 오류 */

/* 타임아웃 및 주기 제한 (100ms Task 기준 카운트) */
#define ETH_HEARTBEAT_PERIOD    (1U)      /* 100ms 주기 */
#define ETH_BOOTDONE_PERIOD     (5U)      /* 500ms 주기 */
#define ETH_ACK_TIMEOUT         (1U)      /* ACK 응답 대기시간 (100ms) */
#define ETH_DISCONNECT_LIMIT    (50U)     /* 50회(5초) 미응답 시 통신 두절 */
#define ETH_MAX_RETRY_COUNT     (4U)      /* 최초 1회 + 재전송 3회 = 4회 */

/* ---------------------------------------------------------------
 * 네트워크 설정 (고정 IP)
 * --------------------------------------------------------------- */
/* DSP IP: 192.168.200.10 */
#define ETH_DSP_IP0             (192U)
#define ETH_DSP_IP1             (168U)
#define ETH_DSP_IP2             (200U)
#define ETH_DSP_IP3             (10U)

/* PC (화포통제컴퓨터) IP: 192.168.200.1 (스펙 문서의 게이트웨이 및 PC 고정 대역) */
#define ETH_PC_IP0              (192U)
#define ETH_PC_IP1              (168U)
#define ETH_PC_IP2              (200U)
#define ETH_PC_IP3              (1U)

/* DSP MAC: A8:63:F2:00:38:88 (체계 전용 물리 MAC) */
#define ETH_DSP_MAC0            (0xA8U)
#define ETH_DSP_MAC1            (0x63U)
#define ETH_DSP_MAC2            (0xF2U)
#define ETH_DSP_MAC3            (0x00U)
#define ETH_DSP_MAC4            (0x38U)
#define ETH_DSP_MAC5            (0x88U)

/* PC MAC: EC:9A:0C:14:E8:4B (기본값 설정, 통신 시 Auto-Learning 갱신) */
#define ETH_PC_MAC0             (0xECU)
#define ETH_PC_MAC1             (0x9AU)
#define ETH_PC_MAC2             (0x0CU)
#define ETH_PC_MAC3             (0x14U)
#define ETH_PC_MAC4             (0xE8U)
#define ETH_PC_MAC5             (0x4BU)

/* UDP 포트 */
#define ETH_DSP_RX_PORT         (5001U)   /* DSP 수신 포트 */
#define ETH_PC_RX_PORT          (5001U)   /* PC 수신 포트 (체계 스펙 고정) */

/* ---------------------------------------------------------------
 * 프레임 헤더 크기 및 오프셋 정의 (Raw Packet 빌드용)
 * --------------------------------------------------------------- */
#define ETH_MSG_HEADER_SIZE     (12U)
#define ETH_PAYLOAD_DATA_SIZE   (8U)      /* Reflect 데이터 크기 */
#define ETH_PC_DATA_SIZE        (2U)      /* PC→DSP Update 데이터 크기 (시퀀스, 파형) */
#define ETH_ACK_DATA_SIZE       (4U)      /* ACK 데이터 크기 */
#define ETH_CHECKSUM_SIZE       (2U)

#define ETH_TX_NUM_PKT_DESC     (4U)

#define ETH_HDR_DST_OFFSET      (0U)
#define ETH_HDR_SRC_OFFSET      (6U)
#define ETH_HDR_TYPE_OFFSET     (12U)
#define ETH_HDR_SIZE            (14U)

#define IP_HDR_OFFSET           ETH_HDR_SIZE
#define IP_HDR_VER_IHL          (0x45U)   /* IPv4, 20B 헤더 */
#define IP_HDR_DSCP             (0x00U)
#define IP_TTL                  (64U)
#define IP_PROTO_UDP            (0x11U)
#define IP_HDR_SIZE             (20U)

#define UDP_HDR_OFFSET          (ETH_HDR_SIZE + IP_HDR_SIZE)
#define UDP_HDR_SIZE            (8U)

#define PAYLOAD_OFFSET          (ETH_HDR_SIZE + IP_HDR_SIZE + UDP_HDR_SIZE)

#define TX_REFLECT_FRAME_SIZE   (64U)     /* ETH(14) + IP(20) + UDP(8) + Payload(Header 12 + Data 8 + CS 2 = 22) = 64 */
#define TX_ACK_FRAME_SIZE       (60U)     /* ETH(14) + IP(20) + UDP(8) + Payload(Header 12 + Data 4 + CS 2 = 18) = 60 */
#define MIN_RX_FRAME_SIZE       (ETH_HDR_SIZE + IP_HDR_SIZE + UDP_HDR_SIZE + ETH_MSG_HEADER_SIZE + ETH_CHECKSUM_SIZE)

#define ETH_LED_PIN             (146U)    /* EMAC Tx/Rx 동작확인 LED */
#define ETH_LED_ON()            GPIO_writePin(ETH_LED_PIN, 1U)

/* ---------------------------------------------------------------
 * 연동통제안 상태 머신 구조체 정의
 * --------------------------------------------------------------- */
/* 패킷 통신 상태 머신 (Enum) */
typedef enum {
    STATE_BOOTING = 0,      /* 28V 제어전원 인가 후 망 가입 초기화 */
    STATE_WAIT_BOOT_ACK,    /* Boot Done 500ms 주기 반복 전송 및 ACK 대기 */
    STATE_JOINED,           /* 통신망 가입 완료 (Heartbeat 교환 및 CBIT 수행) */
    STATE_COMM_LOSS         /* 통신 두절 상태 */
} EthState_e;

/* 상태 머신 관리 구조체 */
typedef struct {
    EthState_e  State;              /* 현재 망 가입 상태 */
    uint32_t    LastRecvTimestamp;  /* 화포통제컴퓨터가 보낸 가장 최근 Timestamp 유지 */
    uint16_t    TickCount100ms;     /* 100ms 단위로 증가하는 타이머 틱 */
    uint16_t    TimeoutCount;       /* 통신 두절(Heartbeat 미수신) 100ms 카운트 (최대 50) */
    uint16_t    RetryCount;         /* 패킷 재전송 횟수 (1 ~ 4) */
    uint16_t    WaitAckTimer;       /* ACK 대기 타이머 */
    
    uint16_t    CbitPeriodSec;      /* CBIT 전송 주기(초 단위) */
    uint16_t    CbitTimer100ms;     /* CBIT 송신 타이머 카운트 */
    uint8_t     IbitInProgress;     /* IBIT 수행 중 플래그 */
    uint8_t     Power270VStatus;    /* 270V 구동 전원 상태 */
    
    uint8_t     WaitAckCode;        /* 현재 ACK를 기다리고 있는 Code */
    uint8_t     TxBuffer[256];      /* 재전송용 패킷 버퍼 보관 */
    uint16_t    TxSize;             /* 재전송 패킷 크기 */
} stEthControl;

extern stEthControl xEthCtrl;

/* ---------------------------------------------------------------
 * CM↔CPU1 공유 데이터 구조체
 * --------------------------------------------------------------- */
typedef struct {
    float32_t WaveVal;              /* 파형 값 (C28x 계산값) */
    uint16_t  DspTemp;              /* 온도 x10 스케일 */
    uint8_t   SeqNum;               /* 시퀀스 번호 */
    uint8_t   WaveType;             /* 선택된 파형 종류 */
} stEthSharedData;

typedef struct {
    stEthSharedData txData;         /* CPU1 → CM 데이터 */
    stEthSharedData rxData;         /* CM → CPU1 데이터 */
    uint8_t realPcMac[6];           /* 동적 학습된 PC MAC 주소 */
    uint16_t lastRxSrcPort;         /* 마지막 수신 패킷의 출발지 포트 */
} stEthAppState;

extern stEthAppState xEthApp;
extern uint16_t ethActivityTimer;

/* ---------------------------------------------------------------
 * 함수 프로토타입
 * --------------------------------------------------------------- */
void buildAndSendUdpPacket(uint32_t rxTimestamp, uint8_t msgCode, uint8_t reqAck, const uint8_t *pData, uint16_t dataLen);
void processReceivedEthernetPacket(uint8_t *pPacket, uint16_t length);
void sendAckResponse(uint8_t ackResult, uint16_t ackInfo, uint32_t timestamp, uint8_t targetCode);
void Ethernet_StateMachine(void);

#endif /* CSU_ETHERNET_H */
