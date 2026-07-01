/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Debug.h
    Version          : 00.06
    Description      : 노트북 디버깅망 동적 IP 라우팅 프로토콜 및 텔레메트리 정의
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 07. 01. (구조체 변수 상세 한글 주석 추가)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 07. 01. - 구조체 변수 상세 한글 주석 추가 (코딩 규칙 적용)
 * 2026. 06. 26. - 파일명에서 _cpu1 제거 (리팩토링)
 * 2026. 06. 26. - 모듈명을 csu_Ethernet_cpu1에서 csu_Debug_cpu1으로 변경
 * 2026. 06. 26. - 기존 체계연동(FC) 통제안 로직 전면 삭제 및 노트북 디버그용 상태머신 적용
 * 2026. 06. 26. - 동적 IP 캡처를 위한 stDbgControl 구조체 추가
 */

#ifndef CSU_DEBUG_H
#define CSU_DEBUG_H

#include "main_cpu1.h"

/* ************************** [[   define   ]] *********************************************************** */

/* --- ID 매크로 설정 --- */
#define DBG_PC_ID               0x10    // 노트북 PC ID (임의 설정)
#define DBG_MY_ID               0x11    // ATTLA_T ID (임의 설정)

/* --- 메시지 Code 정의 --- */
#define DBG_CODE_TELEMETRY      0x20    // 모터 상태 텔레메트리 자동 송신
#define DBG_CODE_REQ_STATE      0x21    // 상태 수동 요청 (PC -> DSP)
#define DBG_CODE_CLR_FAULT      0x22    // 폴트 초기화 요청 (PC -> DSP)

/* --- 타임아웃 --- */
#define DBG_DISCONNECT_LIMIT    50      // 50회(5초) 미응답/미요청 시 통신 두절로 간주

/* ************************** [[  structure ]] *********************************************************** */

/* 패킷 헤더 (12 Bytes, 체계 연동 통제안 구조를 호환 유지) */
typedef struct {
    uint32_t Timestamp;     // 송신 시점의 틱 카운트 또는 0 (단위: 시스템 틱)
    uint8_t Source_ID;      // 송신 장치 고유 ID (예: 0x11 ATTLA_T)
    uint8_t Dest_ID;        // 수신 목적지 고유 ID (예: 0x10 PC)
    uint8_t Code;           // 메시지 명령어 식별 코드 (예: 텔레메트리, 폴트 초기화 등)
    uint8_t Request_ACK;    // 수신 응답(ACK) 요청 플래그 (현재 미사용)
    uint8_t Priority;       // 메시지 전송 우선순위 레벨
    uint8_t Send_Count;     // 현재 메시지의 전송 시도 횟수
    uint16_t Data_Length;   // 뒤따르는 페이로드(Data)의 바이트 길이 (단위: Byte)
} stDbgHeader;

/* 디버그 텔레메트리 구조체 (임의 정의 - 확장 가능) */
typedef struct {
    uint32_t systemTick;    // 시스템 구동 후 누적된 100ms 틱 카운트 (단위: 100ms)
    float currentA;         // A상 인버터 출력 모터 전류 (단위: A)
    float currentB;         // B상 인버터 출력 모터 전류 (단위: A)
    float currentC;         // C상 인버터 출력 모터 전류 (단위: A)
    uint32_t faultStatus;   // 현재 모터 및 시스템의 폴트/에러 상태 통합 비트맵
} stDbgTelemetry;

/* 상태 머신 관리 구조체 */
typedef struct {
    uint8_t     isActive;           // 통신 활성화 상태 플래그 (1: PC 연결 및 활성화, 0: 대기)
    uint8_t     targetIp[4];        // 동적 캡처된 통신 대상(PC)의 IP 주소 배열
    uint16_t    targetPort;         // 동적 캡처된 통신 대상(PC)의 UDP 포트 번호
    
    uint16_t    TimeoutCount;       // 통신 두절을 감지하기 위한 대기 카운터 (단위: 100ms 틱)
    uint16_t    TickCount100ms;     // 모듈 내부에서 관리하는 100ms 단위 타이머 틱 카운트
    
    uint8_t     TxBuffer[1024];     // UDP 송신용 데이터를 직렬화하기 위한 임시 버퍼 (단위: Byte)
} stDbgControl;

extern stDbgControl xDbgCtrl;

/* ************************** [[  function  ]] *********************************************************** */

/**
 * @brief  디버깅 통신 파이프라인(상태머신) 초기화
 */
void Debug_ProtocolInit(void);

/**
 * @brief  체크섬 계산 함수 (기존 방식 유지)
 * @param  pData    계산할 데이터 버퍼 포인터
 * @param  length   전체 길이 (마지막 체크섬 2바이트 포함)
 * @return uint16_t 계산된 하위 2바이트 체크섬 반환
 */
uint16_t Debug_CalculateChecksum(const uint8_t *pData, uint16_t length);

/**
 * @brief  디버그 통신망 100ms 주기 상태 머신 (텔레메트리 송신 등)
 */
void Debug_StateMachine(void);

/**
 * @brief  수신된 디버그 UDP 패킷 파싱 및 동적 IP 캡처
 * @param  pRxBuf      수신된 패킷 버퍼 포인터
 * @param  length      수신된 패킷 전체 길이
 * @param  pSenderIp   송신자(PC) IP 주소 (4바이트)
 * @param  senderPort  송신자(PC) Port 번호
 */
void Debug_ParsePacket(uint8_t *pRxBuf, uint16_t length, uint8_t *pSenderIp, uint16_t senderPort);

/**
 * @brief  디버그 메시지 전송 (동적 캡처된 IP/Port로 전송)
 */
void Debug_SendMessage(uint8_t code, uint8_t *pData, uint16_t dataLen);


#endif // CSU_DEBUG_H
