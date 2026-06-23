/**********************************************************************
 Nexcom Co., Ltd.
 Filename         : hal_Ethernet.h
 Version          : 00.06
 Description      : W6100 하드웨어 제어 및 이더넷 소켓 통신 헤더
 Programmer       : Kim Jeonghwan
 Last Updated     : 2026. 06. 23. (main.h -> main_cpu1.h 인클루드 명칭 리팩토링)
 **********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 23. - main.h -> main_cpu1.h 인클루드 명칭 리팩토링
 * 2026. 06. 23. - 모니터링 포트 변경: 5002
 * 2026. 06. 23. - 코딩 규칙 및 구조 불일치 사항 리팩토링 반영
 * 2026. 06. 15. - Initial_W6100() 반환형을 int8_t로 변경하여 하드웨어 미연결 예외 처리 기능 추가
 * 2026. 06. 12. - 소켓 및 포트 매크로 상수를 헤더(.h)로 이동 (글로벌 룰 적용)
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */


#ifndef HAL_ETHERNET_H
#define HAL_ETHERNET_H

#include "main_cpu1.h"

/* ************************** [[   define   ]] *********************************************************** */
#define SOCK_UDP_COM 0
#define PORT_UDP_COM 5002

/* ************************** [[  function  ]] *********************************************************** */

/**
 * @brief  W6100 초기화 (칩 설정 및 IP/MAC 바인딩)
 * @return 0: 성공, -1: 실패(하드웨어 없음)
 */
int8_t Initial_W6100(void);

// 하드웨어 연결 상태를 외부 모듈(csu_Ethernet 등)에서 참조하기 위한 플래그
extern uint8_t isW6100Connected;

/**
 * @brief  이더넷 기본 망 설정 및 W6100 외부 인터럽트 PIE 등록
 */
void Ethernet_Init(void);

/**
 * @brief      W6100 외부 인터럽트 (XINT1) 서비스 루틴
 * @param      void
 * @return     void (__interrupt)
 */
__interrupt void isr_EthernetExtInt(void);

/**
 * @brief      W6100 인터럽트 발생 시 패킷 수신 및 즉각 응답
 * @param      void
 * @return     void
 */
void Ethernet_Process(void);

#endif // HAL_ETHERNET_H
