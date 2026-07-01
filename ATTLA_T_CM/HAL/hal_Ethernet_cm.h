/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : hal_Ethernet_cm.h
    Version          : 00.03
    Description      : Ethernet EMAC 드라이버 계층 헤더 (MII 모드, DP8382 PHY)
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 07. 01. (구조체 변수 상세 한글 주석 추가 및 헤더 버전 동기화)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 07. 01. - 구조체 변수 상세 한글 주석 추가 및 헤더 버전 동기화 (코딩 규칙 적용)
 * 2026. 06. 24. - 파일명 리팩토링 (_cm 분리)
 * 2026. 06. 19. - 변수명 규칙 적용 (xHalEth -> xEthDriver 변경)
 * 2026. 06. 19. - 이더넷 전역 변수 캡슐화 적용
 * 2026. 06. 05. - 코드 주석 포맷팅 및 한글화
 */

#ifndef HAL_ETHERNET_CM_H
#define HAL_ETHERNET_CM_H

#include "main_cm.h"

/* ---------------------------------------------------------------
 * EMAC 기본 주소 (hw_memmap.h 정의값 사용)
 * EMAC_BASE     = 0x400C0000U
 * EMAC_SS_BASE  = 0x400C2000U
 * --------------------------------------------------------------- */

/* Rx 버퍼 크기 및 채널 개수 */
#define ETH_RX_NUM_PKT_DESC    (3U)     /* Rx 디스크립터(패킷 버퍼) 개수 */
#define ETH_RX_BUF_SIZE        (1536U)  /* 단일 Rx 버퍼 크기 (1518B 이더넷 최대 + 여유) */
#define ETH_TX_NUM_PKT_DESC    (4U)     /* Tx 디스크립터(패킷 버퍼) 개수 */
#define ETH_TX_BUF_SIZE        (256U)   /* Tx 버퍼 크기 (최대 UDP 패킷 61B 대비 여유) */

/* ---------------------------------------------------------------
 * HAL 계층 이더넷 상태 구조체 캡슐화
 * --------------------------------------------------------------- */
typedef struct {
    Ethernet_Handle hEMAC; // LLD (Low Level Driver) 단에서 반환된 이더넷 MAC 모듈 제어용 인스턴스 핸들
    uint8_t txBuf[ETH_TX_NUM_PKT_DESC][ETH_TX_BUF_SIZE]; // 송신 데이터를 임시로 담아두는 버퍼 (디스크립터와 1:1 매칭)
    uint32_t initRet; // 이더넷 초기화 API 수행 후 반환된 결과 코드 저장 (에러 트래킹용)
} stEthDriverState;

/* 구조체 인스턴스 (csu_Ethernet.c 등에서 사용) */
extern stEthDriverState xEthDriver;

/* ---------------------------------------------------------------
 * 함수 프로토타입
 * --------------------------------------------------------------- */

/* EMAC 초기화 (GPIO 제외 - CPU1 hal_DspInit.c 에서 GPIO 설정) */
void Initial_Ethernet(void);

/* 이더넷 수신 폴링 태스크 (main loop 에서 주기 호출) */
void updateEthernetTask(void);

/* Ethernet_InitConfig 에 등록되는 콜백 함수들 */
Ethernet_Pkt_Desc *App_ethGetPacketBuffer(void);
Ethernet_Pkt_Desc *App_ethRxCallback(Ethernet_Handle hApp, Ethernet_Pkt_Desc *pPkt);
void               App_ethTxCallback(Ethernet_Handle hApp, Ethernet_Pkt_Desc *pPkt);

/* 패킷 버퍼 점유 상태 및 버퍼 획득 API */
bool     Ethernet_isTxAvailable(void);
uint8_t* Ethernet_getTxBuffer(uint8_t descIdx);
void     Ethernet_consumeTxBuffer(void);

/* LLD 드라이버 내 하드폴트 방지용 인터럽트 제어 콜백 래퍼 */
void Platform_enableCoreInterrupt(void);
void Platform_disableCoreInterrupt(void);

#endif /* HAL_ETHERNET_H */

