/**********************************************************************
 Nexcom Co., Ltd.
 Filename         : csu_Encoder.h
 Version          : 00.01
 Description      : AksIM-2 엔코더 어플리케이션 기능 처리 모듈
 Programmer       : Kim Jeonghwan
 Last Updated     : 2026. 06. 09. (신규 생성)
**********************************************************************/

#ifndef CSU_ENCODER_H_
#define CSU_ENCODER_H_

#include "main.h"
#include "hal_Encoder.h"

#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------------
// 레지스터 및 커맨드 매크로 (AksIM-2 데이터시트 기준)
//---------------------------------------------------------------------------
#define ENC_REG_BANK_SEL        0x40

#define ENC_REG_KEY             0x48
#define ENC_KEY_UNLOCK          0xCD

#define ENC_REG_CMD             0x49
#define ENC_CMD_SAVE_PARAMS     0x63  // 'c'
#define ENC_CMD_SAVE_MULTITURN  0x6D  // 'm'
#define ENC_CMD_START_CALIB     0x41  // 'A'
#define ENC_CMD_FACTORY_RESET   0x72  // 'r'
#define ENC_CMD_SAVE_USER_MEM   0x75  // 'u'

#define ENC_REG_STATUS_H        0x4A
#define ENC_REG_STATUS_L        0x4B
#define ENC_REG_TEMP_H          0x4C
#define ENC_REG_TEMP_L          0x4D
#define ENC_REG_SIGNAL_LVL_H    0x4E
#define ENC_REG_SIGNAL_LVL_L    0x4F
#define ENC_REG_RPM_H           0x50
#define ENC_REG_RPM_L           0x51
#define ENC_REG_CALIB_STATUS    0x52

//---------------------------------------------------------------------------
// 함수 프로토타입
//---------------------------------------------------------------------------
extern void Encoder_Init(void);
extern void Encoder_UpdatePosition(void);
extern void Encoder_ProcessCDTasks(void);

extern void Encoder_SaveParameters(void);
extern void Encoder_StartCalibration(void);
extern void Encoder_RequestTemperature(void);

#ifdef __cplusplus
}
#endif

#endif /* CSU_ENCODER_H_ */
