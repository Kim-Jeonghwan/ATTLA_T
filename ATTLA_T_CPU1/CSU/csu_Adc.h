/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Adc.h
    Version          : 00.00
    Description      : ADC 데이터 처리 로직 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 08. (주석 템플릿 일괄 적용)
**********************************************************************/

#ifndef CSU_ADC_H
#define CSU_ADC_H

/* ************************** [[   include  ]]  *********************************************************** */
#include "main.h"

/* ************************** [[   define   ]]  *********************************************************** */


/* ************************** [[   enum or struct   ]]  *************************************************** */


/* ************************** [[   global   ]]  *********************************************************** */
extern float32_t Isen_Mot_lpf;
extern float32_t Isen_Brk_lpf;
extern float32_t Vsen_28V_lpf;
extern float32_t Vsen_5VD_lpf;
extern float32_t Vsen_Ref_lpf;
extern float32_t Tsen_Bd_lpf;

/* ************************** [[  function  ]]  *********************************************************** */
/**
 * @brief ADC 애플리케이션 초기화
 */
void Initial_Adc(void);

/**
 * @brief ADC 데이터 업데이트 (뼈대) - 더 이상 사용하지 않음
 */
void updateAdcData(void);

/**
 * @brief 10kHz ADC 인터럽트에서 호출되는 실시간 데이터 스케일링 및 필터링
 */
void csu_CalcAdcData(void);

#endif /* CSU_ADC_H */
