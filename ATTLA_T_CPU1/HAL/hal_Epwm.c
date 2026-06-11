/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : hal_Epwm.c
    Version          : 00.01
    Description      : EPWM 제어 및 초기화 로직
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 08. (주석 템플릿 일괄 적용)
**********************************************************************/

#include "hal_Epwm.h"
/* ************************** [[   define   ]]  *********************************************************** */



/* ************************** [[   global   ]]  *********************************************************** */
/* ISR 정적 선언 */
static __interrupt void isr_Epwm1Timer100us(void);

/*
@funtion    static void initEpwm7aGpio(void)
@brief      EPWM7A(GPIO12) 핀 설정
@param      void
@return     static void
@remark
    - GPIO 12번 핀을 EPWM7A 출력으로 할당합니다.
*/
static void initEpwm7aGpio(void)
{
    EALLOW;
    GPIO_setPinConfig(GPIO_12_EPWM7A);
    GPIO_setPadConfig(12, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(12, GPIO_DIR_MODE_OUT);
    EDIS;
}

/*
@funtion    void Initial_Epwm7a(void)
@brief      EPWM7A 모듈 초기화
@param      void
@return     void
@remark
    - 기본 설정: 100Hz, 50% Duty 로 구성하되, 최초 기동 시에는 출력 정지(Force Low) 상태로 설정합니다.
*/
void Initial_Epwm7a(void)
{
    initEpwm7aGpio();

    EALLOW;
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM7); // ePWM7 클럭 활성화
    EDIS;

    // 기본 설정: Up-count mode (Driverlib 적용)
    EPWM_setTimeBaseCounterMode(EPWM7_BASE, EPWM_COUNTER_MODE_UP);
    EPWM_disablePhaseShiftLoad(EPWM7_BASE);
    EPWM_disableSyncOutPulseSource(EPWM7_BASE, EPWM_SYNC_OUT_PULSE_ON_ALL); // 동기화 비활성화
    EPWM_setTimeBaseCounter(EPWM7_BASE, 0u);

    // 액션 한정기 설정
    EPWM_setActionQualifierAction(EPWM7_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA); // CMPA 도달 시 Low
    EPWM_setActionQualifierAction(EPWM7_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_HIGH, EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);   // 0 도달 시 High
    
    // PWM 출력 오프 (Force Low)로 시작
    EPWM_setActionQualifierContSWForceAction(EPWM7_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_SW_OUTPUT_LOW);

    // 클럭 동기화 (ePWM7 활성화)
    EALLOW;
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC); 
    EDIS;
}

/*
@funtion    void Initial_EpwmTimer(void)
@brief      EPWM1 기반 100us 타이머 초기화 및 ISR 등록
@param      void
@return     void
@remark
    - EPWM1 모듈을 UP-DOWN 카운터 모드로 설정합니다.
    - Period = 10,000 (200MHz 기준 100us 주기)
    - Zero Event 인터럽트 활성화 후 ISR 등록
*/
void Initial_EpwmTimer(void)
{
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

    /* Zero Event 인터럽트 활성화 (카운터가 0이 될 때마다 인터럽트) */
    EPWM_setInterruptSource(EPWM_TIMER1_BASE, EPWM_INT_TBCTR_ZERO);
    EPWM_enableInterrupt(EPWM_TIMER1_BASE);
    EPWM_setInterruptEventCount(EPWM_TIMER1_BASE, 1U); /* 매 1회 이벤트마다 인터럽트 */

    /* ADC 트리거용 SOCA 활성화 (Zero Event 시 발생) */
    EPWM_enableADCTrigger(EPWM_TIMER1_BASE, EPWM_SOC_A);
    EPWM_setADCTriggerSource(EPWM_TIMER1_BASE, EPWM_SOC_A, EPWM_SOC_TBCTR_ZERO);
    EPWM_setADCTriggerEventPrescale(EPWM_TIMER1_BASE, EPWM_SOC_A, 1U); /* 매 1회 마다 SOCA */

    /* PIE 인터럽트 등록 및 활성화 */
    Interrupt_register(INT_EPWM1, isr_Epwm1Timer100us);
}

/*
@funtion    static __interrupt void isr_Epwm1Timer100us(void)
@brief      EPWM1 타이머 100us Zero Event ISR - 시스템 운용 제어 파이프라인 실행
@param      void
@return     static __interrupt void
@remark
    - 100us 마다 호출되어 전체 시스템 제어 로직을 순차적으로 수행합니다.
*/
static __interrupt void isr_Epwm1Timer100us(void)
{
    // 시스템 제어 및 운용 로직 일괄 수행
    csu_Control_SystemOperation();

    /* EPWM 인터럽트 플래그 클리어 */
    EPWM_clearEventTriggerInterruptFlag(EPWM_TIMER1_BASE);

    /* PIE ACK */
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP3);
}

