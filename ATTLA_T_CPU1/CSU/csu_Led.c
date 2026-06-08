/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Led.c
    Version          : 00.00
    Description      : 시스템 상태 표시 LED 제어 로직
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 08. (주석 템플릿 일괄 적용)
**********************************************************************/

/* ************************** [[   include  ]]  *********************************************************** */
#include "csu_Led.h"


/* ************************** [[   define   ]]  *********************************************************** */



/* ************************** [[   define   ]]  *********************************************************** */


/* ************************** [[   global   ]]  *********************************************************** */
stLedStatus xLed;


/* ************************** [[  static prototype  ]]  *************************************************** */
static void HW_writeLedPin(uint16_t Index, bool State); 
static void HW_toggleLedPin(uint16_t Index);


/* ************************** [[  function  ]]  *********************************************************** */

/*
@funtion    void initGpioDoutLed(void)
@brief      LED 관련 GPIO 초기화
@param      void
@return     void
@remark
    - 보드의 각 LED 핀들을 CPU1 소유의 출력 핀으로 설정합니다.
*/
void initGpioDoutLed(void)
{
    EALLOW;
    
    // nG LED (GPIO30)
    GPIO_setPinConfig(GPIO_30_GPIO30);
    GPIO_setPadConfig(30u, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(30u, GPIO_DIR_MODE_OUT);
    GPIO_setMasterCore(30u, GPIO_CORE_CPU1);

    EDIS;
}

/*
@funtion    void Initial_Led(void)
@brief      LED 모드 초기값 설정
@param      void
@return     void
@remark
    - G LED는 1초 토글, 기타 LED는 기본 OFF 상태로 초기화합니다.
*/
void Initial_Led(void)
{
    // nG LED (GPIO30) 설정
    xLed.lednG.Index  = eLED_nG;
    setLedModeToggle(&xLed.lednG, LED_TOGGLE, 10u); // 1초 주기 토글

    // // ERROR LED (GPIO146) 설정
    // xLed.ledError.Index = eLED_ERROR;
    // setLedModeToggle(&xLed.ledError, LED_NONE, 0u);   // 기본 꺼짐
    // setLedStatus(&xLed.ledError, LED_OFF);

}

/*
@funtion    void updateLedStatus(void)
@brief      LED 상태 머신 업데이트
@param      void
@return     void
@remark
    - 100ms 주기로 호출되며 각 LED의 토글 카운트를 갱신하고 물리 핀 출력을 제어합니다.
    - 내부 온도 센서 값이 LIMIT_TEMP_ERROR를 초과하면 ERROR LED를 점등합니다.
*/
void updateLedStatus(void)
{
    uint16_t i = 0u;
    // nG LED 1개 관리
    stLed *pLed[1];
    
    // 1. 구조체 포인터 배열 매핑
    pLed[0] = &xLed.lednG;
//    pLed[1] = &xLed.ledError;

    // 2. 전체 LED 상태 업데이트 루프
    for(i = 0u; i < 1u; i++)
    {
        if(pLed[i]->Toggle == LED_TOGGLE)
        {
            if(pLed[i]->Temp == 0u)
            {
                HW_toggleLedPin(pLed[i]->Index);
                pLed[i]->Temp = pLed[i]->Time;
            }
            else
            {
                pLed[i]->Temp--;
            }
        }
        else
        {
            // State 값에 따라 물리 핀 출력
            HW_writeLedPin(pLed[i]->Index, pLed[i]->State);
        }
    }
}


/*
@funtion    void setLedStatus(stLed *pLed, bool State)
@brief      개별 LED 상태(점등/소등) 강제 설정
@param      pLed: 대상 LED 구조체 포인터
@param      State: LED_ON(1) 또는 LED_OFF(0)
@return     void
*/
void setLedStatus(stLed *pLed, bool State)
{
    if(pLed != NULL)
    {
        if(pLed->State != State)
        {
            pLed->State = State;
            pLed->Toggle = LED_NONE; 
            HW_writeLedPin(pLed->Index, State);
        }
    }
}


/*
@funtion    void setLedModeToggle(stLed *pLed, bool State, uint16_t Time)
@brief      LED 토글 모드 설정
@param      pLed: 대상 LED 구조체 포인터
@param      State: LED_TOGGLE(1) 또는 LED_NONE(0)
@param      Time: 토글 유지 카운트 (100ms 단위)
@return     void
*/
void setLedModeToggle(stLed *pLed, bool State, uint16_t Time)
{
    if(pLed != NULL)
    {
        pLed->Toggle = State;
        pLed->Time   = Time;
        pLed->Temp   = 0u;
    }
}



/*
@funtion    static void HW_writeLedPin(uint16_t Index, bool State)
@brief      물리 GPIO 핀 상태 출력 (Write)
@param      Index: LED 식별 인덱스
@param      State: 출력 상태
@return     static void
*/
static void HW_writeLedPin(uint16_t Index, bool State)
{
	switch(Index)
	{
	case eLED_nG:
		GPIO_writePin(eLED_nG, (uint32_t)State);
		break;

	// case eLED_ERROR:
	// 	GPIO_writePin(eLED_ERROR, (uint32_t)State);
	// 	break;

	default:
		// MISRA
		break;
	}
}

/*
@funtion    static void HW_toggleLedPin(uint16_t Index)
@brief      물리 GPIO 핀 상태 반전 (Toggle)
@param      Index: LED 식별 인덱스
@return     static void
*/
static void HW_toggleLedPin(uint16_t Index)
{
	switch(Index)
	{
	case eLED_nG:
		GPIO_togglePin(eLED_nG);
		break;

	// case eLED_ERROR:
	// 	GPIO_togglePin(eLED_ERROR);
	// 	break;

	default:
		// MISRA
		break;
	}
}
