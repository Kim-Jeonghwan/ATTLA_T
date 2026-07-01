/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : hal_Adc.c
    Version          : 00.08
    Description      : ADC 하드웨어 제어 (폴링 방식으로 인터럽트 활성화 제거)
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 07. 01. (초기화 구문 상세 한글 주석 추가)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 07. 01. - 초기화 구문 상세 한글 주석 추가 (코딩 규칙 적용)
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
    ADC_setPrescaler(ADCA_BASE, ADC_CLK_DIV_4_0); // ADCCLK 분주비 4로 설정 (ADCCLK = SYSCLK / 4 = 50MHz 속도 구동)
    ADC_setMode(ADCA_BASE, ADC_RESOLUTION_12BIT, ADC_MODE_SINGLE_ENDED); // 12비트 분해능(0~4095) 및 단일 종단 입력 모드 설정
    ADC_setInterruptPulseMode(ADCA_BASE, ADC_PULSE_END_OF_CONV); // 인터럽트 펄스 발생 시점을 변환 완료(EOC) 시로 설정
    ADC_enableConverter(ADCA_BASE); // ADC-A 컨버터 하드웨어 활성화
    DEVICE_DELAY_US(1000U); // 아날로그 회로 파워업 및 안정화 대기 (1ms)

    // ADCA SOC 매핑 설정 (SOC 번호 = 채널 번호 일치)
    // 트리거: EPWM1_SOCA (10kHz 주기적으로 변환 시작)
    // 14u: S/H(Sample and Hold) 일반 센서용 ACQPS(Acquisition Window) 클럭 수 (최소 샘플링 시간 확보)
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER2, ADC_TRIGGER_EPWM1_SOCA, ADC_CH_ADCIN2, 14u); // SOC2: A2 채널 매핑, 모터 구동 전류(ISEN_MOT) 측정용
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER3, ADC_TRIGGER_EPWM1_SOCA, ADC_CH_ADCIN3, 14u); // SOC3: A3 채널 매핑, 브레이크 구동 전류(ISEN_BRK) 측정용
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER4, ADC_TRIGGER_EPWM1_SOCA, ADC_CH_ADCIN4, 14u); // SOC4: A4 채널 매핑, 메인 28V 전압(VSEN_28V) 측정용
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER5, ADC_TRIGGER_EPWM1_SOCA, ADC_CH_ADCIN5, 14u); // SOC5: A5 채널 매핑, 로직 5V 전압(5VD) 측정용

    // ADCA의 마지막 변환 완료 시점(SOC5 EOC)에서 인터럽트 INT1 발생 플래그 세팅 설정
    ADC_setInterruptSource(ADCA_BASE, ADC_INT_NUMBER1, ADC_SOC_NUMBER5);
    ADC_disableInterrupt(ADCA_BASE, ADC_INT_NUMBER1); // PIE 코어로의 실제 인터럽트 진입은 차단 (EPWM ISR에서의 폴링 확인용으로만 활용)
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1); // 초기화 시점에 남아있을 수 있는 쓰레기 인터럽트 플래그 강제 클리어
    EDIS;

    // -------------------------------------------------------------------------
    // 2. ADC-B 초기화 (레퍼런스 모니터링 및 온도 센서)
    // -------------------------------------------------------------------------
    EALLOW;
    ADC_setPrescaler(ADCB_BASE, ADC_CLK_DIV_4_0); // ADCCLK 분주비 4로 설정 (ADCCLK = SYSCLK / 4 = 50MHz 속도 구동)
    ADC_setMode(ADCB_BASE, ADC_RESOLUTION_12BIT, ADC_MODE_SINGLE_ENDED); // 12비트 분해능(0~4095) 및 단일 종단 입력 모드 설정
    ADC_enableConverter(ADCB_BASE); // ADC-B 컨버터 하드웨어 활성화
    DEVICE_DELAY_US(1000U); // 아날로그 회로 파워업 및 안정화 대기 (1ms)

    // ADCB SOC 매핑 설정 (SOC 번호 = 채널 번호 일치)
    // 14u: S/H(Sample and Hold) 일반 센서용 ACQPS(Acquisition Window) 클럭 수
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER1, ADC_TRIGGER_EPWM1_SOCA, ADC_CH_ADCIN1, 14u);  // SOC1: B1 채널 매핑, 레퍼런스 모니터링 전압(VSEN_REF) 측정용
    
    // 250u: NTC 온도 센서와 같이 높은 임피던스를 갖거나 신호 변화가 느린 경우 S/H 윈도우를 최대한 여유 있게(최대 255) 할당하여 신호 무결성 확보
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER3, ADC_TRIGGER_EPWM1_SOCA, ADC_CH_ADCIN3, 250u); // SOC3: B3 채널 매핑, 기판 NTC 써미스터(TSEN_BD) 측정용
    EDIS;
}

// AdcaIsr 제거됨 (csu_Control.c 의 EPWM1 ISR 내 폴링으로 대체)
