/**********************************************************************
   Nexcom Co., Ltd.
   Filename         : main_cpu1.c
   Version          : 00.08
   Description      : 메인 백그라운드 루프 및 주기적 태스크 실행 (main.c ➡️ main_cpu1.c 리팩토링)
   Programmer       : Kim Jeonghwan
   Last Updated     : 2026. 06. 23. (main_cpu1.c 로 물리 파일명 리팩토링)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 23. - main_cpu1.c 로 물리 파일명 리팩토링
 * 2026. 06. 23. - CM 코어 IPC 동기화 추가 및 W6100 상태머신 제거
 * 2026. 06. 23. - 코딩 규칙 및 구조 불일치 사항 리팩토링 반영
 * 2026. 06. 17. - 명명 규칙 위반 리팩토링 연동
 * 2026. 06. 16. - 이더넷 프로토콜 연동 통제안에 따른 상태머신(Ethernet_StateMachine) 호출 로직 추가
 * 2026. 06. 12. - CM 및 IPC 관련 주석 제거
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 전역 변수를 상태 구조체(xSysCtrl 등)로 통합 적용
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */

#include "main_cpu1.h"

/* ************************** [[   define   ]]  *********************************************************** */

/* ************************** [[   global   ]]  *********************************************************** */
uint16_t FramTest = 0U;

/* ************************** [[  static prototype  ]]  *************************************************** */
// 1ms 주기(1000Hz)
static void cycle_1ms(void);

// 10ms(100Hz)
static void cycle_10ms(void);

// 100ms(10Hz)
static void cycle_100ms(void);

// 1000ms(1Hz)
static void cycle_1000ms(void);

/* ************************** [[  function  ]]  *********************************************************** */
/*
@function   main
@brief      CPU1 코어 메인 엔트리 포인트 및 백그라운드 태스크 스케줄러
@param      void
@return     void
@remark	
	- 시스템 초기화를 수행한 후, 주기에 맞춰 태스크 실행을 시작합니다.
*/
void main(void)
{
	System_Initialization();

    /* --- [핵심 개선] CM 코어와 IPC 하드웨어 동기화 (기동 대기) --- */
    Initial_IPC();

    // [CRITICAL BUG FIX]: 전역 인터럽트(INTM) 및 실시간 디버깅(DBGM) 활성화
    // 이 두 줄이 없으면 C28x 코어는 PIE 인터럽트를 무시하므로 어떠한 ISR도 발생하지 않고 IPC 수신도 불가합니다.
    EINT;
    ERTM;

    /* --- CM 코어가 통신 및 주변장치 기동을 마칠 때까지 안전하게 대기 --- */
    while (xIpcState.isCmReady == false)
    {
        // CM 코어가 모든 부팅 및 이더넷/인터럽트 초기화를 끝내고 READY 신호를 쏠 때까지 대기
    }

	// 동적 인터럽트 스위칭의 시작점 (Offset -> PBIT -> Main 순차 진행)
	// EPWM1(100us) 타이머 인터럽트에 최초 오프셋 조정 ISR을 매핑하고 활성화합니다.
	EALLOW;
	Interrupt_register(INT_EPWM1, &Offset_Isr);
	EDIS;
	Interrupt_enable(INT_EPWM1);

    // PBIT 완료 대기 (이더넷 초기화 및 통신은 초기 점검(PBIT) 통과 후 안전하게 수행)
    while (xSysCtrl.isPbitComplete == 0U)
    {
        // 100us 인터럽트(Offset_Isr -> Pbit_Isr) 체인이 완료될 때까지 백그라운드 대기
    }

	// 이더넷 (W6100) 하드웨어 초기화
	// CM 코어 활성화와 별개로, W6100은 192.168.200.11:5002 로 모니터링 동작을 수행함.
	Ethernet_Init();
	
	// 이더넷 상태 머신 초기화 (CSU)
	Ethernet_ProtocolInit();
	
	// 외부 통신 인터럽트(W6100) 활성화
	Interrupt_enable(INT_XINT1);

    // 이더넷 및 외부 통신 인터럽트 초기화가 완료되었으므로,
    // PBIT 완료 시 일시 중지되었던 EPWM1 인터럽트를 재활성화하여 메인 컨트롤 ISR 구동을 시작함.
    Interrupt_enable(INT_EPWM1);

	// 백그라운드 유휴 루프 (Background Loop)
	while(1U)
	{
		// sendScia_SCI_PC(); // SCI 미사용 요청으로 주석 처리

		while(xTimer.Cycle_1ms >= 1U)
		{
			xTimer.Cycle_1ms -= 1U;
			cycle_1ms();
		}

		while(xTimer.Cycle_10ms >= 10U)
		{
			xTimer.Cycle_10ms -= 10U;
			cycle_10ms();
		}

		while(xTimer.Cycle_100ms >= 100U)
		{
			xTimer.Cycle_100ms -= 100U;
			cycle_100ms();
		}

		while(xTimer.Cycle_1000ms >= 1000U)
		{
			xTimer.Cycle_1000ms -= 1000U;
			cycle_1000ms();
		}
	}
}

/*
@function   cycle_1ms
@brief      1ms 주기로 실행되는 주기 Task
@param      void
@return     void
*/
static void cycle_1ms(void)
{
	xTimer.Hzcnt++;
}

/*
@function   cycle_10ms
@brief      10ms 주기로 실행되는 주기 Task
@param      void
@return     void
*/
static void cycle_10ms(void)
{
}

/*
@function   cycle_100ms
@brief      100ms 주기로 실행되는 주기 Task
@param      void
@return     void
*/
static void cycle_100ms(void)
{
    updateLedStatus();
    
    // [REMOVED] W6100 체계 통신 상태 머신(100ms 주기 동작) 제거 ➡️ CM 코어가 체계 통신을 담당
}

/*
@function   cycle_1000ms
@brief      1000ms(1초) 주기로 실행되는 주기 Task
@param      void
@return     void
*/
static void cycle_1000ms(void)
{
}
