/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Control.h
    Version          : 00.07
    Description      : 시스템 제어 모듈 (PBIT, CBIT, 오프셋 조정 등) 헤더
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 12. (매크로 상수명 추상화: ADC_SCALE_REF_VOLT)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 12. - 매크로 상수명 추상화: ADC_SCALE_REF_VOLT
 * 2026. 06. 12. - 오프셋 보정 변수 및 ADC 상수 헤더(.h)로 이동 (글로벌 룰 적용)
 * 2026. 06. 12. - DIO 디바운싱 필터 및 FRAM 저장 래퍼 연동
 * 2026. 06. 12. - 3단계 동적 인터럽트 ISR 선언 추가 (csu_Offset_Isr, csu_Pbit_Isr, csu_MainControl_Isr)
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 상태 변수들을 stControlState 구조체(xSysCtrl)로 통합
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 * 2026. 06. 11. - 함수명 접두어(csu_, hal_) 제거 리팩토링
 */

#ifndef CSU_CONTROL_H
#define CSU_CONTROL_H

#include "main.h"

#define ADC_SCALE_REF_VOLT (3.0f / 4096.0f) // 3V 레퍼런스 기준 변환 상수

typedef struct {
    uint16_t isOffsetCalibrated;
    uint16_t isPbitComplete;
    uint16_t offsetCount;
    float32_t sumMot;
    float32_t sumBrk;
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

/**
 * @brief      전류 오프셋을 FRAM에 저장 (초기화 완료 시 1회 호출)
 * @param      void
 * @return     void
 */
void Control_SaveOffsetToFram(void);

/**
 * @brief      주기적인 데이터 FRAM 저장 래퍼 함수 (미정)
 * @param      void
 * @return     void
 */
void Control_SaveDataToFram(void);

// 시스템 제어 시퀀스 ISR 체인 (EPWM1 인터럽트 발생)
__interrupt void csu_Offset_Isr(void);
__interrupt void csu_Pbit_Isr(void);
__interrupt void csu_MainControl_Isr(void);

#endif // CSU_CONTROL_H
