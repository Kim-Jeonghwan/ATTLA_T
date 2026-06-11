/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : hal_Ethernet.h
    Version          : 00.01
    Description      : W6100 하드웨어 제어 및 이더넷 소켓 통신 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 11. (csu_Ethernet과 hal_W6100 구조 병합)
**********************************************************************/

#ifndef HAL_ETHERNET_H
#define HAL_ETHERNET_H

#include "main.h"

/* ************************** [[  function  ]] *********************************************************** */
void Initial_W6100(void);
void Ethernet_Init(void);
__interrupt void isr_EthernetExtInt(void);
void Ethernet_Process(void);

#endif // HAL_ETHERNET_H
