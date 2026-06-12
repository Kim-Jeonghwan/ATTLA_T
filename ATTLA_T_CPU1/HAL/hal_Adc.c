/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : hal_Adc.c
    Version          : 00.07
    Description      : ADC 하드웨어 제어 (폴링 방식으로 인터럽트 활성화 제거)
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 12. (샘플링 윈도우 매직넘버 주석 보강)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 12. - SOC 설정 시 하드코딩된 샘플링 윈도우 크기(14u, 250u)에 대한 직관적인 주석 보강
 * 2026. 06. 12. - 매크로 상수를 헤더(.h)로 이동 (글로벌 룰 적용)
 * 2026. 06. 12. - 내부 온도 센서 미사용에 따른 전역 변수(adcResult) 제거
 * 2026. 06. 12. - ADC 인터럽트 비활성화 및 AdcaIsr 제거 (EPWM1 폴링으로 전환)
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 * 2026. 06. 11. - 함수명 접두어(csu_, hal_) 제거 리팩토링
 */


/* ************************** [[   include  ]] *********************************************************** */
#include "hal_Adc.h"



// EPWM1 트리거 사용하므로 별도의 ADC 전용 EPWM8/9 초기화 불필요

/* ************************** [[   define   ]] *********************************************************** */

/* ************************** [[   global   ]] *********************************************************** */
AdcRawData_t adcRawData; // 실시간 ADC 채널 RAW 데이터 버퍼

/* ************************** [[  function  ]] *********************************************************** */

/*
@function    void InitialAdc(void)
@brief      ADC 초기화 기동 및 인터럽트 등록
@param      void
@return     void
*/
void InitialAdc(void)
{
    InitAdcModules(); // ADC 모듈 하드웨어 초기화

    // 기존 ADCA1 인터럽트 활성화 코드는 삭제됨 (EPWM1 인터럽트 내에서 폴링 방식으로 대기)
}

/*
@function    void InitAdcModules(void)
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
    // 14u: S/H(Sample and Hold) 일반 센서용 ACQPS(Acquisition Window) 클럭 수
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER2, ADC_TRIGGER_EPWM1_SOCA, ADC_CH_ADCIN2, 14u); // SOC2: A2 (ISEN_MOT)
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER3, ADC_TRIGGER_EPWM1_SOCA, ADC_CH_ADCIN3, 14u); // SOC3: A3 (ISEN_BRK)
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER4, ADC_TRIGGER_EPWM1_SOCA, ADC_CH_ADCIN4, 14u); // SOC4: A4 (VSEN_28V)
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER5, ADC_TRIGGER_EPWM1_SOCA, ADC_CH_ADCIN5, 14u); // SOC5: A5 (5VD)

    // ADCA의 마지막 변환 완료 시점(SOC5)에서 인터럽트 INT1 발생
    ADC_setInterruptSource(ADCA_BASE, ADC_INT_NUMBER1, ADC_SOC_NUMBER5);
    ADC_disableInterrupt(ADCA_BASE, ADC_INT_NUMBER1); // PIE로의 인터럽트 발생 차단 (폴링용 플래그만 세팅됨)
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
    // 14u: S/H(Sample and Hold) 일반 센서용 ACQPS(Acquisition Window) 클럭 수
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER1, ADC_TRIGGER_EPWM1_SOCA, ADC_CH_ADCIN1, 14u);  // SOC1: B1 (VSEN_REF)
    
    // 250u: 온도 센서 등 신호 안정화를 위해 S/H 윈도우를 여유있게 확보 (긴 샘플링 윈도우)
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER3, ADC_TRIGGER_EPWM1_SOCA, ADC_CH_ADCIN3, 250u); // SOC3: B3 (TSEN_BD)
    EDIS;
}

// AdcaIsr 제거됨 (csu_Control.c 의 EPWM1 ISR 내 폴링으로 대체)
