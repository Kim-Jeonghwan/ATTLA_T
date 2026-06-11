/**********************************************************************
  Nexcom Co., Ltd.
  Filename         : csu_MotorDriver.c
  Version          : 00.02
  Description      : DRV8343 모터 드라이버 제어 어플리케이션 계층
  Programmer       : Kim Jeonghwan
  Last Updated     : 2026. 06. 11. (주석 표준화 및 레거시 코드 정리)
 **********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 상태 변수들을 stMotorDriverState 구조체(xMotorDriver)로 통합
 * 2026. 06. 11. - hal_MotorDriver.h의 레지스터 매크로(DRV8343_REG_CONTROL_1)로 변경 반영
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */


#include "csu_MotorDriver.h"

//---------------------------------------------------------------------------
// 전역 변수
//---------------------------------------------------------------------------
// 모터 드라이버 상태
stMotorDriverState xMotorDriver;

/**
 * @function MotorDriver_Init
 * @brief    모터 드라이버(DRV8343) 상태 초기화 및 기본 레지스터 셋업
 * @param    void
 * @return   void
 */
void MotorDriver_Init(void)
{
    // 구조체 명시적 초기화
    xMotorDriver.faultStatus = 0U;

    // 하드웨어 핀 초기 상태 설정
    MotorDriver_Enable(false);
    
    // HAL 초기화 호출 (SPI-B 설정)
    MotorDriver_Init_Hardware();

    // 초기 상태에서 에러 레지스터 클리어 및 기본 설정 (예: PWM 모드)
    MotorDriver_ClearFaults();
    
    // 예: DRV_CTRL 레지스터 설정 (PWM Mode 등)
    // MotorDriver_WriteReg(DRV8343_REG_CONTROL_1, 0x00...);
}

/**
 * @function MotorDriver_ClearFaults
 * @brief    DRV8343 드라이버 내부의 결함 상태 레지스터 클리어 명령 전송
 * @param    void
 * @return   void
 */
void MotorDriver_ClearFaults(void)
{
    // CLR_FLT 비트(Bit 0)를 세트하여 에러 해제
    // (데이터시트 확인: DRV_CTRL 레지스터의 비트 0이 CLR_FLT인 경우)
    uint16_t currentCtrl = MotorDriver_ReadReg(DRV8343_REG_CONTROL_1);
    currentCtrl |= 0x01; 
    MotorDriver_WriteReg(DRV8343_REG_CONTROL_1, currentCtrl);
}

/**
 * @function MotorDriver_UpdateStatus
 * @brief    DRV8343의 결함 레지스터 상태를 주기적으로 읽어 구조체 업데이트
 * @param    void
 * @return   void
 */
void MotorDriver_UpdateStatus(void)
{
    // 주기적으로 FAULT_STATUS_1 레지스터를 읽어서 에러 발생 여부 확인
    xMotorDriver.faultStatus = MotorDriver_ReadReg(DRV8343_REG_FAULT_STATUS_1);
}
