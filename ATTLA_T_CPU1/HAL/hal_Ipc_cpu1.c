/**********************************************************************
   Nexcom Co., Ltd.
   Filename         : hal_Ipc_cpu1.c
   Version          : 00.00
   Description      : CM Core IPC 디바이스 드라이버 및 동기화 구현
   Programmer       : Kim Jeonghwan
   Last Updated     : 2026. 06. 23. (CM 기동 검출을 위한 IPC ISR 및 동기화 기동 구현)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 23. - CM 기동 검출을 위한 IPC ISR 및 동기화 기동 구현
 */

#include "hal_Ipc_cpu1.h"

/* CM 기동 상태 구조체 인스턴스 */
volatile stIpcState xIpcState = {false};

/* 정적 ISR 선언 */
static __interrupt void isrIpcFromCM(void);

/*
@function   Initial_IPC_Clear
@brief      CM 코어 기동 전에 IPC 제어 레지스터 플래그 사전 정리
@param      void
@return     void
*/
void Initial_IPC_Clear(void)
{
    /* IPC 제어 레지스터의 모든 플래그 강제 클리어 (이전 오염 플래그 제거) */
    IPC_init(IPC_CPU1_L_CM_R);
}

/*
@function   Initial_IPC
@brief      IPC 통신 초기화 및 CM 코어와 동기화
@param      void
@return     void
*/
void Initial_IPC(void)
{
    /* CM으로부터 수신받을 인터럽트 등록 (IPC_INT0) */
    IPC_registerInterrupt(IPC_CPU1_L_CM_R, IPC_INT0, isrIpcFromCM);

    /* CM 코어와 IPC_FLAG31을 통해 동기화 수행 (CM의 락업 방지) */
    IPC_sync(IPC_CPU1_L_CM_R, IPC_FLAG31);
}

/*
@function   isrIpcFromCM
@brief      CM 코어로부터 수신된 IPC 인터럽트 서비스 루틴 (IPC_INT0)
@param      void
@return     static __interrupt void
*/
static __interrupt void isrIpcFromCM(void)
{
    uint32_t uiCmd  = 0U;
    uint32_t uiAddr = 0U;
    uint32_t uiData = 0U;
    bool     bRet   = false;

    bRet = IPC_readCommand(IPC_CPU1_L_CM_R, IPC_FLAG0, IPC_ADDR_CORRECTION_DISABLE, &uiCmd, &uiAddr, &uiData);

    if (bRet)
    {
        if (uiCmd == IPC_CMD_CM_BOOT_READY)
        {
            /* CM 코어 이더넷 및 통신 준비 완료 플래그 활성화 */
            xIpcState.isCmReady = true;
        }
        else
        {
            /* 방어적 default 분기: 정적분석 DAPA SCR-G 만족용 */
        }

        IPC_ackFlagRtoL(IPC_CPU1_L_CM_R, IPC_FLAG0);
    }

    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP11);
}
