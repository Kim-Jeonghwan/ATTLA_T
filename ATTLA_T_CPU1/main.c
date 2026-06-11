/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : main.c
    Version          : 00.02
    Description      : 메인 백그라운드 루프 및 주기적 태스크 실행
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 11. (주석 표준화 및 레거시 코드 정리)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 전역 변수를 상태 구조체(xSysCtrl 등)로 통합 적용
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */


#include "main.h"



/* ************************** [[   define   ]]  *********************************************************** */



/* ************************** [[   global   ]]  *********************************************************** */
uint16_t FramTest = 0u;



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
@function	void main(void)
@brief		CPU1 코어 메인 엔트리 포인트 및 백그라운드 태스크 스케줄러
@param		void
@return		void
@remark	
	- 시스템 초기화 및 CM 코어와의 동기화를 수행한 후, 주기에 맞춰 태스크를 실행합니다.
*/
void main(void)
{
	System_Initialization();

	// 100us 시스템 운용 파이프라인 인터럽트는 InitialAdc() 내부에서 INT_ADCA1 활성화를 통해 이미 시작됨.

	// 전류센서 Offset 조정 대기
	// 100us 타이머 내부에서 오프셋 조정이 완료될 때까지 대기
	while(xSysCtrl.isOffsetCalibrated == 0U)
	{
	}

	// 초기점검(PBIT) 대기
	// 100us 타이머 내부에서 PBIT가 완료될 때까지 대기
	while(xSysCtrl.isPbitComplete == 0U)
	{
	}

	// 이더넷 (W6100) 초기화 및 통신 인터럽트 활성화
	Ethernet_Init();
	Interrupt_enable(INT_XINT1);

	// 백그라운드 유휴 루프 (Background Loop)
	while(1u)
	{
		sendScia_SCI_PC(); // 디버깅용 비동기 SCI 송신 처리

		while(xTimer.Cycle_1ms >= 1u)
		{
			xTimer.Cycle_1ms -= 1u;
			cycle_1ms();
		}

		while(xTimer.Cycle_10ms >= 10u)
		{
			xTimer.Cycle_10ms -= 10u;
			cycle_10ms();
		}

		while(xTimer.Cycle_100ms >= 100u)
		{
			xTimer.Cycle_100ms -= 100u;
			cycle_100ms();
		}

		while(xTimer.Cycle_1000ms >= 1000u)
		{
			xTimer.Cycle_1000ms -= 1000u;
			cycle_1000ms();
		}
	}
}



/*
@function	static void cycle_1ms(void)
@brief		1ms 주기로 실행되는 주기 Task
@param		void
@return		static void
@remark	
	- 시스템 클럭 계수(Hzcnt)를 증가시키는 등 고속 모니터링 작업을 실시간 처리합니다.
*/
static void cycle_1ms(void)
{
	// 사용자 코드 작성 (실시간 처리는 100us 인터럽트로 이동됨)
	xTimer.Hzcnt++;
}



/*
@function	static void cycle_10ms(void)
@brief		10ms 주기로 실행되는 주기 Task
@param		void
@return		static void
@remark	
	- ADC 센서 데이터를 갱신하고 PC로 보낼 통신 메시지를 처리합니다.
*/
static void cycle_10ms(void)
{
    // 통신 메시지 송신 등 덜 중요한 백그라운드 태스크
    sendSciPcMessage1();
}


/*
@function	static void cycle_100ms(void)
@brief		100ms 주기로 실행되는 주기 Task
@param		void
@return		static void
@remark	
	- 시스템 상태 모니터링 및 LED 상태를 갱신합니다.
*/
static void cycle_100ms(void)
{
    updateLedStatus();
}




/*
@function	static void cycle_1000ms(void)
@brief		1000ms(1초) 주기로 실행되는 주기 Task
@param		void
@return		static void
@remark	
	- 디바이스 상태 및 저속 이벤트 처리를 위한 예약 필드입니다.
*/
static void cycle_1000ms(void)
{
}
