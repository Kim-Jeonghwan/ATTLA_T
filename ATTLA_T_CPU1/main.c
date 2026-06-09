/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : main.c
    Version          : 00.00
    Description      : 메인 백그라운드 루프 및 주기적 태스크 실행
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 08. (주석 템플릿 일괄 적용)
**********************************************************************/

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
@funtion	void main(void)
@brief		CPU1 코어 메인 엔트리 포인트 및 백그라운드 태스크 스케줄러
@param		void
@return		void
@remark	
	- 시스템 초기화 및 CM 코어와의 동기화를 수행한 후, 주기에 맞춰 태스크를 실행합니다.
*/
void main(void)
{
	DSP_Initialization();

	// 이더넷 (W6100) 초기화
	Ethernet_Init();

	// FRAM 초기화
	Fram_Init();

	// 엔코더 (SSI / SPI-C) 초기화
	Encoder_Init();

	// CM 코어와 IPC를 완전 제거하고 CPU1 단독 동작 수행

	// 백그라운드 유휴 루프 (Background Loop)
	while(1u)
	{
		sendScia_SCI_PC();

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
@funtion	static void cycle_1ms(void)
@brief		1ms 주기로 실행되는 주기 Task
@param		void
@return		static void
@remark	
	- 시스템 클럭 계수(Hzcnt)를 증가시키는 등 고속 모니터링 작업을 실시간 처리합니다.
*/
static void cycle_1ms(void)
{
	// W6100 이더넷 송수신 폴링 (1ms 주기)
	Ethernet_Process();

	// 엔코더 위치 업데이트 (1ms 주기)
	Encoder_UpdatePosition();

	// 사용자 코드 작성
	xTimer.Hzcnt++;
}



/*
@funtion	static void cycle_10ms(void)
@brief		10ms 주기로 실행되는 주기 Task
@param		void
@return		static void
@remark	
	- ADC 센서 데이터를 갱신하고 PC로 보낼 통신 메시지를 처리합니다.
*/
static void cycle_10ms(void)
{
    updateAdcData();
    
    // 3. 통신 메시지 송신
    sendSciPcMessage1();
}


/*
@funtion	static void cycle_100ms(void)
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
@funtion	static void cycle_1000ms(void)
@brief		1000ms(1초) 주기로 실행되는 주기 Task
@param		void
@return		static void
@remark	
	- 디바이스 상태 및 저속 이벤트 처리를 위한 예약 필드입니다.
*/
static void cycle_1000ms(void)
{
#if 0 // 2026-06-08 FRAM SPI-D Test Code
    Fram_WriteByte(0x0010u, 0xAAu);
    FramTest = Fram_ReadByte(0x0010u);
    //Fram_WriteByte(0x0000u, 0x00u);
#endif
}
