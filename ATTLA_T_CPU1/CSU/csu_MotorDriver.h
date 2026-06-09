/**********************************************************************
 Nexcom Co., Ltd.
 Filename         : csu_MotorDriver.h
 Version          : 00.00
 Description      : DRV8343 모터 드라이버 제어 어플리케이션 계층
 Programmer       : Kim Jeonghwan
 Last Updated     : 2026. 06. 09. (신규 생성)
**********************************************************************/

#ifndef CSU_MOTORDRIVER_H_
#define CSU_MOTORDRIVER_H_

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------------
// DRV8343 레지스터 맵
//---------------------------------------------------------------------------
#define DRV8343_REG_FAULT_STATUS_1  0x00
#define DRV8343_REG_FAULT_STATUS_2  0x01
#define DRV8343_REG_DRV_CTRL        0x02
#define DRV8343_REG_GATE_DRV_HS     0x03
#define DRV8343_REG_GATE_DRV_LS     0x04
#define DRV8343_REG_OCP_CTRL        0x05
#define DRV8343_REG_CSA_CTRL        0x06

//---------------------------------------------------------------------------
// 전역 변수
//---------------------------------------------------------------------------
// 모터 에러 상태를 저장하는 변수 (접두어 없음)
extern uint16_t motorDriverFaultStatus;

//---------------------------------------------------------------------------
// 함수 프로토타입
//---------------------------------------------------------------------------
extern void MotorDriver_Init(void);
extern void MotorDriver_ClearFaults(void);
extern void MotorDriver_UpdateStatus(void);

#ifdef __cplusplus
}
#endif

#endif /* CSU_MOTORDRIVER_H_ */
