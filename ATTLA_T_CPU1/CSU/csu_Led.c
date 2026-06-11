/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Led.c
    Version          : 00.02
    Description      : 시스템 상태 표시 LED 제어 로직
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 11. (주석 표준화 및 레거시 코드 정리)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 * 2026. 06. 11. - 함수명 접두어(csu_, hal_) 제거 리팩토링
 */


/* ************************** [[   include  ]]  *********************************************************** */
#include "csu_Led.h"


/* ************************** [[   define   ]]  *********************************************************** */



/* ************************** [[   define   ]]  *********************************************************** */


/* ************************** [[   global   ]]  *********************************************************** */
stLedStatus xLed;


/* ************************** [[  static prototype  ]]  *************************************************** */


/* ************************** [[  function  ]]  *********************************************************** */

/*
@function    void Initial_Led(void)
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
}

/*
@function    void updateLedStatus(void)
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

    // 2. 전체 LED 상태 업데이트 루프
    for(i = 0u; i < 1u; i++)
    {
        if(pLed[i]->Toggle == LED_TOGGLE)
        {
            if(pLed[i]->Temp == 0u)
            {
                Led_TogglePin(pLed[i]->Index);
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
            Led_WritePin(pLed[i]->Index, pLed[i]->State);
        }
    }
}


/*
@function    void setLedStatus(stLed *pLed, bool State)
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
            Led_WritePin(pLed->Index, State);
        }
    }
}


/*
@function    void setLedModeToggle(stLed *pLed, bool State, uint16_t Time)
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




