/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Adc.h
    Version          : 00.04
    Description      : ADC 데이터 처리 로직 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 23. (main.h -> main_cpu1.h 인클루드 명칭 리팩토링)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 23. - main.h -> main_cpu1.h 인클루드 명칭 리팩토링
 * 2026. 06. 12. - 매크로 상수명 추상화 (ADC_SCALE_REF_VOLT)
 * 2026. 06. 12. - ADC 스케일 팩터 및 필터 상수를 헤더로 통합
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 상태 변수들을 stAdcState 구조체(xAdc)로 통합
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */


#ifndef CSU_ADC_H
#define CSU_ADC_H

/* ************************** [[   include  ]]  *********************************************************** */
#include "main_cpu1.h"

/* ************************** [[   define   ]]  *********************************************************** */

// ADC 기준 변환 상수 (3.0V Reference)
#define ADC_SCALE_REF_VOLT (3.0f / 4096.0f) // 3V 레퍼런스 기준 변환 상수

// LPF 상수 (전압/전류)
#define LPF_OLD_CV 0.3f
#define LPF_REAL_CV 0.7f

// LPF 상수 (온도)
#define LPF_OLD_TEMP 0.9f
#define LPF_REAL_TEMP 0.1f

// ADC 센서 오프셋 영점 기본값 매크로
#define ADC_OFFSET_ISEN_MOT_DEFAULT 1.49702f    // 모터 전류 영점 기본값 (1.5V 근사치)
#define ADC_OFFSET_ISEN_BRK_DEFAULT 1.49702f    // 브레이크 전류 영점 기본값 (1.5V 근사치)

// ADC 데이터 역산 스케일 팩터 매크로
#define ADC_SCALE_ISEN_MOT          16.1550888f // 모터 전류 변환 상수
#define ADC_SCALE_ISEN_BRK          1.6155088f  // 브레이크 전류 변환 상수
#define ADC_SCALE_VSEN_28V          16.86619f   // 28V 전압 변환 상수
#define ADC_SCALE_VSEN_5VD          2.0f        // 5V 전압 변환 상수
#define ADC_SCALE_TSEN_BD_SLOPE     84.033613f  // 온도 센서(MAX6605) 기울기
#define ADC_SCALE_TSEN_BD_OFFSET    55.0f       // 온도 센서(MAX6605) 오프셋


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
