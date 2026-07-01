/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : hal_Epwm.c
    Version          : 00.03
    Description      : PWM 제어 (디버거 연결 시 Free Run 대응 및 클럭 분주 개선)
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 07. 01. (초기화 구문 상세 한글 주석 추가)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 07. 01. - 초기화 구문 상세 한글 주석 추가 (코딩 규칙 적용)
 * 2026. 06. 15. - SysCtl_setEPWMClockDivider(1) 추가로 200MHz TBCLK 정상 확보
 * 2026. 06. 15. - 디버거 연결 상태에서 록업 방지를 위해 FREE_RUN 모드 적용
 * 2026. 06. 12. - EPWM1 인터럽트 발생 설정(INT_EPWM1) 추가 (ADC 폴링 및 시퀀스 스위칭용)
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 모터 1x PWM Duty 제어용 Epwm_SetMotorDuty_1x() 구현
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */


#include "hal_Epwm.h"
/* ************************** [[   define   ]]  *********************************************************** */



/* ************************** [[   global   ]]  *********************************************************** */
/* ISR 정적 선언 (제거됨) */

/*
@function    void Initial_EpwmTimer(void)
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
    GPIO_setPadConfig(0U, GPIO_PIN_TYPE_STD); // 표준 출력(Push-Pull) 모드 적용
    GPIO_setDirectionMode(0U, GPIO_DIR_MODE_OUT); // 핀을 출력으로 설정
    EDIS;

    /* EPWM1 클럭 활성화 */
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM1); // 내부 주변장치 클럭 공급 트리거

    /* EPWM 클럭 분주비를 1로 설정 (EPWMCLK = SYSCLK = 200MHz) */
    SysCtl_setEPWMClockDivider(SYSCTL_EPWMCLK_DIV_1);

    /* EPWM1 리셋 후 초기화 */
    EPWM_setTimeBaseCounterMode(EPWM_TIMER1_BASE, EPWM_COUNTER_MODE_UP_DOWN); // 삼각파(Up-Down) 모드 - 모터 제어 및 센터 얼라인 PWM에 적합
    EPWM_setTimeBasePeriod(EPWM_TIMER1_BASE, (uint16_t)EPWM_TIMER1_PERIOD); // 주기 설정 (10,000 = 100us)
    EPWM_setTimeBaseCounter(EPWM_TIMER1_BASE, 0U); // 타이머 카운터를 0부터 시작하도록 강제 초기화

    /* 프리스케일러 설정: CLKDIV=1, HSPCLKDIV=1 → TBCLK = EPWMCLK = 200MHz */
    EPWM_setClockPrescaler(EPWM_TIMER1_BASE,
                           EPWM_TIMER1_CLK_DIV,
                           EPWM_TIMER1_HCLK_DIV);

    // [BUG FIX] CCS 디버거 사용 시 메인 진입이나 Breakpoint 등에서 멈췄을 때 EPWM 카운터가 완전히 죽어버리는 현상을 방지합니다.
    EPWM_setEmulationMode(EPWM_TIMER1_BASE, EPWM_EMULATION_FREE_RUN); // 에뮬레이터 일시 정지 중에도 모터 및 PWM 클럭이 멈추지 않도록 프리런(Free-Run) 모드 적용

    // 액션 한정기 설정 (Symmetric Active High PWM)
    // Up-count CMPA 도달 시 Low, Down-count CMPA 도달 시 High (Duty 가변을 위한 기본 정책)
    EPWM_setActionQualifierAction(EPWM_TIMER1_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);
    EPWM_setActionQualifierAction(EPWM_TIMER1_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_HIGH, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);

    // 최초 기동 시에는 출력 정지(Force Low) 상태로 설정합니다. (의도치 않은 모터 급발진 차단용 안전장치)
    EPWM_setActionQualifierContSWForceAction(EPWM_TIMER1_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_SW_OUTPUT_LOW);

    /* ADC 트리거용 SOCA 활성화 (Zero Event 시 발생) */
    EPWM_enableADCTrigger(EPWM_TIMER1_BASE, EPWM_SOC_A);
    EPWM_setADCTriggerSource(EPWM_TIMER1_BASE, EPWM_SOC_A, EPWM_SOC_TBCTR_ZERO); // 업-다운 모드에서 카운터가 0이 될 때 ADC 변환을 시작하여 노이즈가 가장 적은 시점에 계측
    EPWM_setADCTriggerEventPrescale(EPWM_TIMER1_BASE, EPWM_SOC_A, 1U); /* 매 1회(매 PWM 주기마다) 마다 SOCA 트리거 전송 */

    /* 시스템 제어 시퀀스용 EPWM1 인터럽트 활성화 (Zero Event 시 발생) */
    EPWM_setInterruptSource(EPWM_TIMER1_BASE, EPWM_INT_TBCTR_ZERO); // 카운터가 0이 되는(Zero) 하단 정점 포인트에서 메인 제어 루프 인터럽트(10kHz) 발생
    EPWM_enableInterrupt(EPWM_TIMER1_BASE);
    EPWM_setInterruptEventCount(EPWM_TIMER1_BASE, 1U); // 1 주기마다 ISR 동작 보장

    // 클럭 동기화 (ePWM 활성화)
    EALLOW;
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC); // ePWM 모듈들 간의 동기를 맞추고 카운터를 일제히 구동 시작
    EDIS;
}

/*
@function    void Epwm_SetMotorDuty_1x(float32_t dutyPercent)
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


