/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : hal_Epwm.c
    Version          : 00.02
    Description      : EPWM 제어 및 초기화 로직 (GPIO0 EPWM1A 모터 PWM 통합)
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 11. (모터 1x PWM Duty 제어 함수 추가)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 11. - 모터 1x PWM Duty 제어용 Epwm_SetMotorDuty_1x() 구현
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */


#include "hal_Epwm.h"
/* ************************** [[   define   ]]  *********************************************************** */



/* ************************** [[   global   ]]  *********************************************************** */
/* ISR 정적 선언 (제거됨) */

/*
@funtion    void Initial_EpwmTimer(void)
@brief      EPWM1 기반 100us 타이머 및 모터 PWM (EPWM1A / GPIO0) 초기화
@param      void
@return     void
@remark
    - EPWM1 모듈을 UP-DOWN 카운터 모드로 설정합니다.
    - Period = 10,000 (200MHz 기준 100us 주기)
    - EPWM1A (GPIO 0)를 모터 드라이버 PWM 출력 핀으로 설정합니다.
    - Zero Event 인터럽트 활성화 후 ISR 등록
*/
void Initial_EpwmTimer(void)
{
    /* EPWM1A (GPIO 0) 출력 핀 설정 */
    EALLOW;
    GPIO_setPinConfig(GPIO_0_EPWM1A);
    GPIO_setPadConfig(0U, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(0U, GPIO_DIR_MODE_OUT);
    EDIS;

    /* EPWM1 클럭 활성화 */
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM1);

    /* EPWM1 리셋 후 초기화 */
    EPWM_setTimeBaseCounterMode(EPWM_TIMER1_BASE, EPWM_COUNTER_MODE_UP_DOWN);
    EPWM_setTimeBasePeriod(EPWM_TIMER1_BASE, (uint16_t)EPWM_TIMER1_PERIOD);
    EPWM_setTimeBaseCounter(EPWM_TIMER1_BASE, 0U);

    /* 프리스케일러 설정: CLKDIV=1, HSPCLKDIV=1 → TBCLK = SYSCLK = 200MHz */
    EPWM_setClockPrescaler(EPWM_TIMER1_BASE,
                           EPWM_TIMER1_CLK_DIV,
                           EPWM_TIMER1_HCLK_DIV);

    // 액션 한정기 설정 (Symmetric Active High PWM)
    // Up-count CMPA 도달 시 Low, Down-count CMPA 도달 시 High
    EPWM_setActionQualifierAction(EPWM_TIMER1_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);
    EPWM_setActionQualifierAction(EPWM_TIMER1_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_HIGH, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);

    // 최초 기동 시에는 출력 정지(Force Low) 상태로 설정합니다.
    EPWM_setActionQualifierContSWForceAction(EPWM_TIMER1_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_SW_OUTPUT_LOW);

    /* ADC 트리거용 SOCA 활성화 (Zero Event 시 발생) */
    EPWM_enableADCTrigger(EPWM_TIMER1_BASE, EPWM_SOC_A);
    EPWM_setADCTriggerSource(EPWM_TIMER1_BASE, EPWM_SOC_A, EPWM_SOC_TBCTR_ZERO);
    EPWM_setADCTriggerEventPrescale(EPWM_TIMER1_BASE, EPWM_SOC_A, 1U); /* 매 1회 마다 SOCA */

    // 클럭 동기화 (ePWM 활성화)
    EALLOW;
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC); 
    EDIS;
}

/*
@funtion    void Epwm_SetMotorDuty_1x(float32_t dutyPercent)
@brief      1x PWM 모드 Duty 및 SW Force 제어
@param      dutyPercent: 0.0 ~ 100.0 (%)
@return     void
*/
void Epwm_SetMotorDuty_1x(float32_t dutyPercent)
{
    // EPWM1 주기 기준 Duty 계산 (Period = 10000 이라면, Duty% = CMPA)
    // float 기반 연산 수행
    uint16_t cmpa_val = (uint16_t)(dutyPercent * ((float32_t)EPWM_TIMER1_PERIOD / 100.0f));

    // 제한 값 검사
    if (cmpa_val >= EPWM_TIMER1_PERIOD)
    {
        cmpa_val = EPWM_TIMER1_PERIOD;
    }

    // Duty가 0%일 때는 하드웨어 SW Force Low 상태로 확실히 차단하고, 그렇지 않을 때 해제
    if (cmpa_val == 0U)
    {
        EPWM_setActionQualifierContSWForceAction(EPWM_TIMER1_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_SW_OUTPUT_LOW);
    }
    else
    {
        EPWM_setActionQualifierContSWForceAction(EPWM_TIMER1_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_SW_DISABLED);
    }

    EPWM_setCounterCompareValue(EPWM_TIMER1_BASE, EPWM_COUNTER_COMPARE_A, cmpa_val);
}


