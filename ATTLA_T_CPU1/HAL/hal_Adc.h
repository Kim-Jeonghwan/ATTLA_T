/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : hal_Adc.h
    Version          : 00.01
    Description      : ADC 및 내부 온도 센서 하드웨어 제어 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 09. (RAW 데이터 구조체 추가)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 02. - 온도 센서 전용 1kHz 느린 트리거용 ePWM9 함수 전역 선언 추가
 * 
*/


/* DESCRIPTION
 * 
 * 
*/

#ifndef HAL_ADC_H
#define HAL_ADC_H

/* ************************** [[   include  ]]  *********************************************************** */
#include "main.h"

/* ************************** [[   define   ]]  *********************************************************** */
#define SYSCLK			200E6	// 200MHz, 28X Core(CPU) 시스템 클럭 주파수
#define TBCLK			 10E6	// 10MHz, EPWM 모듈 타임베이스 클럭 주파수
#define SAMPLING_FREQ	 10E3	// 10kHz, ADC 샘플링 주파수


#define BUFF_SIZE		500u     // ADC 결과 저장 버퍼 크기

#define CONV_ADC_3V		0.000732421875f		// 3 / 4096
#define CONV_ADC_3_3V	0.0008056640625f	// 3.3 / 4096



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
// ADC ISR 초기화
void InitialAdc(void);

// ADC 모듈 초기화 함수
void InitAdcModules(void);

// ADCINA1 인터럽트 서비스 루틴 (뼈대)
__interrupt void AdcaIsr(void);

#endif	// #ifndef HAL_ADC_H
