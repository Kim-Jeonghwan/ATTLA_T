/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Control.c
    Version          : 00.05
    Description      : 시스템 제어 모듈 (동적 인터럽트 스위칭 및 ADC 폴링) 구현
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 12. (이산신호 갱신 및 FRAM 저장 연동)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 12. - 이산신호(DIO) 디바운싱 갱신 및 FRAM 저장 로직(saveData) 연동
 * 2026. 06. 12. - 3단계 동적 인터럽트 전환 적용 (csu_Offset_Isr, Pbit, MainControl)
 * 2026. 06. 11. - CBIT(Bit_RunCBIT)에 스톨, 과속, 엔코더 신규 점검 함수 추가
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 상태 변수들을 stControlState 구조체(xSysCtrl)로 통합
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 * 2026. 06. 11. - 함수명 접두어(csu_, hal_) 제거 리팩토링
 */


#include "csu_Control.h"
volatile stControlState xSysCtrl;

#pragma CODE_SECTION(csu_Offset_Isr, ".TI.ramfunc");
#pragma CODE_SECTION(csu_Pbit_Isr, ".TI.ramfunc");
#pragma CODE_SECTION(csu_MainControl_Isr, ".TI.ramfunc");

// 100us 주기 기준, 10000회 누적 시 1초 대기
static uint16_t offsetCount = 0U;
static float32_t sumMot = 0.0f;
static float32_t sumBrk = 0.0f;
#define SCALE_ADC_3V (3.0f / 4096.0f)

/**
 * @function Control_Init
 * @brief    시스템 제어 모듈 상태 구조체 초기화
 * @param    void
 * @return   void
 */
void Control_Init(void)
{
    // 구조체 명시적 초기화
    Dio_Init();
    xSysCtrl.isOffsetCalibrated = 0U;
    xSysCtrl.isPbitComplete = 0U;
}

/*
@function    void Control_SaveOffsetToFram(void)
@brief      초기 1초 오프셋 보정 완료 후 FRAM에 오프셋 값 저장
@param      void
@return     void
*/
void Control_SaveOffsetToFram(void)
{
    // 오프셋 값을 밀리암페어(mA) 단위 정수형으로 변환 후 FRAM(0x0000)에 저장
    uint16_t motOffset_mA = (uint16_t)(xAdc.isenMotOffset * 1000.0f);
    uint16_t brkOffset_mA = (uint16_t)(xAdc.isenBrkOffset * 1000.0f);

    Fram_WriteByte(0x0000U, (motOffset_mA >> 8U) & 0xFFU);
    Fram_WriteByte(0x0001U, motOffset_mA & 0xFFU);
    
    Fram_WriteByte(0x0002U, (brkOffset_mA >> 8U) & 0xFFU);
    Fram_WriteByte(0x0003U, brkOffset_mA & 0xFFU);
}

/*
@function    void Control_SaveDataToFram(void)
@brief      시스템 운영 중 특정 조건에 맞춰 데이터 저장 래퍼 함수
@param      void
@return     void
*/
void Control_SaveDataToFram(void)
{
    // 추후 데이터 저장 조건(주기적 로깅 또는 고장 발생 시 에러 코드 기록) 구현 예정
}

// 기존의 PWM 인터럽트 내에서 처리하던 오프셋 로직은 제거되고 csu_Offset_Isr 로 이동됨

/*
@function    void Bit_RunPBIT(void)
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
@function    void Bit_RunCBIT(void)
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
    Bit_Encoder_Check();
    Bit_MotorStall_Check();
    Bit_MotorOverSpeed_Check();
}

void Control_SystemOperation(void)
{
    // 인터럽트 체인에 의해 오프셋 및 PBIT는 모두 통과된 상태로 진입함

    // 1. 아날로그신호 입력 및 연산 CSU
    // CalcAdcData(); // (참고용. ADC 자체 인터럽트로 분리될 경우 호출 안 함)
    
    // 2. 이산신호 입력 CSU
    Dio_UpdateInput();

    // 3. 위치(각도) 정보 획득 및 처리 CSU
    Encoder_UpdatePosition();

    // 4. 모터 드라이버 상태 정보 획득 및 처리 CSU
    // updateMotorDriverStatus();

    // 5. 주기점검(CBIT) CSU
    Bit_RunCBIT();

    // 6. 모터 구동제어 CSU
    MotorCtrl_Run();

    // 7. 데이터 저장 CSU
    Control_SaveDataToFram();
}


// ==============================================================================
// 동적 인터럽트 스위칭 기반 ISR 체인
// ==============================================================================

/*
@function    __interrupt void csu_Offset_Isr(void)
@brief      최초 1초 대기 및 10,000회 전류 센서 오프셋 측정 인터럽트
@param      void
@return     __interrupt void
*/
__interrupt void csu_Offset_Isr(void)
{
    // ADC 변환 완료 대기 (폴링 블로킹)
    while(ADC_getInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1) == false)
    {
    }
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);

    // ADC 실시간 결과 취득 및 스케일링
    adcRawData.isenMot = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER2);
    adcRawData.isenBrk = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER3);
    adcRawData.vsen28v = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER4);
    adcRawData.vsen5vd = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER5);
    adcRawData.vsenRef = ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER1);
    adcRawData.tsenBd  = ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER3);

    // 오프셋 누적 (1.5V 기준)
    if (offsetCount < 10000U)
    {
        sumMot += (float32_t)adcRawData.isenMot * SCALE_ADC_3V;
        sumBrk += (float32_t)adcRawData.isenBrk * SCALE_ADC_3V;
        offsetCount++;
    }
    else
    {
        // 1초 도달 시 평균 적용
        xAdc.isenMotOffset = sumMot / 10000.0f;
        xAdc.isenBrkOffset = sumBrk / 10000.0f;
        xSysCtrl.isOffsetCalibrated = 1U;

        // PBIT 인터럽트로 스위칭
        EALLOW;
        PieVectTable.EPWM1_INT = &csu_Pbit_Isr;
        EDIS;

        // FRAM에 오프셋 저장 래퍼 함수 호출 (인터럽트 교체 후 지연 처리)
        Control_SaveOffsetToFram();
    }

    EPWM_clearEventTriggerInterruptFlag(EPWM_TIMER1_BASE);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP3);
}

/*
@function    __interrupt void csu_Pbit_Isr(void)
@brief      초기 점검 수행 및 메인 제어루프 전환 인터럽트
@param      void
@return     __interrupt void
*/
__interrupt void csu_Pbit_Isr(void)
{
    // ADC 폴링 대기
    while(ADC_getInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1) == false)
    {
    }
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);

    adcRawData.isenMot = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER2);
    adcRawData.isenBrk = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER3);
    adcRawData.vsen28v = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER4);
    adcRawData.vsen5vd = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER5);
    adcRawData.vsenRef = ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER1);
    adcRawData.tsenBd  = ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER3);

    CalcAdcData();
    Bit_RunPBIT();

    // 1회 확인 시 바로 메인 제어로 스위칭 (대기시간 없음)
    if (xSysCtrl.isPbitComplete == 1U)
    {
        EALLOW;
        PieVectTable.EPWM1_INT = &csu_MainControl_Isr;
        EDIS;
    }

    EPWM_clearEventTriggerInterruptFlag(EPWM_TIMER1_BASE);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP3);
}

/*
@function    __interrupt void csu_MainControl_Isr(void)
@brief      시스템 메인 파이프라인 인터럽트
@param      void
@return     __interrupt void
*/
__interrupt void csu_MainControl_Isr(void)
{
    // ADC 폴링 대기
    while(ADC_getInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1) == false)
    {
    }
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);

    adcRawData.isenMot = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER2);
    adcRawData.isenBrk = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER3);
    adcRawData.vsen28v = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER4);
    adcRawData.vsen5vd = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER5);
    adcRawData.vsenRef = ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER1);
    adcRawData.tsenBd  = ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER3);

    CalcAdcData();

    // 100us 시스템 운용 파이프라인 실행
    Control_SystemOperation();

    EPWM_clearEventTriggerInterruptFlag(EPWM_TIMER1_BASE);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP3);
}
