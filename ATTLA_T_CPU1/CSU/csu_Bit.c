/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Bit.c
    Version          : 00.02
    Description      : 1x PWM 구조용 간소화된 BIT 로직 (CSU)
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 11. (함수명 명명 규칙 위반 접두어 제거)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 11. - 불필요한 헤더 Include 제거
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 * 2026. 06. 11. - 함수명 접두어(csu_, hal_) 제거 리팩토링
 */


#include "csu_Bit.h"

// Global variables used in this project
Uint32  Bit_Inform_all = 0U;   // BIT 결과 데이터(종합)
Uint16  BitStartFlag_Set = 0U; // BIT 시작 시 '1'
Uint16  BitFaultFlag_Set = 0U; // Fault 시 '1', 해당 Flag 발생 시 구동 Disable

Uint16  BitFault_OvCurr_Mot = 0U;
Uint16  BitFault_OvCurr_Brk = 0U;
Uint16  BitFault_OvTemp_Bd = 0U;
Uint16  BitFault_OvVolt_28V = 0U;
Uint16  BitFault_Drv8343_nFault = 0U;

// 임계치 매크로 (Project Spec 기반)
#define BIT_LIMIT_OVC_MOT_MAX 10.0f // 최대 연속 전류 9.34A 초과 시 Fault
#define BIT_LIMIT_OVC_BRK_MAX 1.5f  // 브레이크 최대 동작 전류 1.0A 초과 시 Fault
#define BIT_LIMIT_OVT_BD_MAX  80.0f // 보드 내부 온도
#define BIT_LIMIT_OVV_28V_MAX 32.0f // 28V 과전압

#define BIT_CNT_REF_100MS 1000U // 100us * 1000 = 100ms

void Bit_OvCurrent_Check(void)
{
    static Uint16 BitCnt_Mot = 0U;
    static Uint16 BitCnt_Brk = 0U;

    // 모터 과전류 체크
    if (Isen_Mot_lpf > BIT_LIMIT_OVC_MOT_MAX)
    {
        if (BitCnt_Mot > BIT_CNT_REF_100MS)
        {
            BitFault_OvCurr_Mot = 1U;
            BitFaultFlag_Set = 1U;
            Bit_Inform_all |= 0x00000100U;
            BitCnt_Mot = 0U;
        }
        else BitCnt_Mot++;
    }
    else
    {
        if (BitCnt_Mot > 0U) BitCnt_Mot--;
    }

    // 브레이크 과전류 체크
    if (Isen_Brk_lpf > BIT_LIMIT_OVC_BRK_MAX)
    {
        if (BitCnt_Brk > BIT_CNT_REF_100MS)
        {
            BitFault_OvCurr_Brk = 1U;
            BitFaultFlag_Set = 1U;
            Bit_Inform_all |= 0x00000200U;
            BitCnt_Brk = 0U;
        }
        else BitCnt_Brk++;
    }
    else
    {
        if (BitCnt_Brk > 0U) BitCnt_Brk--;
    }
}

void Bit_OvTemperature_Check(void)
{
    static Uint16 BitCnt_Bd = 0U;

    if (Tsen_Bd_lpf > BIT_LIMIT_OVT_BD_MAX)
    {
        if (BitCnt_Bd > BIT_CNT_REF_100MS)
        {
            BitFault_OvTemp_Bd = 1U;
            BitFaultFlag_Set = 1U;
            Bit_Inform_all |= 0x00000800U;
            BitCnt_Bd = 0U;
        }
        else BitCnt_Bd++;
    }
    else
    {
        if (BitCnt_Bd > 0U) BitCnt_Bd--;
    }
}

void Bit_OvVoltage_Check(void)
{
    static Uint16 BitCnt_28V = 0U;

    if (Vsen_28V_lpf > BIT_LIMIT_OVV_28V_MAX)
    {
        if (BitCnt_28V > BIT_CNT_REF_100MS)
        {
            BitFault_OvVolt_28V = 1U;
            BitFaultFlag_Set = 1U;
            Bit_Inform_all |= 0x00001000U;
            BitCnt_28V = 0U;
        }
        else BitCnt_28V++;
    }
    else
    {
        if (BitCnt_28V > 0U) BitCnt_28V--;
    }
}

void Bit_GateFault_Check(void)
{
    // DRV8343 nFAULT 확인 로직
    // nFAULT 핀은 Active Low 이므로 '0'일 때 에러 상태입니다. (GPIO 10번 가정)
    if (GpioDataRegs.GPADAT.bit.GPIO10 == 0U)
    {
        BitFault_Drv8343_nFault = 1U;
        BitFaultFlag_Set = 1U;
        Bit_Inform_all |= 0x00010000U;
    }
}

// Fault Reset
void Bit_FaultReset(Uint16 Data)
{
    if (Data == 1U)
    {
        Bit_Inform_all = 0U;
        BitStartFlag_Set = 0U;
        BitFaultFlag_Set = 0U;

        BitFault_OvCurr_Mot = 0U;
        BitFault_OvCurr_Brk = 0U;
        BitFault_OvTemp_Bd = 0U;
        BitFault_OvVolt_28V = 0U;
        BitFault_Drv8343_nFault = 0U;
    }
}
