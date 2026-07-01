/**********************************************************************
   Nexcom Co., Ltd.
   Filename         : csu_Ethernet_cm.h
   Version          : 00.09
   Description      : CM 코어 체계 이더넷(Raw UDP) 연동통제안 및 프로토콜 정의
   Programmer       : Kim Jeonghwan
   Last Updated     : 2026. 07. 01. (구조체 변수 상세 한글 주석 추가 및 헤더 버전 동기화)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 07. 01. - 구조체 변수 상세 한글 주석 추가 및 헤더 버전 동기화 (GEMINI 코딩 규칙 적용)
 * 2026. 06. 29. - 사용자 요청으로 초기 PC MAC 주소 원복 (EC:9A:0C:14:E8:4B)
 * 2026. 06. 29. - 초기 PC MAC 주소 브로드캐스트로 변경하여 통신 가입 지연 해결
 * 2026. 06. 24. - 파일명 리팩토링 (_cm 분리)
 * 2026. 06. 23. - 체계 연동통제안(ICD) 상태머신 및 메시지 코드 통합
 */

#ifndef CSU_ETHERNET_CM_H
#define CSU_ETHERNET_CM_H

#include "main_cm.h"

/* ---------------------------------------------------------------
 * 체계 연동통제안(ICD) ID 및 메시지 코드 정의
 * --------------------------------------------------------------- */
#define ETH_FC_ID               (0x01U)   /* 화포통제컴퓨터 ID (임시) */
#define ETH_MY_ID               (0x02U)   /* ATTLA_T ID (임시) */

/* 메시지 Code 정의 */
#define ETH_CODE_BOOT_DONE      (0x10U)   /* 망 가입 요청 (Boot Done) */
#define ETH_CODE_STATUS_REQ     (0x11U)   /* 상태 정보 (100ms 통신상태 확인) */
#define ETH_CODE_PBIT_REQ       (0x12U)   /* PBIT 요청 */
#define ETH_CODE_PBIT_REP       (0x13U)   /* PBIT 결과 응답 */
#define ETH_CODE_IBIT_REQ       (0x14U)   /* IBIT 요청 */
#define ETH_CODE_IBIT_REP       (0x15U)   /* IBIT 결과 응답 */
#define ETH_CODE_CBIT_SET       (0x16U)   /* CBIT 전송주기 설정 */
#define ETH_CODE_CBIT_REP       (0x17U)   /* CBIT 주기 송신 (Reflect) */

#define ETH_CODE_IBIT_DONE      (0x19U)   /* IBIT 완료 통보 */
#define ETH_CODE_IBIT_RES_REQ   (0x1AU)   /* IBIT 결과 요청 */
#define ETH_CODE_CBIT_STOP      (0x1BU)   /* CBIT 전송 중지 요청 */
#define ETH_CODE_CBIT_REQ       (0x1CU)   /* CBIT 전송 시작 요청 */
#define ETH_CODE_ACK            (0xFFU)   /* ACK 메시지 */

/* Priority 설정 */
#define ETH_PRIORITY_EMERG      (0x01U)   /* 비상정지 우선순위 */
#define ETH_PRIORITY_NORMAL     (0x02U)   /* 일반 우선순위 */

/* Request ACK 설정 */
#define ETH_ACK_NOT_REQ         (0xFFU)   /* ACK 미요청 */
#define ETH_ACK_REQ             (0x01U)   /* ACK 요청함 */
#define ETH_ACK_NORMAL          (0x10U)   /* 정상 ACK 응답 */
#define ETH_ACK_NACK            (0x11U)   /* NACK 응답 */

/* NACK 사유 (Ack_Info) */
#define ETH_ACK_INFO_OK         (0x0000U) /* 정상 */
#define ETH_ACK_INFO_CS_ERR     (0x0001U) /* 체크섬 오류 */

/* 타임아웃 및 주기 제한 (100ms Task 기준 카운트) */

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

/* PC (화포통제컴퓨터) IP: 192.168.200.100 (테스터기 전용 로컬 IP 대역 맞춤) */
#define ETH_PC_IP0              (192U)
#define ETH_PC_IP1              (168U)
#define ETH_PC_IP2              (200U)
#define ETH_PC_IP3              (100U)

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
#define ETH_PC_RX_PORT          (5003U)   /* PC 수신 포트 (포트 충돌 방지) */

/* ---------------------------------------------------------------
 * 프레임 헤더 크기 및 오프셋 정의 (Raw Packet 빌드용)
 * --------------------------------------------------------------- */
#define ETH_MSG_HEADER_SIZE     (12U)
#define ETH_PAYLOAD_DATA_SIZE   (8U)      /* Reflect 데이터 크기 */
#define ETH_PC_DATA_SIZE        (2U)      /* PC→DSP Update 데이터 크기 (시퀀스, 파형) */
#define ETH_ACK_DATA_SIZE       (4U)      /* ACK 데이터 크기 */
#define ETH_CHECKSUM_SIZE       (2U)


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
    STATE_JOINED,           /* 통신망 가입 완료 (상태 교환 및 CBIT 수행) */
    STATE_COMM_LOSS         /* 통신 두절 상태 */
} EthState_e;

/* 상태 머신 관리 구조체 */
typedef struct {
    EthState_e  State;              /* 현재 망 가입 상태 (부팅 대기, ACK 대기, 통신망 가입 완료 등) */
    uint32_t    LastRecvTimestamp;  /* 화포통제컴퓨터가 보낸 가장 최근 Timestamp 유지 (연속성 검증용) */
    uint16_t    TickCount100ms;     /* 100ms 단위로 증가하는 타이머 틱 카운터 */
    uint16_t    TimeoutCount;       /* 통신 두절(상태정보 미수신) 상태를 감지하기 위한 100ms 카운트 (최대 50회 누적 시 두절) */
    uint16_t    RetryCount;         /* 패킷 재전송 시도 횟수 (1회 ~ 최대 4회) */
    uint16_t    WaitAckTimer;       /* 특정 메시지 전송 후 ACK 응답을 기다리는 타이머 */
    
    uint16_t    CbitPeriodSec;      /* CBIT(연속 자체 점검) 메시지 전송 주기(초 단위 설정값) */
    uint16_t    CbitTimer100ms;     /* CBIT 메시지 주기 송신 타이머 카운트 */
    uint8_t     CbitTxFlag;         /* CBIT 결과 주기적 전송 상태 플래그 (1: 전송 진행 중, 0: 전송 중지 상태) */
    uint8_t     IbitInProgress;     /* IBIT(초기화 자체 점검) 수행 상태 플래그 (1: 진행 중, 2: 완료 대기, 0: 종료) */
    uint16_t    IbitTimer;          /* IBIT 수행 소요 시간을 시뮬레이션하기 위한 지연 타이머 */
    uint16_t    IbitDuration;       /* 통제 컴퓨터가 지정한 IBIT 수행 소요 시간(초 단위) */

    uint8_t     WaitAckCode;        /* 현재 시스템이 응답 ACK를 기다리고 있는 대상 메시지 Code */
    uint8_t     TxBuffer[256];      /* 재전송 실패 시 재전송을 수행하기 위해 마지막 패킷을 임시 보관하는 버퍼 */
    uint16_t    TxSize;             /* 버퍼에 저장된 재전송 패킷의 실제 크기 */
} stEthControl;

extern stEthControl xEthCtrl;

/* ---------------------------------------------------------------
 * CM↔CPU1 공유 데이터 구조체
 * --------------------------------------------------------------- */
typedef struct {
    float32_t WaveVal;              /* CPU1(C28x)에서 계산된 파형 데이터 값 */
    uint16_t  DspTemp;              /* DSP 내부 온도 센서 측정값 (실제 온도 * 10 스케일 적용) */
    uint8_t   SeqNum;               /* 통신 데이터 동기화를 위한 8비트 시퀀스 카운터 번호 */
    uint8_t   WaveType;             /* 통제기에서 선택된 파형의 종류 (사인파, 구형파 등) 식별자 */
} stEthSharedData;

typedef struct {
    stEthSharedData txData;         /* CPU1에서 CM 코어로 전달되어 이더넷으로 송신될 데이터 블록 */
    stEthSharedData rxData;         /* CM 코어가 이더넷으로 수신하여 CPU1으로 전달할 데이터 블록 */
    uint8_t realPcMac[6];           /* ARP 또는 수신 패킷을 통해 동적으로 학습된 화포통제컴퓨터 실제 물리 MAC 주소 */
    uint16_t lastRxSrcPort;         /* 마지막으로 수신된 UDP 패킷의 출발지 포트 (가변 포트 대응용 응답 목적지) */
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
