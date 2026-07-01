/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Led.c
    Version          : 00.09
    Description      : 시스템 상태 표시 LED 제어 로직
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 07. 01. (초기화 구문 상세 한글 주석 추가)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 07. 01. - 초기화 구문 상세 한글 주석 추가 (코딩 규칙 적용)
 * 2026. 06. 30. - 사용자 지시에 따라 nG LED(GPIO 145)의 500ms 주기 점멸 토글 기능 복구
 * 2026. 06. 30. - nG LED(GPIO 145)의 토글 점멸 기능을 삭제하고 기본 상태(OFF)로 초기화
 * 2026. 06. 15. - 성능 및 가독성을 위해 불필요한 Led_TogglePin, Led_WritePin 래퍼 제거 후 SDK의 GPIO_ API 직접 호출
 * 2026. 06. 15. - 34번 핀과 점멸 주기 일치화 (100ms 상태 머신 기준 1.1초 -> 0.5초 토글)
 * 2026. 06. 15. - stLed 구조체 변경에 따른 파라미터 자료형(bool -> uint16_t) 수정
 * 2026. 06. 12. - 내부 온도 센서 관련 레거시 주석 삭제
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
    - nG LED는 500ms(0.5초) 주기로 점멸(토글) 되도록 초기화합니다.
*/
void Initial_Led(void)
{
    // nG LED (GPIO 145) 설정
    xLed.lednG.Index  = eLED_nG;                   // nG LED의 GPIO 핀 번호를 구조체에 초기화 할당
    setLedModeToggle(&xLed.lednG, LED_TOGGLE, 4u); // 500ms 주기 점멸 모드로 설정 (100ms 단위 기준 카운트 4)
}

/*
@function    void updateLedStatus(void)
@brief      LED 상태 머신 업데이트
@param      void
@return     void
@remark
    - 100ms 주기로 호출되며 각 LED의 토글 카운트를 갱신하고 물리 핀 출력을 제어합니다.
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
                GPIO_togglePin(pLed[i]->Index);
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
            GPIO_writePin(pLed[i]->Index, (uint32_t)pLed[i]->State);
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
void setLedStatus(stLed *pLed, uint16_t State)
{
    if(pLed != NULL)
    {
        if(pLed->State != State)
        {
            pLed->State = State;
            pLed->Toggle = LED_NONE; 
            GPIO_writePin(pLed->Index, (uint32_t)State);
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
void setLedModeToggle(stLed *pLed, uint16_t State, uint16_t Time)
{
    if(pLed != NULL)
    {
        pLed->Toggle = State;
        pLed->Time   = Time;
        pLed->Temp   = 0u;
    }
}




