/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : hal_Adc.h
    Version          : 00.06
    Description      : ADC 및 내부 온도 센서 하드웨어 제어 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 23. (main.h -> main_cpu1.h 인클루드 명칭 리팩토링)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 23. - main.h -> main_cpu1.h 인클루드 명칭 리팩토링
 * 2026. 06. 12. - MAVE 카운트 및 PWM 주파수 매크로 헤더로 이동 (글로벌 룰 적용)
 * 2026. 06. 12. - AdcaIsr 선언 제거
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 09. - RAW 데이터 구조체 추가
 */


/* DESCRIPTION
 * 
 * 
*/

#ifndef HAL_ADC_H
#define HAL_ADC_H

/* ************************** [[   include  ]]  *********************************************************** */
#include "main_cpu1.h"

/* ************************** [[   define   ]]  *********************************************************** */
#define SYSCLK			200E6	// 200MHz, 28X Core(CPU) 시스템 클럭 주파수
#define TBCLK			 10E6	// 10MHz, EPWM 모듈 타임베이스 클럭 주파수
#define SAMPLING_FREQ	 10E3	// 10kHz, ADC 샘플링 주파수


#define BUFF_SIZE		500u     // ADC 결과 저장 버퍼 크기

#define CONV_ADC_3V		0.000732421875f		// 3 / 4096
#define CONV_ADC_3_3V	0.0008056640625f	// 3.3 / 4096

#define DEFAULT_MAVE_COUNT  100u   // 이동 평균 필터 카운트
#define DEFAULT_PWM_HZ      100000u // ePWM8 트리거 주파수 (100kHz 조정)



/* ************************** [[   struct   ]]  *********************************************************** */
typedef struct {
    uint16_t isenMot;
    uint16_t isenBrk;
    uint16_t vsen28v;
    uint16_t vsen5vd;
    uint16_t vsenRef;
    uint16_t tsenBd;
} AdcRawData_t;

/* ************************** [[   global   ]]  *********************************************************** */
extern AdcRawData_t adcRawData;


/* ************************** [[  function  ]]  *********************************************************** */
/**
 * @brief      ADC 초기화 기동 및 인터럽트 등록
 * @param      void
 * @return     void
 */
void InitialAdc(void);

/**
 * @brief      ADC 모듈 초기 설정 (Driverlib 적용)
 * @param      void
 * @return     void
 */
void InitAdcModules(void);

// (AdcaIsr removed)

#endif	// #ifndef HAL_ADC_H
