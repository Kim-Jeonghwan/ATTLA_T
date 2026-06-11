/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Control.c
    Version          : 00.00
    Description      : 시스템 제어 모듈 (PBIT, CBIT, 시스템 운용 파이프라인) 구현
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 11. (신규 생성)
**********************************************************************/

#include "csu_Control.h"

volatile uint16_t isOffsetCalibrated = 0U;
volatile uint16_t isPbitComplete = 0U;

/*
@funtion    void csu_Control_CalibrateCurrentOffset(void)
@brief      전류 센서 오프셋 영점 조정 (PWM ISR 호출용)
@param      void
@return     void
*/
void csu_Control_CalibrateCurrentOffset(void)
{
    // TODO: 모터/브레이크 미구동 상태에서 ADC 값 누적 및 영점 계산
    // TODO: FRAM에 오프셋 값 저장
    
    // 임시: 즉시 완료 처리
    isOffsetCalibrated = 1U;
}

/*
@funtion    void csu_Bit_RunPBIT(void)
@brief      초기 점검 (PBIT) 수행 (PWM ISR 호출용)
@param      void
@return     void
*/
void csu_Bit_RunPBIT(void)
{
    // TODO: 각 하드웨어 및 센서 상태 초기 점검 로직 구현
    
    // 임시: 즉시 완료 처리
    isPbitComplete = 1U;
}

/*
@funtion    void csu_Bit_RunCBIT(void)
@brief      주기 점검 (CBIT) 수행
@param      void
@return     void
*/
void csu_Bit_RunCBIT(void)
{
    // TODO: 시스템 운용 중 주기적인 점검 로직 구현
}

/*
@funtion    void csu_Control_SystemOperation(void)
@brief      100us PWM 인터럽트 기반 시스템 운용 파이프라인
@param      void
@return     void
*/
void csu_Control_SystemOperation(void)
{
    if (isOffsetCalibrated == 0U)
    {
        csu_Control_CalibrateCurrentOffset();
        return; // 오프셋 완료 전까지 운용 로직 대기
    }
    
    if (isPbitComplete == 0U)
    {
        csu_Bit_RunPBIT();
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
    csu_Bit_RunCBIT();

    // 6. 모터 구동제어 CSU
    // runMotorControl();

    // 7. 데이터 저장 CSU
    // saveData();
}
