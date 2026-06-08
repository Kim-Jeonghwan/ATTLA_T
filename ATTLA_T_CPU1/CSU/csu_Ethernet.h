/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Ethernet.h
    Version          : 00.00
    Description      : 이더넷 통신 프로세스 제어 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 08. (주석 템플릿 일괄 적용)
**********************************************************************/

#ifndef CSU_ETHERNET_H
#define CSU_ETHERNET_H

#include "main.h"

/* ************************** [[   global   ]] *********************************************************** */
extern uint8_t flag_2ms_tx;

/* ************************** [[  function  ]] *********************************************************** */
void Ethernet_Init(void);
void Ethernet_Process(void);

#endif // CSU_ETHERNET_H
