/**********************************************************************
 Nexcom Co., Ltd.
 Filename         : hal_MotorDriver.h
 Version          : 00.00
 Description      : DRV8343 모터 드라이버 하드웨어 초기화 (SPI-B)
 Programmer       : Kim Jeonghwan
 Last Updated     : 2026. 06. 11. (방향 제어 핀 매크로 및 함수 원형 추가)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 11. - 모터 방향 제어용 함수 원형 및 매크로 추가
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */


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
// 모터 방향 핀 매크로 (회로도 기준 GPIO 3)
//---------------------------------------------------------------------------
#define DRV8343_DIR_PIN                   3U

//---------------------------------------------------------------------------
// DRV8343 Register Addresses & Commands
//---------------------------------------------------------------------------
#define DRV8343_REG_FAULT_STATUS_1      0x00U
#define DRV8343_REG_FAULT_STATUS_2      0x01U
#define DRV8343_REG_CONTROL_1           0x02U
#define DRV8343_REG_CONTROL_2           0x03U
#define DRV8343_REG_CONTROL_3           0x04U
#define DRV8343_REG_CONTROL_4           0x05U
#define DRV8343_REG_CONTROL_5           0x06U
#define DRV8343_REG_CONTROL_6           0x07U

// 1x PWM Mode Setting (PWM_MODE = 10b in Control 2 Register, Bits 6:5)
#define DRV8343_CTRL2_PWM_MODE_1X       (0x02U << 5)

//---------------------------------------------------------------------------
// 함수 프로토타입
//---------------------------------------------------------------------------
extern void MotorDriver_Init_Hardware(void);
extern uint16_t MotorDriver_ReadReg(uint16_t addr);
extern void MotorDriver_WriteReg(uint16_t addr, uint16_t data);
extern void MotorDriver_SetDir(bool bForward);

#ifdef __cplusplus
}
#endif

#endif /* HAL_MOTORDRIVER_H_ */
