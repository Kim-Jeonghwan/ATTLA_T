/**********************************************************************
 Nexcom Co., Ltd.
 Filename         : hal_MotorDriver.h
 Version          : 00.00
 Description      : DRV8343 모터 드라이버 하드웨어 초기화 (SPI-B)
 Programmer       : Kim Jeonghwan
 Last Updated     : 2026. 06. 09. (신규 생성)
**********************************************************************/

#ifndef HAL_MOTORDRIVER_H_
#define HAL_MOTORDRIVER_H_

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------------
// 핀 매크로 정의 (SPI-B)
//---------------------------------------------------------------------------
#define HAL_MOTORDRIVER_SPI_CLK_PIN       58
#define HAL_MOTORDRIVER_SPI_STE_PIN       59
#define HAL_MOTORDRIVER_SPI_SIMO_PIN      60
#define HAL_MOTORDRIVER_SPI_SOMI_PIN      61

//---------------------------------------------------------------------------
// 함수 프로토타입
//---------------------------------------------------------------------------
extern void MotorDriver_Init_Hardware(void);
extern uint16_t MotorDriver_ReadReg(uint16_t addr);
extern void MotorDriver_WriteReg(uint16_t addr, uint16_t data);

#ifdef __cplusplus
}
#endif

#endif /* HAL_MOTORDRIVER_H_ */
