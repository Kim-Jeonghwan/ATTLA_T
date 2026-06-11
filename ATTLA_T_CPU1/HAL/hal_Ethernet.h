/**********************************************************************
 Nexcom Co., Ltd.
 Filename         : hal_Ethernet.h
 Version          : 00.01
 Description      : W6100 하드웨어 제어 및 이더넷 소켓 통신 헤더
 Programmer       : Kim Jeonghwan
 Last Updated     : 2026. 06. 11. (주석 표준화 및 레거시 코드 정리)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */


#ifndef HAL_ETHERNET_H
#define HAL_ETHERNET_H

#include "main.h"

/* ************************** [[  function  ]] *********************************************************** */

/**
 * @brief      W6100 하드웨어 초기화 및 IP/MAC 설정
 * @param      void
 * @return     void
 */
void Initial_W6100(void);

/**
 * @brief      이더넷 통신망, 소켓 개방 및 W6100 외부 인터럽트 등록
 * @param      void
 * @return     void
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
