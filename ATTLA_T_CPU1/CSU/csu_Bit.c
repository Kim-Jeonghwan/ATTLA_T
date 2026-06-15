/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Bit.c
    Version          : 00.08
    Description      : 1x PWM 구조용 간소화된 BIT 로직 (CSU)
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 12. (과속 판단 기준 3240 RPM으로 하향 조정)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 12. - 과속 판단 기준을 3240 RPM으로 하향 조정
 * 2026. 06. 12. - 매크로 상수명 추상화: BIT_CNT_FILTER_REF 반영
 * 2026. 06. 12. - 매크로 상수들을 헤더 파일로 이동하여 중복 제거
 * 2026. 06. 12. - PM_n24V 및 GateFault 점검 로직을 디바운싱된 xDio 변수 참조로 변경
 * 2026. 06. 11. - 신규 결함 진단(Stall, OverSpeed, Encoder) 함수 구현
 * 2026. 06. 11. - PM_n24V 및 GateFault 점검에 Driverlib(GPIO_readPin) 적용
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 상태 변수들을 stBitState 구조체(xBit)로 통합
 * 2026. 06. 11. - 불필요한 헤더 Include 제거
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 * 2026. 06. 11. - 함수명 접두어(csu_, hal_) 제거 리팩토링
 */


#include "csu_Bit.h"

// Global variables used in this project
stBitState xBit;

/**
 * @function Bit_Init
 * @brief    Built-In Test (BIT) 상태 변수 및 에러 구조체 초기화
 * @param    void
 * @return   void
 */
void Bit_Init(void)
{
    // 구조체 명시적 초기화
    Bit_FaultReset(1U);
}

/**
 * @function Bit_OvCurrent_Check
 * @brief    모터 및 브레이크 전류의 과전류 여부 검증 (100ms 누적 필터링)
 * @param    void
 * @return   void
 */
void Bit_OvCurrent_Check(void)
{
    static Uint16 BitCnt_Mot = 0U;
    static Uint16 BitCnt_Brk = 0U;

    // 모터 과전류 체크
    if (xAdc.isenMotLpf > BIT_LIMIT_OVC_MOT_MAX)
    {
        if (BitCnt_Mot > BIT_CNT_FILTER_REF)
        {
            xBit.faultOvCurrMot = 1U;
            xBit.faultFlagSet = 1U;
            xBit.informAll |= 0x00000100U;
            BitCnt_Mot = 0U;
        }
        else BitCnt_Mot++;
    }
    else
    {
        if (BitCnt_Mot > 0U) BitCnt_Mot--;
    }

    // 브레이크 과전류 체크
    if (xAdc.isenBrkLpf > BIT_LIMIT_OVC_BRK_MAX)
    {
        if (BitCnt_Brk > BIT_CNT_FILTER_REF)
        {
            xBit.faultOvCurrBrk = 1U;
            xBit.faultFlagSet = 1U;
            xBit.informAll |= 0x00000200U;
            BitCnt_Brk = 0U;
        }
        else BitCnt_Brk++;
    }
    else
    {
        if (BitCnt_Brk > 0U) BitCnt_Brk--;
    }
}

/**
 * @function Bit_OvTemperature_Check
 * @brief    보드 온도 센서의 과열 여부 검증 (100ms 누적 필터링)
 * @param    void
 * @return   void
 */
void Bit_OvTemperature_Check(void)
{
    static Uint16 BitCnt_Bd = 0U;

    if (xAdc.tsenBdLpf > BIT_LIMIT_OVT_BD_MAX)
    {
        if (BitCnt_Bd > BIT_CNT_FILTER_REF)
        {
            xBit.faultOvTempBd = 1U;
            xBit.faultFlagSet = 1U;
            xBit.informAll |= 0x00000800U;
            BitCnt_Bd = 0U;
        }
        else BitCnt_Bd++;
    }
    else
    {
        if (BitCnt_Bd > 0U) BitCnt_Bd--;
    }
}

/**
 * @function Bit_OvVoltage_Check
 * @brief    시스템 입력 28V 전압의 과전압 여부 검증 (100ms 누적 필터링)
 * @param    void
 * @return   void
 */
void Bit_OvVoltage_Check(void)
{
    static Uint16 BitCnt_28V = 0U;
    static Uint16 BitCnt_Brk24V = 0U;

    // 28V 과전압 감시
    if (xAdc.vsen28VLpf > BIT_LIMIT_OVV_28V_MAX)
    {
        if (BitCnt_28V > BIT_CNT_FILTER_REF)
        {
            xBit.faultOvVolt28V = 1U;
            xBit.faultFlagSet = 1U;
            xBit.informAll |= 0x00001000U;
            BitCnt_28V = 0U;
        }
        else BitCnt_28V++;
    }
    else
    {
        if (BitCnt_28V > 0U) BitCnt_28V--;
    }

    // 신규 PM_n24V 브레이크 전압 감시 (Active Low, 디바운싱 필터 적용)
    if (xDio.pm24V == 0U)
    {
        if (BitCnt_Brk24V > BIT_LIMIT_OVV_BRK_TIME_CNT)
        {
            xBit.faultOvVoltBrk = 1U;
            xBit.faultFlagSet = 1U;
            xBit.informAll |= 0x00002000U;
            BitCnt_Brk24V = 0U;
        }
        else BitCnt_Brk24V++;
    }
    else
    {
        if (BitCnt_Brk24V > 0U) BitCnt_Brk24V--;
    }
}

/**
 * @function Bit_GateFault_Check
 * @brief    모터 드라이버(DRV8343) 하드웨어 nFAULT 신호 상태 감지
 * @param    void
 * @return   void
 */
void Bit_GateFault_Check(void)
{
    // DRV8343 nFAULT 확인 로직
    // nFAULT 핀은 Active Low 이므로 '0'일 때 에러 상태입니다.
    // 디바운싱 처리된 xDio.drvFault 변수를 참조합니다.
    if (xDio.drvFault == 0U)
    {
        xBit.faultDrv8343nFault = 1U;
        xBit.faultFlagSet = 1U;
        xBit.informAll |= 0x00010000U;
    }
}

/**
 * @function Bit_MotorStall_Check
 * @brief    모터의 스톨 상태 감지 (전류 5A 초과 및 속도 10 RPM 미만이 1.0초 지속)
 * @param    void
 * @return   void
 */
void Bit_MotorStall_Check(void)
{
    float32_t currentAbs = (xAdc.isenMotLpf < 0.0f) ? -xAdc.isenMotLpf : xAdc.isenMotLpf;
    float32_t speedAbs = (xMotorCtrl.currentSpeedRpm < 0.0f) ? -xMotorCtrl.currentSpeedRpm : xMotorCtrl.currentSpeedRpm;

    if (currentAbs > BIT_LIMIT_STALL_CURR_MIN && speedAbs < BIT_LIMIT_STALL_RPM_LIMIT)
    {
        if (xBit.stallCheckCnt > BIT_LIMIT_STALL_TIME_CNT)
        {
            xBit.faultStall = 1U;
            xBit.faultFlagSet = 1U;
            xBit.informAll |= 0x00020000U;
            xBit.stallCheckCnt = 0U;
        }
        else xBit.stallCheckCnt++;
    }
    else
    {
        if (xBit.stallCheckCnt > 0U) xBit.stallCheckCnt--;
    }
}

/**
 * @function Bit_MotorOverSpeed_Check
 * @brief    모터의 과속 상태 감지 (절대 속도 3240 RPM 초과가 100ms 지속)
 * @param    void
 * @return   void
 */
void Bit_MotorOverSpeed_Check(void)
{
    static Uint16 BitCnt_OvSpeed = 0U;
    
    if (xMotorCtrl.currentSpeedRpm > BIT_LIMIT_SPEED_MOT_MAX || xMotorCtrl.currentSpeedRpm < BIT_LIMIT_SPEED_MOT_MIN)
    {
        if (BitCnt_OvSpeed > BIT_LIMIT_OVS_TIME_CNT)
        {
            xBit.faultOverSpeed = 1U;
            xBit.faultFlagSet = 1U;
            xBit.informAll |= 0x00040000U;
            BitCnt_OvSpeed = 0U;
        }
        else BitCnt_OvSpeed++;
    }
    else
    {
        if (BitCnt_OvSpeed > 0U) BitCnt_OvSpeed--;
    }
}

/**
 * @function Bit_Encoder_Check
 * @brief    엔코더 상태 이상 감지 (에러 및 워닝 비트 검출)
 * @param    void
 * @return   void
 */
void Bit_Encoder_Check(void)
{
    if (xEncoder.errBit == 0U)
    {
        xBit.faultEncError = 1U;
        xBit.faultFlagSet = 1U;
        xBit.informAll |= 0x00080000U;
    }
    
    if (xEncoder.warnBit == 0U)
    {
        xBit.warnEncWarning = 1U;
        xBit.informAll |= 0x00100000U;
    }
}

/**
 * @function Bit_FaultReset
 * @brief    시스템의 전체 BIT 결함 플래그 및 레지스터 리셋
 * @param    Data : 1U 인 경우 리셋 실행
 * @return   void
 */
void Bit_FaultReset(Uint16 Data)
{
    if (Data == 1U)
    {
        xBit.informAll = 0U;
        xBit.startFlagSet = 0U;
        xBit.faultFlagSet = 0U;

        xBit.faultOvCurrMot = 0U;
        xBit.faultOvCurrBrk = 0U;
        xBit.faultOvTempBd = 0U;
        xBit.faultOvVolt28V = 0U;
        xBit.faultOvVoltBrk = 0U;
        xBit.faultDrv8343nFault = 0U;
        xBit.faultStall = 0U;
        xBit.faultOverSpeed = 0U;
        xBit.faultEncError = 0U;
        xBit.warnEncWarning = 0U;
        xBit.stallCheckCnt = 0U;
    }
}
