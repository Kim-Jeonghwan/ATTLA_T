/**********************************************************************
   Nexcom Co., Ltd.
   Filename         : hal_Debug.h
   Version          : 00.10
   Description      : W6100 하드웨어 제어 및 디버그 소켓 통신 헤더
   Programmer       : Kim Jeonghwan
   Last Updated     : 2026. 07. 01. (헤더 버전 동기화 및 템플릿 유지)
 **********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 07. 01. - 헤더 버전 동기화 및 템플릿 유지 (코딩 규칙 적용)
 * 2026. 06. 26. - 파일명에서 _cpu1 제거 (리팩토링)
 * 2026. 06. 26. - hal_Ethernet_cpu1 에서 hal_Debug_cpu1 으로 모듈명 변경
 * 2026. 06. 24. - 파일명 리팩토링 (_cpu1 분리)
 * 2026. 06. 23. - 모니터링 포트 변경: 5002
 * 2026. 06. 15. - Initial_W6100() 반환형을 int8_t로 변경하여 하드웨어 미연결 예외 처리 기능 추가
 */

#ifndef HAL_DEBUG_H
#define HAL_DEBUG_H

#include "main_cpu1.h"

/* ************************** [[   define   ]] *********************************************************** */
#define SOCK_UDP_DBG 0
#define PORT_UDP_DBG 5002

/* ************************** [[  function  ]] *********************************************************** */

/**
 * @brief  W6100 초기화 (칩 설정 및 IP/MAC 바인딩)
 * @return 0: 성공, -1: 실패(하드웨어 없음)
 */
int8_t Initial_W6100(void);

// 하드웨어 연결 상태를 외부 모듈(csu_Debug 등)에서 참조하기 위한 플래그
extern uint8_t isW6100Connected;

/**
 * @brief  디버깅 망 설정 및 W6100 외부 인터럽트 PIE 등록
 */
void Debug_Init(void);

/**
 * @brief      W6100 외부 인터럽트 (XINT1) 서비스 루틴
 * @param      void
 * @return     void (__interrupt)
 */
__interrupt void isr_DebugExtInt(void);

/**
 * @brief      W6100 인터럽트 발생 시 패킷 수신 및 파싱
 * @param      void
 * @return     void
 */
void Debug_Process(void);

#endif // HAL_DEBUG_H
