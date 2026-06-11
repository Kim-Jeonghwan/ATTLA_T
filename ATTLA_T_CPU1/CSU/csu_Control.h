/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Control.h
    Version          : 00.02
    Description      : 시스템 제어 모듈 (PBIT, CBIT, 오프셋 조정 등) 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 11. (주석 표준화 및 레거시 코드 정리)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 상태 변수들을 stControlState 구조체(xSysCtrl)로 통합
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 * 2026. 06. 11. - 함수명 접두어(csu_, hal_) 제거 리팩토링
 */

#ifndef CSU_CONTROL_H
#define CSU_CONTROL_H

#include "main.h"

typedef struct {
    uint16_t isOffsetCalibrated;
    uint16_t isPbitComplete;
} stControlState;

extern volatile stControlState xSysCtrl;

/**
 * @brief      시스템 제어 모듈 상태 구조체 초기화
 * @param      void
 * @return     void
 */
void Control_Init(void);

/**
 * @brief      전류 센서 오프셋 영점 조정 (PWM ISR 호출용)
 * @param      void
 * @return     void
 */
void Control_CalibrateCurrentOffset(void);

/**
 * @brief      초기 점검 (PBIT) 수행 (PWM ISR 호출용)
 * @param      void
 * @return     void
 */
void Bit_RunPBIT(void);

/**
 * @brief      주기 점검 (CBIT) 수행
 * @param      void
 * @return     void
 */
void Bit_RunCBIT(void);

/**
 * @brief      100us PWM 인터럽트 기반 시스템 운용 파이프라인
 * @param      void
 * @return     void
 */
void Control_SystemOperation(void);

#endif // CSU_CONTROL_H
