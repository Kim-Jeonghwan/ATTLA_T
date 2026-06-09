/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : hal_Adc.c
    Version          : 00.02
    Description      : ADC 및 내부 온도 센서 하드웨어 제어
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 09. (VREF 설정 코드 제거)
**********************************************************************/

/* ************************** [[   include  ]] *********************************************************** */
#include "hal_Adc.h"



// EPWM1 트리거 사용하므로 별도의 ADC 전용 EPWM8/9 초기화 불필요

/* ************************** [[   define   ]] *********************************************************** */
#define DEFAULT_MAVE_COUNT  100u   // 이동 평균 필터 카운트
#define DEFAULT_PWM_HZ      100000u // ePWM8 트리거 주파수 (100kHz 조정)

/* ************************** [[   global   ]] *********************************************************** */
uint16_t adcResult = 0u; // 실시간 온도 센서 원시 결과 전역 변수 (csu_Adc.c에서 참조)
AdcRawData_t adcRawData; // 실시간 ADC 채널 RAW 데이터 버퍼

/* ************************** [[  function  ]] *********************************************************** */

/*
@funtion    void InitialAdc(void)
@brief      ADC 초기화 기동 및 인터럽트 등록
@param      void
@return     void
*/
void InitialAdc(void)
{
    InitAdcModules(); // ADC 모듈 하드웨어 초기화

    // ADC 인터럽트 등록 및 활성화 (원래 주기 동작 복원)
    Interrupt_register(INT_ADCA1, &AdcaIsr);
    Interrupt_enable(INT_ADCA1);
}

/*
@funtion    void InitAdcModules(void)
@brief      ADC 모듈 초기 설정 (Driverlib 적용)
@param      void
@return     void
*/
void InitAdcModules(void)
{
    // -------------------------------------------------------------------------
    // 1. ADC-A 초기화 (모터 전류, 28V, 5V 계측 담당)
    // -------------------------------------------------------------------------
    EALLOW;
    ADC_setPrescaler(ADCA_BASE, ADC_CLK_DIV_4_0); // ADCCLK = SYSCLK / 4 (50MHz)
    ADC_setMode(ADCA_BASE, ADC_RESOLUTION_12BIT, ADC_MODE_SINGLE_ENDED);
    ADC_setInterruptPulseMode(ADCA_BASE, ADC_PULSE_END_OF_CONV);
    ADC_enableConverter(ADCA_BASE);
    DEVICE_DELAY_US(1000U); // 아날로그 회로 파워업 대기

    // ADCA SOC 매핑 설정 (SOC 번호 = 채널 번호 일치)
    // 트리거: EPWM1_SOCA (10kHz)
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER2, ADC_TRIGGER_EPWM1_SOCA, ADC_CH_ADCIN2, 14u); // SOC2: A2 (ISEN_MOT)
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER3, ADC_TRIGGER_EPWM1_SOCA, ADC_CH_ADCIN3, 14u); // SOC3: A3 (ISEN_BRK)
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER4, ADC_TRIGGER_EPWM1_SOCA, ADC_CH_ADCIN4, 14u); // SOC4: A4 (VSEN_28V)
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER5, ADC_TRIGGER_EPWM1_SOCA, ADC_CH_ADCIN5, 14u); // SOC5: A5 (5VD)

    // ADCA의 마지막 변환 완료 시점(SOC5)에서 인터럽트 INT1 발생
    ADC_setInterruptSource(ADCA_BASE, ADC_INT_NUMBER1, ADC_SOC_NUMBER5);
    ADC_enableInterrupt(ADCA_BASE, ADC_INT_NUMBER1);
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);
    EDIS;

    // -------------------------------------------------------------------------
    // 2. ADC-B 초기화 (레퍼런스 모니터링 및 온도 센서)
    // -------------------------------------------------------------------------
    EALLOW;
    ADC_setPrescaler(ADCB_BASE, ADC_CLK_DIV_4_0);
    ADC_setMode(ADCB_BASE, ADC_RESOLUTION_12BIT, ADC_MODE_SINGLE_ENDED);
    ADC_enableConverter(ADCB_BASE);
    DEVICE_DELAY_US(1000U);

    // ADCB SOC 매핑 설정 (SOC 번호 = 채널 번호 일치)
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER1, ADC_TRIGGER_EPWM1_SOCA, ADC_CH_ADCIN1, 14u);  // SOC1: B1 (VSEN_REF)
    // 온도 센서는 신호 안정성을 위해 샘플링 윈도우를 여유있게 확보 (250)
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER3, ADC_TRIGGER_EPWM1_SOCA, ADC_CH_ADCIN3, 250u); // SOC3: B3 (TSEN_BD)
    EDIS;
}

// (Removed EPWM8 and EPWM9 specific setups as they are now unified under EPWM1 10kHz timer)

/*
@funtion    __interrupt void AdcaIsr(void)
@brief      ADCINA1 인터럽트 서비스 루틴 (백그라운드 실시간 초고속 데이터 취득)
@param      void
@return     __interrupt void
*/
__interrupt void AdcaIsr(void)
{
    // 실시간 ADC RAW 데이터 취득
    adcRawData.isenMot = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER2);
    adcRawData.isenBrk = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER3);
    adcRawData.vsen28v = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER4);
    adcRawData.vsen5vd = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER5);
    adcRawData.vsenRef = ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER1);
    adcRawData.tsenBd  = ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER3);

    // 실시간 ADC 데이터 스케일링 및 필터링 호출
    CalcAdcData();

    // 인터럽트 오버플로우(Interrupt Overflow) 감지 시 강제 해제하여 ADC 락업 방어 (CWE-658 방어 규격 준수)
    if (ADC_getInterruptOverflowStatus(ADCA_BASE, ADC_INT_NUMBER1))
    {
        ADC_clearInterruptOverflowStatus(ADCA_BASE, ADC_INT_NUMBER1);
    }

    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}
