/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Control.c
    Version          : 00.10
    Description      : 시스템 제어 모듈 (동적 인터럽트 스위칭 및 ADC 폴링) 구현
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 22. (리미트 스위치 모듈 초기화 호출 추가)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 22. - 리미트 스위치 모듈 초기화(LimitSwitch_Init) 호출 추가
 * 2026. 06. 17. - 명명 규칙 위반 리팩토링 및 헤더 인클루드 수정
 * 2026. 06. 16. - 100us 핵심 제어 루프(ISR) 실행 순서 가이드에 맞게 재배치 (CBIT/FRAM 유지)
 * 2026. 06. 15. - ADC 인터럽트 폴링 무한루프 방지용 타임아웃 적용 (하드웨어 미연결 보드 대응)
 */


#include "csu_Control.h"
volatile stControlState xSysCtrl;

#pragma CODE_SECTION(Offset_Isr, ".TI.ramfunc");
#pragma CODE_SECTION(Pbit_Isr, ".TI.ramfunc");
#pragma CODE_SECTION(MainControl_Isr, ".TI.ramfunc");

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
    LimitSwitch_Init();
    xSysCtrl.isOffsetCalibrated = 0U;
    xSysCtrl.isPbitComplete = 0U;
    xSysCtrl.offsetCount = 0U;
    xSysCtrl.sumMot = 0.0f;
    xSysCtrl.sumBrk = 0.0f;
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

// 기존의 PWM 인터럽트 내에서 처리하던 오프셋 로직은 제거되고 Offset_Isr 로 이동됨

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

/*
@function    void Control_SystemOperation(void)
@brief      시스템 운용 CSU (전체 로직 판단 및 제어권 관리)
@param      void
@return     void
*/
void Control_SystemOperation(void)
{
    // 인터럽트 체인에 의해 오프셋 및 PBIT는 모두 통과된 상태로 진입함

    // 1. 아날로그신호 입력 및 연산 CSU
    CalcAdcData();
    
    // 2. 이산신호 입력 CSU
    Dio_UpdateInput();

    // 3. 위치(각도) 정보 획득 및 처리 CSU
    Encoder_UpdatePosition();

    // 4. 모터 드라이버 상태 정보 획득 및 처리 CSU
    MotorDriver_UpdateStatus();

    // 5. 주기점검(CBIT) CSU (모터 제어 이전 수행)
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
@function    __interrupt void Offset_Isr(void)
@brief      최초 1초 대기 및 10,000회 전류 센서 오프셋 측정 인터럽트
@param      void
@return     __interrupt void
*/
__interrupt void Offset_Isr(void)
{
    // ADC 변환 완료 대기 (폴링 블로킹 타임아웃 적용)
    uint16_t adcTimeout = 100U;
    while((ADC_getInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1) == false) && (--adcTimeout > 0U))
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
    if (xSysCtrl.offsetCount < 10000U)
    {
        xSysCtrl.sumMot += (float32_t)adcRawData.isenMot * ADC_SCALE_REF_VOLT;
        xSysCtrl.sumBrk += (float32_t)adcRawData.isenBrk * ADC_SCALE_REF_VOLT;
        xSysCtrl.offsetCount++;
    }
    else
    {
        // 1초 도달 시 평균 적용
        xAdc.isenMotOffset = xSysCtrl.sumMot / 10000.0f;
        xAdc.isenBrkOffset = xSysCtrl.sumBrk / 10000.0f;
        xSysCtrl.isOffsetCalibrated = 1U;

        // PBIT 인터럽트로 스위칭
        EALLOW;
        PieVectTable.EPWM1_INT = &Pbit_Isr;
        EDIS;

        // FRAM에 오프셋 저장 래퍼 함수 호출 (인터럽트 교체 후 지연 처리)
        Control_SaveOffsetToFram();
    }

    EPWM_clearEventTriggerInterruptFlag(EPWM_TIMER1_BASE);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP3);
}

/*
@function    __interrupt void Pbit_Isr(void)
@brief      초기 점검 수행 및 메인 제어루프 전환 인터럽트
@param      void
@return     __interrupt void
*/
__interrupt void Pbit_Isr(void)
{
    // ADC 폴링 대기 (타임아웃 적용)
    uint16_t adcTimeout = 100U;
    while((ADC_getInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1) == false) && (--adcTimeout > 0U))
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

    // 1회 확인 후 PBIT을 통과했다면,
    // 이더넷 초기화가 완료될 때까지 반복 실행되지 않도록 EPWM 인터럽트를 일시 중지하고 벡터를 메인 제어로 교체함.
    if (xSysCtrl.isPbitComplete == 1U)
    {
        Interrupt_disable(INT_EPWM1);
        
        EALLOW;
        PieVectTable.EPWM1_INT = &MainControl_Isr;
        EDIS;
    }

    EPWM_clearEventTriggerInterruptFlag(EPWM_TIMER1_BASE);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP3);
}

/*
@function    __interrupt void MainControl_Isr(void)
@brief      시스템 메인 파이프라인 인터럽트
@param      void
@return     __interrupt void
*/
__interrupt void MainControl_Isr(void)
{
    // ADC 폴링 대기 (타임아웃 적용 - 메인루프 기아 방지를 위해 100U로 대폭 축소)
    uint16_t adcTimeout = 100U;
    while((ADC_getInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1) == false) && (--adcTimeout > 0U))
    {
    }
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);

    adcRawData.isenMot = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER2);
    adcRawData.isenBrk = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER3);
    adcRawData.vsen28v = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER4);
    adcRawData.vsen5vd = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER5);
    adcRawData.vsenRef = ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER1);
    adcRawData.tsenBd  = ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER3);

    // 100us 시스템 운용 파이프라인 실행
    Control_SystemOperation();


    // --- 사용자 요청: 메인컨트롤 ISR 동작 확인용 GPIO 34번 토글 (1초 주기: 5000번 호출 시 상태 반전) ---
    static uint16_t isrAliveCounter = 0;
    isrAliveCounter++;
    if (isrAliveCounter >= 5000U)
    {
        isrAliveCounter = 0;
        GPIO_togglePin(34U);
    }
    // ---------------------------------------------------------------------------------

    EPWM_clearEventTriggerInterruptFlag(EPWM_TIMER1_BASE);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP3);
}
