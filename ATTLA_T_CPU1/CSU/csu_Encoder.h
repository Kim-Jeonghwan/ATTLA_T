/**********************************************************************
 Nexcom Co., Ltd.
 Filename         : csu_Encoder.h
 Version          : 00.04
 Description      : AksIM-2 엔코더 어플리케이션 기능 처리 모듈
 Programmer       : Kim Jeonghwan
 Last Updated     : 2026. 06. 11. (전역 변수 구조체화 마이그레이션)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 11. - 전역 변수를 stEncoderState 구조체(xEncoder)로 통합하여 네임스페이스 및 상태 관리 개선
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
// 전역 변수 구조체
//---------------------------------------------------------------------------
typedef struct {
    // 1. 수신 원시 데이터 관련
    uint64_t fullFrame;   // SPI에서 수신된 64비트 전체 데이터
    uint64_t rawPos;      // 파싱된 34비트 Position 원시값
    uint8_t  errBit;      // 파싱된 Error 비트 (Active Low)
    uint8_t  warnBit;     // 파싱된 Warning 비트
    uint8_t  crcRecv;     // 파싱된 수신 CRC (6비트)
    uint8_t  crcCalc;     // 자체 계산한 CRC (6비트)

    // 2. 어플리케이션 상태
    uint64_t offset;      // 제로셋 오프셋 값
    uint64_t position;    // 오프셋 적용 및 롤오버가 반영된 실시간 위치
    float32_t angleDeg;   // 360도 환산 기계각
    bool      isValid;    // Error 비트 및 CRC 검증 결과에 따른 최종 유효성
} stEncoderState;

extern stEncoderState xEncoder;

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
