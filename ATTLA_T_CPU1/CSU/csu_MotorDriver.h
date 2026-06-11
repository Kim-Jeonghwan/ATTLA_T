/**********************************************************************
 Nexcom Co., Ltd.
 Filename         : csu_MotorDriver.h
 Version          : 00.02
 Description      : DRV8343 모터 드라이버 제어 어플리케이션 계층
 Programmer       : Kim Jeonghwan
 Last Updated     : 2026. 06. 11. (전역 변수 구조체화 마이그레이션)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 11. - 상태 변수들을 stMotorDriverState 구조체(xMotorDriver)로 통합
 * 2026. 06. 11. - hal_MotorDriver.h와 중복되는 DRV8343 레지스터 매크로 정의 제거
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */


#ifndef CSU_MOTORDRIVER_H_
#define CSU_MOTORDRIVER_H_

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif


//---------------------------------------------------------------------------
// 전역 변수
//---------------------------------------------------------------------------
// 모터 에러 상태를 저장하는 구조체
typedef struct {
    uint16_t faultStatus;
} stMotorDriverState;

extern stMotorDriverState xMotorDriver;

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
