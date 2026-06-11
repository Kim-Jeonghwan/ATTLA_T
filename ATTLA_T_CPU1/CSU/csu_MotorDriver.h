/**********************************************************************
 Nexcom Co., Ltd.
 Filename         : csu_MotorDriver.h
 Version          : 00.01
 Description      : DRV8343 모터 드라이버 제어 어플리케이션 계층
 Programmer       : Kim Jeonghwan
 Last Updated     : 2026. 06. 11. (중복 매크로 정의 제거)
**********************************************************************/

/*
 * Modification History
 * --------------------
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
