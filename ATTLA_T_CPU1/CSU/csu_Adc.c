/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Adc.c
    Version          : 00.06
    Description      : ADC 데이터 필터링 및 10ms 주기 데이터 처리 로직
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 12. (매크로 상수명 추상화: ADC_SCALE_REF_VOLT 반영)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 12. - 매크로 상수명 추상화 (ADC_SCALE_REF_VOLT) 반영
 * 2026. 06. 12. - ADC 스케일 팩터 및 필터 상수를 헤더 파일로 이동 (매직넘버 제거)
 * 2026. 06. 12. - 내부 온도 센서 미사용에 따른 관련 변수 및 로직 제거
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 상태 변수들을 stAdcState 구조체(xAdc)로 통합
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 * 2026. 06. 11. - ADC 스케일 팩터 및 오프셋 정밀 교정 (모터/브레이크 전류, 28V 전압)
 */


/* ************************** [[   include  ]]  *********************************************************** */
#include "csu_Adc.h"

/* ************************** [[   global   ]]  *********************************************************** */
// --- 신규 ADC 상태 구조체 변수 ---
stAdcState xAdc;

/* ************************** [[  static prototype  ]]  *************************************************** */


/* ************************** [[  function  ]]  *********************************************************** */

/*
@function    void Initial_Adc(void)
@brief      ADC 애플리케이션 초기화
@param      void
@return     void
@remark
    - ADC 상태 변수들을 명시적으로 초기화합니다.
*/
void Initial_Adc(void)
{
    // 구조체 명시적 초기화 (CWE-457 방지 및 시스템 리셋 시 클리어 목적)
    xAdc.isenMotLpf = 0.0f;
    xAdc.isenBrkLpf = 0.0f;
    xAdc.vsen28VLpf = 0.0f;
    xAdc.vsen5VDLpf = 0.0f;
    xAdc.vsenRefLpf = 0.0f;
    xAdc.tsenBdLpf = 0.0f;
    xAdc.isenMotOffset = ADC_OFFSET_ISEN_MOT_DEFAULT;
    xAdc.isenBrkOffset = ADC_OFFSET_ISEN_BRK_DEFAULT;
}


/*
@function    void updateAdcData(void)
@brief      ADC 데이터 업데이트 (10ms 주기 호출)
@param      void
@return     void
@remark
    - 하위 센서 모듈들을 차례로 갱신합니다.
*/
void updateAdcData(void)
{
    // 현재는 주기적으로 처리할 ADC 소프트웨어 갱신 내용 없음
}

/*
@function    void CalcAdcData(void)
@brief      10kHz ADC 인터럽트 루틴 내 데이터 갱신 및 필터링
@param      void
@return     void
@remark
    - 하드웨어에서 취득한 원시 결과(12비트)를 물리량으로 선형 스케일링
    - EMA(Exponential Moving Average) LPF 필터 적용
*/
void CalcAdcData(void)
{
    float32_t V_in;
    float32_t curr_val;

// 1. ISEN_MOT (ADCA SOC2): TMCS1126 (100mV/A). 영점 1.49702V 기준 -24A~+24A 맵핑. 역산 상수 16.1550888f 적용.
    V_in = (float32_t)adcRawData.isenMot * ADC_SCALE_REF_VOLT;
    curr_val = (V_in - xAdc.isenMotOffset) * ADC_SCALE_ISEN_MOT;
    xAdc.isenMotLpf = (LPF_OLD_CV * xAdc.isenMotLpf) + (LPF_REAL_CV * curr_val);

    // 2. ISEN_BRK (ADCA SOC3): TMCS1126. 영점 1.49702V 기준 -2.4A~+2.4A 맵핑. 역산 상수 1.6155088f 적용.
    V_in = (float32_t)adcRawData.isenBrk * ADC_SCALE_REF_VOLT;
    curr_val = (V_in - xAdc.isenBrkOffset) * ADC_SCALE_ISEN_BRK;
    xAdc.isenBrkLpf = (LPF_OLD_CV * xAdc.isenBrkLpf) + (LPF_REAL_CV * curr_val);

    // 3. VSEN_28V (ADCA SOC4): 50V 입력 시 약 2.96451V -> 50 / 2.96451 = 16.86619f
    V_in = (float32_t)adcRawData.vsen28v * ADC_SCALE_REF_VOLT;
    curr_val = V_in * ADC_SCALE_VSEN_28V;
    xAdc.vsen28VLpf = (LPF_OLD_CV * xAdc.vsen28VLpf) + (LPF_REAL_CV * curr_val);

    // 4. 5VD (ADCA SOC5): 5V 입력 시 약 2.500V -> 5 / 2.5 = 2.0
    V_in = (float32_t)adcRawData.vsen5vd * ADC_SCALE_REF_VOLT;
    curr_val = V_in * ADC_SCALE_VSEN_5VD;
    xAdc.vsen5VDLpf = (LPF_OLD_CV * xAdc.vsen5VDLpf) + (LPF_REAL_CV * curr_val);

    // 5. VSEN_REF (ADCB SOC1): 2.048V 측정
    V_in = (float32_t)adcRawData.vsenRef * ADC_SCALE_REF_VOLT;
    curr_val = V_in;
    xAdc.vsenRefLpf = (LPF_OLD_CV * xAdc.vsenRefLpf) + (LPF_REAL_CV * curr_val);

    // 6. TSEN_BD (ADCB SOC3): MAX6605. -55도 ~ +125도 (Delta 180도) -> 0V ~ 2.142V (Delta 2.142V)
    // T = (V_in / 2.142) * 180 - 55 = V_in * 84.033613f - 55
    V_in = (float32_t)adcRawData.tsenBd * ADC_SCALE_REF_VOLT;
    curr_val = (V_in * ADC_SCALE_TSEN_BD_SLOPE) - ADC_SCALE_TSEN_BD_OFFSET;
    xAdc.tsenBdLpf = (LPF_OLD_TEMP * xAdc.tsenBdLpf) + (LPF_REAL_TEMP * curr_val);
}

