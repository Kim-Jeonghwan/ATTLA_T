/**********************************************************************
  Nexcom Co., Ltd.
  Filename         : csu_MotorDriver.h
  Version          : 00.04
  Description      : DRV8343 모터 드라이버 제어 어플리케이션 계층
  Programmer       : Kim Jeonghwan
  Last Updated     : 2026. 07. 01. (구조체 변수 상세 한글 주석 추가)
 **********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 07. 01. - 구조체 변수 상세 한글 주석 추가 (코딩 규칙 적용)
 * 2026. 06. 23. - main.h -> main_cpu1.h 인클루드 명칭 리팩토링
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 상태 변수들을 stMotorDriverState 구조체(xMotorDriver)로 통합
 * 2026. 06. 11. - hal_MotorDriver.h와 중복되는 DRV8343 레지스터 매크로 정의 제거
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */


#ifndef CSU_MOTORDRIVER_H_
#define CSU_MOTORDRIVER_H_

#include "main_cpu1.h"

#ifdef __cplusplus
extern "C" {
#endif


//---------------------------------------------------------------------------
// 전역 변수
//---------------------------------------------------------------------------
// 모터 에러 상태를 저장하는 구조체
typedef struct {
    uint16_t faultStatus;      // DRV8343의 FAULT_STATUS_1 등 결함 상태 레지스터 값을 저장하는 변수 (비트 단위 에러 플래그 포함)
} stMotorDriverState;

extern stMotorDriverState xMotorDriver;

//---------------------------------------------------------------------------
// 함수 프로토타입
//---------------------------------------------------------------------------
/**
 * @brief      모터 드라이버(DRV8343) 상태 초기화 및 기본 레지스터 셋업
 * @param      void
 * @return     void
 */
extern void MotorDriver_Init(void);

/**
 * @brief      DRV8343 드라이버 내부의 결함 상태 레지스터 클리어 명령 전송
 * @param      void
 * @return     void
 */
extern void MotorDriver_ClearFaults(void);

/**
 * @brief      DRV8343의 결함 레지스터 상태를 주기적으로 읽어 구조체 업데이트
 * @param      void
 * @return     void
 */
extern void MotorDriver_UpdateStatus(void);

#ifdef __cplusplus
}
#endif

#endif /* CSU_MOTORDRIVER_H_ */
