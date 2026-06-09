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
#define ENC_DATA_PIN        51      // SPIC_SOMI
#define ENC_CLK_PIN         52      // SPIC_CLK

//---------------------------------------------------------------------------
// 전역 변수 선언
//---------------------------------------------------------------------------
// BiSS-C 대신 위치값을 수신할 변수 (임시, 필요시 CSU에서 관리)
extern uint32_t encRawData;

//---------------------------------------------------------------------------
// 함수 프로토타입
//---------------------------------------------------------------------------
extern void Encoder_Init_Hardware(void);
extern uint32_t Encoder_ReadSpiData(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_ENCODER_H_ */
