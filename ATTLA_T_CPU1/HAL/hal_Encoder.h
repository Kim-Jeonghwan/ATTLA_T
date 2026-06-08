/**********************************************************************
 Nexcom Co., Ltd.
 Filename         : hal_Encoder.h
 Version          : 00.01
 Description      : AksIM-2 엔코더 제어를 위한 HAL (하드웨어 초기화 및 pm_bissc 연동)
 Programmer       : Kim Jeonghwan
 Last Updated     : 2026. 06. 09. (신규 생성)
**********************************************************************/

#ifndef HAL_ENCODER_H_
#define HAL_ENCODER_H_

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------------
// 매크로 정의
//---------------------------------------------------------------------------
#define HAL_ENC_DATA_PIN        25      // SPI-B SOMI
#define HAL_ENC_CLK_PIN         26      // OUTPUT X-BAR 3 (CLB MA Clock)

//---------------------------------------------------------------------------
// 전역 변수 선언 (pm_bissc 파라미터 구조체)
//---------------------------------------------------------------------------
extern PM_bissc_scdStruct encoderScdParams;
extern PM_bissc_cdStruct encoderCdParams;
extern PM_bissc_encoderStruct encoderData;

//---------------------------------------------------------------------------
// 함수 프로토타입
//---------------------------------------------------------------------------
extern void Encoder_Init_Hardware(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_ENCODER_H_ */
