/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Adc.c
    Version          : 00.03
    Description      : ADC 데이터 필터링 및 10ms 주기 데이터 처리 로직
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 11. (주석 표준화 및 레거시 코드 정리)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 상태 변수들을 stAdcState 구조체(xAdc)로 통합
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 * 2026. 06. 11. - ADC 스케일 팩터 및 오프셋 정밀 교정 (모터/브레이크 전류, 28V 전압)
 */


/* ************************** [[   include  ]]  *********************************************************** */
#include "csu_Adc.h"

/* ************************** [[   global   ]]  *********************************************************** */
// hal_Adc.c에 선언된 실시간 온도 센서 원시 결과 전역 변수 공유 참조
extern uint16_t adcResult;

// 디버깅 및 실시간 표출용 온도 센서 결과 전역 변수 (타입 미스매치 방지를 위해 float32_t로 선언)
float32_t currentTemperatureC = 0.0f;

// --- 신규 ADC 상태 구조체 변수 ---
stAdcState xAdc;


// ADC 기준 변환 상수 (3.0V Reference)
#define SCALE_ADC_3V (3.0f / 4096.0f)

// LPF 상수 (전압/전류)
#define LPF_OLD_CV 0.3f
#define LPF_REAL_CV 0.7f

// LPF 상수 (온도)
#define LPF_OLD_TEMP 0.9f
#define LPF_REAL_TEMP 0.1f

// C2000Ware OTP 캘리브레이션 셋업 전역 변수 참조
extern float32 tempSensor_tempSlope;
extern float32 tempSensor_tempOffset;
extern float32 tempSensor_scaleFactor;

extern void InitTempSensor(float32_t vrefhi_voltage);


/* ************************** [[  static prototype  ]]  *************************************************** */
static void updateDspTempSensor(void);


/* ************************** [[  function  ]]  *********************************************************** */

/*
@function    void Initial_Adc(void)
@brief      ADC 애플리케이션 초기화
@param      void
@return     void
@remark
    - DSP 내부 온도 센서 캘리브레이션을 수행합니다.
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
    xAdc.isenMotOffset = 1.49702f;
    xAdc.isenBrkOffset = 1.49702f;

    // DSP 내부 온도 센서 활성화(캘리브레이션 초기화, 외부 레퍼런스 3.0V 적용 - 스펙 문서 일치)
    InitTempSensor(3.0f);
}


/*
@function    void updateAdcData(void)
@brief      ADC 데이터 업데이트 (10ms 주기 호출)
@param      void
@return     void
@remark
    - 하위 센서 모듈들을 차례로 갱신하며, 현재 내부 온도 센서 데이터를 수집합니다.
*/
void updateAdcData(void)
{
    // DSP 내부 온도 센서 데이터 업데이트
    updateDspTempSensor();
}


/*
@function    static void updateDspTempSensor(void)
@brief      DSP 내부 온도 센서 데이터 측정 및 섭씨 온도 변환
@param      void
@return     static void
@remark
    - ePWM9 이벤트로 수집된 ADC 원시 결과를 섭씨(C)로 변환합니다.
    - 소수점 이하 정밀도 유지를 위해 실수 연산을 직접 적용하며, Divide by Zero 취약점을 방어합니다.
*/
static void updateDspTempSensor(void)
{

    // C2000Ware 내장 원본 소수점 계산 공식 직접 적용 (12비트 기준 최대 4096 분해능 분모 적용)
    // Divide by Zero 취약점 (CWE-369) 방지를 위한 사전 분모 조건 검사 수행
    if (tempSensor_tempSlope != 0.0f)
    {
        float32_t rawTempC = ((((float32_t)adcResult * tempSensor_scaleFactor / 4096.0f) - tempSensor_tempOffset) / tempSensor_tempSlope);
        
        // IIR 로우패스 필터 적용 (노이즈로 인한 1의 자리 및 소수점 자리 요동 방지)
        // Alpha = 0.05 (새로운 값 5%, 기존 값 95% 반영)
        if (currentTemperatureC == 0.0f)
        {
            currentTemperatureC = rawTempC; // 초기화
        }
        else
        {
            currentTemperatureC = (currentTemperatureC * 0.95f) + (rawTempC * 0.05f);
        }
    }
    else
    {
        currentTemperatureC = 0.0f; // 슬로프 값이 비정상일 경우 안전 디폴트 값 할당
    }
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
    V_in = (float32_t)adcRawData.isenMot * SCALE_ADC_3V;
    curr_val = (V_in - xAdc.isenMotOffset) * 16.1550888f;
    xAdc.isenMotLpf = (LPF_OLD_CV * xAdc.isenMotLpf) + (LPF_REAL_CV * curr_val);

    // 2. ISEN_BRK (ADCA SOC3): TMCS1126. 영점 1.49702V 기준 -2.4A~+2.4A 맵핑. 역산 상수 1.6155088f 적용.
    V_in = (float32_t)adcRawData.isenBrk * SCALE_ADC_3V;
    curr_val = (V_in - xAdc.isenBrkOffset) * 1.6155088f;
    xAdc.isenBrkLpf = (LPF_OLD_CV * xAdc.isenBrkLpf) + (LPF_REAL_CV * curr_val);

    // 3. VSEN_28V (ADCA SOC4): 50V 입력 시 약 2.96451V -> 50 / 2.96451 = 16.86619f
    V_in = (float32_t)adcRawData.vsen28v * SCALE_ADC_3V;
    curr_val = V_in * 16.86619f;
    xAdc.vsen28VLpf = (LPF_OLD_CV * xAdc.vsen28VLpf) + (LPF_REAL_CV * curr_val);

    // 4. 5VD (ADCA SOC5): 5V 입력 시 약 2.500V -> 5 / 2.5 = 2.0
    V_in = (float32_t)adcRawData.vsen5vd * SCALE_ADC_3V;
    curr_val = V_in * 2.0f;
    xAdc.vsen5VDLpf = (LPF_OLD_CV * xAdc.vsen5VDLpf) + (LPF_REAL_CV * curr_val);

    // 5. VSEN_REF (ADCB SOC1): 2.048V 측정
    V_in = (float32_t)adcRawData.vsenRef * SCALE_ADC_3V;
    curr_val = V_in;
    xAdc.vsenRefLpf = (LPF_OLD_CV * xAdc.vsenRefLpf) + (LPF_REAL_CV * curr_val);

    // 6. TSEN_BD (ADCB SOC3): MAX6605. -55도 ~ +125도 (Delta 180도) -> 0V ~ 2.142V (Delta 2.142V)
    // T = (V_in / 2.142) * 180 - 55 = V_in * 84.033613f - 55
    V_in = (float32_t)adcRawData.tsenBd * SCALE_ADC_3V;
    curr_val = (V_in * 84.033613f) - 55.0f;
    xAdc.tsenBdLpf = (LPF_OLD_TEMP * xAdc.tsenBdLpf) + (LPF_REAL_TEMP * curr_val);
}

