/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Control.c
    Version          : 00.02
    Description      : 시스템 제어 모듈 (PBIT, CBIT, 시스템 운용 파이프라인) 구현
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 11. (전역 변수 구조체화 마이그레이션)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 11. - 상태 변수들을 stControlState 구조체(xSysCtrl)로 통합
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 * 2026. 06. 11. - 함수명 접두어(csu_, hal_) 제거 리팩토링
 */


#include "csu_Control.h"
volatile stControlState xSysCtrl;

// 100us 주기 기준, 10000회 누적 시 1초 대기
static uint16_t offsetCount = 0U;
static float32_t sumMot = 0.0f;
static float32_t sumBrk = 0.0f;
#define SCALE_ADC_3V (3.0f / 4096.0f)

void Control_Init(void)
{
    // 구조체 명시적 초기화
    xSysCtrl.isOffsetCalibrated = 0U;
    xSysCtrl.isPbitComplete = 0U;
}

/*
@funtion    void Control_CalibrateCurrentOffset(void)
@brief      전류 센서 오프셋 영점 조정 (PWM ISR 호출용)
@param      void
@return     void
*/
void Control_CalibrateCurrentOffset(void)
{
    if (offsetCount < 10000U)
    {
        sumMot += (float32_t)adcRawData.isenMot * SCALE_ADC_3V;
        sumBrk += (float32_t)adcRawData.isenBrk * SCALE_ADC_3V;
        offsetCount++;
    }
    else
        xAdc.isenMotOffset = sumMot / 10000.0f;
        xAdc.isenBrkOffset = sumBrk / 10000.0f;
        
        // TODO: FRAM에 오프셋 값 저장
        
        xSysCtrl.isOffsetCalibrated = 1U;
    }
}

/*
@funtion    void Bit_RunPBIT(void)
@brief      초기 점검 (PBIT) 수행 (PWM ISR 호출용)
@param      void
@return     void
*/
void Bit_RunPBIT(void)
{
    // 초기에는 과전류 대신 전압, 온도, 게이트 펄트 확인
    Bit_OvVoltage_Check();
    Bit_OvTemperature_Check();
    Bit_GateFault_Check();

    // 치명적 결함이 없으면 초기화 완료
    if (xBit.faultFlagSet == 0U)
    {
        xSysCtrl.isPbitComplete = 1U;
    }
}

/*
@funtion    void Bit_RunCBIT(void)
@brief      주기 점검 (CBIT) 수행
@param      void
@return     void
*/
void Bit_RunCBIT(void)
{
    Bit_OvVoltage_Check();
    Bit_OvCurrent_Check();
    Bit_OvTemperature_Check();
    Bit_GateFault_Check();
    // 엔코더 체크? - 에러, 워닝?
    // 내부 전원 점검?
    // 모터 stall 보호? - 기준 위치 필요, 비정상일때 무엇?
    // 모터 과속 보호? - 기준 속도 필요, 비정상일때 무엇?
}

/*
@funtion    void Control_SystemOperation(void)
@brief      100us PWM 인터럽트 기반 시스템 운용 파이프라인
@param      void
@return     void
*/
void Control_SystemOperation(void)
{
    if (xSysCtrl.isOffsetCalibrated == 0U)
    {
        Control_CalibrateCurrentOffset();
        return; // 오프셋 완료 전까지 운용 로직 대기
    }
    
    if (xSysCtrl.isPbitComplete == 0U)
    {
        Bit_RunPBIT();
        return; // PBIT 완료 전까지 운용 로직 대기
    }

    // 1. 아날로그신호 입력 및 연산 CSU
    // CalcAdcData(); // (참고용. ADC 자체 인터럽트로 분리될 경우 호출 안 함)
    
    // 2. 이산신호 입력 CSU
    // updateDioInput();

    // 3. 위치(각도) 정보 획득 및 처리 CSU
    Encoder_UpdatePosition();

    // 4. 모터 드라이버 상태 정보 획득 및 처리 CSU
    // updateMotorDriverStatus();

    // 5. 주기점검(CBIT) CSU
    Bit_RunCBIT();

    // 6. 모터 구동제어 CSU
    MotorCtrl_Run();

    // 7. 데이터 저장 CSU
    // saveData();
}
