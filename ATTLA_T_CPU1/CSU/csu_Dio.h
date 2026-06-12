/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Dio.h
    Version          : 00.01
    Description      : 이산신호(DIO) 입력 처리 및 디바운싱 필터 모듈 (CSU)
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 12. (매크로 상수명 추상화: DIO_CNT_DEBOUNCE_REF)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 12. - 매크로 상수명 추상화 (DIO_CNT_DEBOUNCE_REF) 및 주석 보강
 * 2026. 06. 12. - 파일 생성 및 기본 구조 작성
 * 2026. 06. 12. - 홀센서(Hall A, B, C) 입력 변수 추가
 */

#ifndef CSU_DIO_H
#define CSU_DIO_H

#include "main.h"

#define DIO_CNT_DEBOUNCE_REF   10U    // 1ms 신호 입력 식별 디바운싱 카운트 (10kHz 100us 기준)

typedef struct {
    uint16_t limit1No;
    uint16_t limit1Nc;
    uint16_t limit2No;
    uint16_t limit2Nc;
    uint16_t pm24V;
    uint16_t cableLoop;
    uint16_t drvFault;
    uint16_t hallA;
    uint16_t hallB;
    uint16_t hallC;
} stDioState;

extern volatile stDioState xDio;

/**
 * @brief      이산신호(DIO) 상태 구조체 초기화
 * @param      void
 * @return     void
 */
void Dio_Init(void);

/**
 * @brief      이산신호 입력 상태 폴링 및 디바운싱 처리 (100us 주기 호출용)
 * @param      void
 * @return     void
 */
void Dio_UpdateInput(void);

#endif // CSU_DIO_H
