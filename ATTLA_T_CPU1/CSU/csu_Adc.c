/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Adc.c
    Version          : 00.01
    Description      : ADC 데이터 필터링 및 10ms 주기 데이터 처리 로직
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 09. (하드웨어 종속성 분리)
**********************************************************************/

/* ************************** [[   include  ]]  *********************************************************** */
#include "csu_Adc.h"

/* ************************** [[   global   ]]  *********************************************************** */
// hal_Adc.c에 선언된 실시간 온도 센서 원시 결과 전역 변수 공유 참조
extern uint16_t adcResult;

// 디버깅 및 실시간 표출용 온도 센서 결과 전역 변수 (타입 미스매치 방지를 위해 float32_t로 선언)
float32_t currentTemperatureC = 0.0f;

// --- 신규 ADC 데이터 로우패스 필터 변수 ---
float32_t Isen_Mot_lpf = 0.0f;
float32_t Isen_Brk_lpf = 0.0f;
float32_t Vsen_28V_lpf = 0.0f;
float32_t Vsen_5VD_lpf = 0.0f;
float32_t Vsen_Ref_lpf = 0.0f;
float32_t Tsen_Bd_lpf = 0.0f;

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
@funtion    void Initial_Adc(void)
@brief      ADC 애플리케이션 초기화
@param      void
@return     void
@remark
    - DSP 내부 온도 센서 캘리브레이션을 수행합니다.
*/
void Initial_Adc(void)
{
    // DSP 내부 온도 센서 활성화(캘리브레이션 초기화, 외부 레퍼런스 3.3V 적용)
    InitTempSensor(3.3f);
}


/*
@funtion    void updateAdcData(void)
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
@funtion    static void updateDspTempSensor(void)
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
@funtion    void CalcAdcData(void)
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

    // 1. ISEN_MOT (ADCA SOC2)
    V_in = (float32_t)adcRawData.isenMot * SCALE_ADC_3V;
    curr_val = (V_in - 1.49702f) * 16.155f;
    Isen_Mot_lpf = (LPF_OLD_CV * Isen_Mot_lpf) + (LPF_REAL_CV * curr_val);

    // 2. ISEN_BRK (ADCA SOC3)
    V_in = (float32_t)adcRawData.isenBrk * SCALE_ADC_3V;
    curr_val = (V_in - 1.49702f) * 1.6155f;
    Isen_Brk_lpf = (LPF_OLD_CV * Isen_Brk_lpf) + (LPF_REAL_CV * curr_val);

    // 3. VSEN_28V (ADCA SOC4)
    V_in = (float32_t)adcRawData.vsen28v * SCALE_ADC_3V;
    curr_val = V_in * 16.866f;
    Vsen_28V_lpf = (LPF_OLD_CV * Vsen_28V_lpf) + (LPF_REAL_CV * curr_val);

    // 4. 5VD (ADCA SOC5)
    V_in = (float32_t)adcRawData.vsen5vd * SCALE_ADC_3V;
    curr_val = V_in * 2.0f;
    Vsen_5VD_lpf = (LPF_OLD_CV * Vsen_5VD_lpf) + (LPF_REAL_CV * curr_val);

    // 5. VSEN_REF (ADCB SOC1)
    V_in = (float32_t)adcRawData.vsenRef * SCALE_ADC_3V;
    curr_val = V_in * 2.0f;
    Vsen_Ref_lpf = (LPF_OLD_CV * Vsen_Ref_lpf) + (LPF_REAL_CV * curr_val);

    // 6. TSEN_BD (ADCB SOC3)
    V_in = (float32_t)adcRawData.tsenBd * SCALE_ADC_3V;
    curr_val = (V_in * 84.0336f) - 55.0f;
    Tsen_Bd_lpf = (LPF_OLD_TEMP * Tsen_Bd_lpf) + (LPF_REAL_TEMP * curr_val);
}
