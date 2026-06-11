/**********************************************************************
 Nexcom Co., Ltd.
 Filename         : csu_Encoder.h
 Version          : 00.03
 Description      : AksIM-2 엔코더 어플리케이션 기능 처리 모듈
 Programmer       : Kim Jeonghwan
 Last Updated     : 2026. 06. 11. (FRAM 오프셋 로드 및 저장 연동 추가)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 11. - FRAM 오프셋 연동을 위한 매크로(ENC_OFFSET_FRAM_ADDR) 및 Load 함수 추가
 * 2026. 06. 11. - 64비트 원시/오프셋/위치 변수, 에러 플래그, 기계각(float) 변수, 제로셋 함수 추가
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */


#ifndef CSU_ENCODER_H_
#define CSU_ENCODER_H_

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------------
// 매크로
//---------------------------------------------------------------------------
#define ENC_OFFSET_FRAM_ADDR    0x0000u

//---------------------------------------------------------------------------
// 전역 변수
//---------------------------------------------------------------------------
extern uint64_t encRawData;
extern uint64_t encOffset;
extern uint64_t encPosition;
extern float32_t encAngleDeg;
extern bool isEncError;

//---------------------------------------------------------------------------
// 함수 프로토타입
//---------------------------------------------------------------------------
extern void Encoder_Init(void);
extern void Encoder_LoadOffset(void);
extern void Encoder_UpdatePosition(void);
extern void Encoder_SetZero(void);

#ifdef __cplusplus
}
#endif

#endif /* CSU_ENCODER_H_ */
