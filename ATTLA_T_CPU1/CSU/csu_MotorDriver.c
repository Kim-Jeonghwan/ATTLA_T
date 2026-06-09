/**********************************************************************
 Nexcom Co., Ltd.
 Filename         : csu_MotorDriver.c
 Version          : 00.00
 Description      : DRV8343 모터 드라이버 제어 어플리케이션 계층
 Programmer       : Kim Jeonghwan
 Last Updated     : 2026. 06. 09. (신규 생성)
**********************************************************************/

#include "csu_MotorDriver.h"

//---------------------------------------------------------------------------
// 전역 변수
//---------------------------------------------------------------------------
uint16_t motorDriverFaultStatus = 0;

//---------------------------------------------------------------------------
// MotorDriver_Init
//---------------------------------------------------------------------------
void MotorDriver_Init(void)
{
    // HAL 초기화 호출 (SPI-B 설정)
    MotorDriver_Init_Hardware();

    // 초기 상태에서 에러 레지스터 클리어 및 기본 설정 (예: PWM 모드)
    MotorDriver_ClearFaults();
    
    // 예: DRV_CTRL 레지스터 설정 (PWM Mode 등)
    // MotorDriver_WriteReg(DRV8343_REG_DRV_CTRL, 0x00...);
}

//---------------------------------------------------------------------------
// MotorDriver_ClearFaults
//---------------------------------------------------------------------------
void MotorDriver_ClearFaults(void)
{
    // CLR_FLT 비트(Bit 0)를 세트하여 에러 해제
    // (데이터시트 확인: DRV_CTRL 레지스터의 비트 0이 CLR_FLT인 경우)
    uint16_t currentCtrl = MotorDriver_ReadReg(DRV8343_REG_DRV_CTRL);
    currentCtrl |= 0x01; 
    MotorDriver_WriteReg(DRV8343_REG_DRV_CTRL, currentCtrl);
}

//---------------------------------------------------------------------------
// MotorDriver_UpdateStatus
//---------------------------------------------------------------------------
void MotorDriver_UpdateStatus(void)
{
    // 주기적으로 FAULT_STATUS_1 레지스터를 읽어서 에러 발생 여부 확인
    motorDriverFaultStatus = MotorDriver_ReadReg(DRV8343_REG_FAULT_STATUS_1);
}
