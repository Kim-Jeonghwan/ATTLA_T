/**********************************************************************
   Nexcom Co., Ltd.
   Filename         : csu_Ipc_cpu1.c
   Version          : 00.00
   Description      : CM Core IPC 통신 프로토콜 및 공유 메모리 구현
   Programmer       : Kim Jeonghwan
   Last Updated     : 2026. 06. 23. (MSGRAM 영역 포인터 매핑 및 수신 공유 변수 할당)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 23. - MSGRAM 영역 포인터 매핑 및 수신 공유 변수 할당
 */

#include "csu_Ipc_cpu1.h"

/* MSGRAM 영역에 구조체 포인터 할당 */
volatile stIpcDataPacket *pxDataCpu1ToCm = (volatile stIpcDataPacket *)IPC_CPU1_TO_CM_MSGRAM_ADDR;
volatile stIpcDataPacket *pxDataCmToCpu1 = (volatile stIpcDataPacket *)IPC_CM_TO_CPU1_MSGRAM_ADDR;

/* CM→CPU1 수신 공유 변수 */
volatile stEthRxData xEthRxData = {0U, 0U};
