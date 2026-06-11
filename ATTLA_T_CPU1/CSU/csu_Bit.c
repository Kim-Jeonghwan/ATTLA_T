/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Bit.c
    Version          : 00.03
    Description      : 1x PWM 구조용 간소화된 BIT 로직 (CSU)
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 11. (주석 표준화 및 레거시 코드 정리)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 상태 변수들을 stBitState 구조체(xBit)로 통합
 * 2026. 06. 11. - 불필요한 헤더 Include 제거
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 * 2026. 06. 11. - 함수명 접두어(csu_, hal_) 제거 리팩토링
 */


#include "csu_Bit.h"

// Global variables used in this project
stBitState xBit;

// 임계치 매크로 (Project Spec 기반)
#define BIT_LIMIT_OVC_MOT_MAX 10.0f // 최대 연속 전류 9.34A 초과 시 Fault
#define BIT_LIMIT_OVC_BRK_MAX 1.5f  // 브레이크 최대 동작 전류 1.0A 초과 시 Fault
#define BIT_LIMIT_OVT_BD_MAX  80.0f // 보드 내부 온도
#define BIT_LIMIT_OVV_28V_MAX 32.0f // 28V 과전압

#define BIT_CNT_REF_100MS 1000U // 100us * 1000 = 100ms

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
        if (BitCnt_Mot > BIT_CNT_REF_100MS)
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
        if (BitCnt_Brk > BIT_CNT_REF_100MS)
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
        if (BitCnt_Bd > BIT_CNT_REF_100MS)
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

    if (xAdc.vsen28VLpf > BIT_LIMIT_OVV_28V_MAX)
    {
        if (BitCnt_28V > BIT_CNT_REF_100MS)
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
    // nFAULT 핀은 Active Low 이므로 '0'일 때 에러 상태입니다. (GPIO 10번 가정)
    if (GpioDataRegs.GPADAT.bit.GPIO10 == 0U)
    {
        xBit.faultDrv8343nFault = 1U;
        xBit.faultFlagSet = 1U;
        xBit.informAll |= 0x00010000U;
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
        xBit.faultDrv8343nFault = 0U;
    }
}
