/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Adc.h
    Version          : 00.01
    Description      : ADC 데이터 처리 로직 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 11. (주석 표준화 및 레거시 코드 정리)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 상태 변수들을 stAdcState 구조체(xAdc)로 통합
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */


#ifndef CSU_ADC_H
#define CSU_ADC_H

/* ************************** [[   include  ]]  *********************************************************** */
#include "main.h"

/* ************************** [[   define   ]]  *********************************************************** */


/* ************************** [[   enum or struct   ]]  *************************************************** */


/* ************************** [[   global   ]]  *********************************************************** */
typedef struct {
    float32_t isenMotLpf;
    float32_t isenBrkLpf;
    float32_t vsen28VLpf;
    float32_t vsen5VDLpf;
    float32_t vsenRefLpf;
    float32_t tsenBdLpf;
    float32_t isenMotOffset;
    float32_t isenBrkOffset;
} stAdcState;

extern stAdcState xAdc;

/* ************************** [[  function  ]]  *********************************************************** */
/**
 * @brief      ADC 애플리케이션 초기화
 * @param      void
 * @return     void
 */
void Initial_Adc(void);

/**
 * @brief      내부 온도 센서 실시간 데이터 수집 및 갱신 (10ms 주기)
 * @param      void
 * @return     void
 */
void updateAdcData(void);

/**
 * @brief      10kHz ADC 인터럽트에서 호출되는 실시간 데이터 스케일링 및 필터링
 * @param      void
 * @return     void
 */
void CalcAdcData(void);

#endif /* CSU_ADC_H */
