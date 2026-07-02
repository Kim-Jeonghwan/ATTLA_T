/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : hal_Common.c
    Version          : 00.01
    Description      : 공통 유틸리티 하드웨어 제어 함수
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 07. 01. (헤더 버전 동기화 및 템플릿 유지)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 07. 01. - 헤더 버전 동기화 및 템플릿 유지 (코딩 규칙 적용)
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 */


/* ************************** [[   include  ]]  *********************************************************** */
#include "hal_Common.h"

/* ************************** [[   define   ]]  *********************************************************** */


/* ************************** [[   global   ]]  *********************************************************** */


/* ************************** [[  function  ]]  *********************************************************** */
/*
@function    void hal_Common_InitTempGpio(void)
@brief      추후 삭제될 임시용 테스트 GPIO 핀(34, 146) 초기화
@param      void
@return     void
*/
void hal_Common_InitTempGpio(void)
{
    // **메인 컨트롤 ISR 동작 확인용 임시 LED (GPIO 34 할당)**
    GPIO_setPinConfig(GPIO_34_GPIO34);
    GPIO_setPadConfig(GPIO_PIN_ALIVE_LED, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_ALIVE_LED, GPIO_DIR_MODE_OUT);
    GPIO_writePin(GPIO_PIN_ALIVE_LED, 1U);

    // **통신 동작(Tx/Rx) 깜빡임 상태 모니터링 임시 LED (GPIO 146 할당)**
    GPIO_setPinConfig(GPIO_146_GPIO146);
    GPIO_setDirectionMode(GPIO_PIN_COMM_LED, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(GPIO_PIN_COMM_LED, GPIO_PIN_TYPE_STD);
    GPIO_setMasterCore(GPIO_PIN_COMM_LED, GPIO_CORE_CM);
}
