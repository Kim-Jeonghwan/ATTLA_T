/**********************************************************************
   Nexcom Co., Ltd.
   Filename         : main_cm.c
   Version          : 00.00
   Description      : CM 코어 메인 루프 및 태스크
   Programmer       : Kim Jeonghwan
   Last Updated     : 2026. 06. 23. (CM 코어 기동 및 동기화/이더넷 제어 로직 생성)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 23. - CM 코어 기동 및 동기화/이더넷 제어 로직 생성
 */

#include "main_cm.h"

// --- 정적 함수 선언 ---
static void Cycle_2ms(void);
static void Cycle_1ms(void);
static void Cycle_10ms(void);
static void Cycle_100ms(void);
static void Cycle_1000ms(void);

/*
@function   main
@brief      CM 코어 메인 엔트리 포인트 및 백그라운드 태스크 스케줄러
@param      void
@return     int
@remark
    - 시스템 및 통신 디바이스(IPC, Ethernet, Timer)를 기동하고 백그라운드 라운드 로빈 방식으로 주기별 태스크를 조율합니다.
*/
int main(void)
{
    /* --- CM 코어 기본 하드웨어 및 클럭 초기화 --- */
    CM_init(); 
    
    /* --- 인터럽트 벡터 테이블을 RAM으로 복사 및 활성화 --- */
    /* 반드시 Initial_IPC() 등의 인터럽트 등록 함수보다 '먼저' 호출되어야 합니다. */
#ifdef _FLASH
    Interrupt_initRAMVectorTable(vectorTableFlash, vectorTableRAM);
#endif

    /* 2. 통신 및 주변장치 초기화 및 동기화 */
    Initial_IPC();       // CPU1과 안전하게 1단계 하드웨어 동기화 (대기 탈출)

    /* --- [물리 이더넷 프리징 예방] PHY 칩 리셋 해제 후 하드웨어 안정화 대기 딜레이 (최소 200ms) --- */
    /* 컴파일러 최적화 옵션(-O1, -O2, -O3 등)에 의해 NOP 루프 실행 속도가 달라져 PHY 기상 전에 통신을 시도하는 현상 방지 */
    /* CM 클럭(125MHz) 기준 200ms 대기: SysCtl_delay( (125000000 / 3) * 0.2 ) = 8,333,333 사이클 */
    SysCtl_delay(8333333U);

    Initial_Ethernet();  // 딜레이 탈출 즉시 이더넷 및 타이머 기동
    Initial_TIMER();
    
    /* 2.5 전역 인터럽트 활성화 */
    (void)Interrupt_enableInProcessor(); 

    /* --- CM 코어의 모든 초기화 완료 및 기동 상태를 CPU1로 최종 통보 (2단계 핸드셰이크) --- */
    sendIpcMessageToCPU1(IPC_CMD_CM_BOOT_READY, 0U, 0U);

    /* 3. 백그라운드 무한 루프 (Background Loop) */
    while(1)
    {
        /* --- 2ms Task: UDP Reflect MSG 송신 --- */
        while (xTimer.Cycle_2ms >= 1U)
        {
            xTimer.Cycle_2ms -= 1U;
            Cycle_2ms();
        }

        /* --- 1ms Task --- */
        while (xTimer.Cycle_1ms >= 1U)
        {
            xTimer.Cycle_1ms -= 1U;
            Cycle_1ms();
        }

        /* --- 10ms Task --- */
        while (xTimer.Cycle_10ms >= 10U)
        {
            xTimer.Cycle_10ms -= 10U;
            Cycle_10ms();
        }

        /* --- 100ms Task --- */
        while (xTimer.Cycle_100ms >= 100U)
        {
            xTimer.Cycle_100ms -= 100U;
            Cycle_100ms();
        }

        /* --- 1000ms Task --- */
        while (xTimer.Cycle_1000ms >= 1000U)
        {
            xTimer.Cycle_1000ms -= 1000U;
            Cycle_1000ms();
        }
    }
}

// --- 주기별 Task 구현부 ---

/*
@function   Cycle_2ms
@brief      2ms 주기 UDP Reflect MSG 송신 Task (사용 안 함)
@param      void
@return     void
*/
static void Cycle_2ms(void)
{
    // PC에서 데이터 요청 패킷 수신 시 응답하므로, 2ms 주기 자가 전송은 하지 않습니다.
}

/*
@function   Cycle_1ms
@brief      1ms 주기로 실행되는 주기 Task (이더넷 활동 LED 소등 관리)
@param      void
@return     void
*/
static void Cycle_1ms(void)
{
    xTimer.Hzcnt++;

    /* 이더넷 활동 LED 소등 타이머 처리 (RJ-45 동작 구현) */
    if (ethActivityTimer > 0U)
    {
        ethActivityTimer--;
        if (ethActivityTimer == 0U)
        {
            ETH_LED_OFF(); // LED OFF (Active High 기준)
        }
    }
}

/*
@function   Cycle_10ms
@brief      10ms 주기로 실행되는 주기 Task (예비 필드)
@param      void
@return     void
*/
static void Cycle_10ms(void)
{
    // 10ms 작업 내용
}

/*
@function   Cycle_100ms
@brief      100ms 주기로 실행되는 주기 Task (CM LED 토글 및 이더넷 연동 상태 머신 구동)
@param      void
@return     void
*/
static void Cycle_100ms(void)
{

    // [ADD] CM 코어에서 100ms 주기로 이더넷 연동통제안 상태 머신을 돌립니다.
    Ethernet_StateMachine();
}

/*
@function   Cycle_1000ms
@brief      1000ms(1초) 주기로 실행되는 주기 Task (예비 필드)
@param      void
@return     void
*/
static void Cycle_1000ms(void)
{
    // 1000ms 작업 내용
}
