/**********************************************************************
 Nexcom Co., Ltd.
 Filename         : hal_Encoder.h
 Version          : 00.02
 Description      : AksIM-2 엔코더 제어를 위한 HAL (하드웨어 초기화 및 SPI 통신)
 Programmer       : Kim Jeonghwan
 Last Updated     : 2026. 06. 11. (64비트 수신 반환형 변경 및 불필요 변수 제거)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 11. - 64비트 수신 반환형 변경 및 불필요 변수 제거
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */


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
// 함수 프로토타입
//---------------------------------------------------------------------------
extern void Encoder_Init_Hardware(void);
extern uint64_t Encoder_ReadSpiData(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_ENCODER_H_ */
