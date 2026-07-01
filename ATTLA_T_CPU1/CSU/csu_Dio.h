/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Dio.h
    Version          : 00.05
    Description      : 이산신호(DIO) 입력 처리 및 디바운싱 필터 모듈 (CSU)
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 07. 01. (구조체 변수 상세 한글 주석 추가)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 07. 01. - 구조체 변수 상세 한글 주석 추가 (코딩 규칙 적용)
 * 2026. 06. 30. - 시스템 상태(LednNormal, LednFault) 반영을 위한 Dio_UpdateOutput 함수 원형 추가
 * 2026. 06. 30. - stDioState 멤버 변수명 리팩토링 (Active Low 표기 적용)
 * 2026. 06. 23. - main.h -> main_cpu1.h 인클루드 명칭 리팩토링
 * 2026. 06. 12. - 매크로 상수명 추상화 (DIO_CNT_DEBOUNCE_REF) 및 주석 보강
 * 2026. 06. 12. - 파일 생성 및 기본 구조 작성
 * 2026. 06. 12. - 홀센서(Hall A, B, C) 입력 변수 추가
 */

#ifndef CSU_DIO_H
#define CSU_DIO_H

#include "main_cpu1.h"

#define DIO_CNT_DEBOUNCE_REF   10U    // 1ms 신호 입력 식별 디바운싱 카운트 (10kHz 100us 기준)

typedef struct {
    uint16_t nLimit1No;      // 리미트 스위치 1번 NO (Normally Open) 접점 입력 상태 (Active Low)
    uint16_t nLimit1Nc;      // 리미트 스위치 1번 NC (Normally Closed) 접점 입력 상태 (Active Low)
    uint16_t nLimit2No;      // 리미트 스위치 2번 NO (Normally Open) 접점 입력 상태 (Active Low)
    uint16_t nLimit2Nc;      // 리미트 스위치 2번 NC (Normally Closed) 접점 입력 상태 (Active Low)
    uint16_t Pmn24V;         // 브레이크 전원 24V 이상 유무 감지 입력 (Active Low)
    uint16_t nCableLoop;     // 외부 통신/전원 케이블 연결 상태 감지 루프백 입력 (Active Low)
    uint16_t DrvnFault;      // DRV8343 모터 드라이버 하드웨어 결함(nFAULT) 상태 입력 (Active Low)
    uint16_t nHallA;         // 모터 홀센서 A상 입력 상태 (Active Low)
    uint16_t nHallB;         // 모터 홀센서 B상 입력 상태 (Active Low)
    uint16_t nHallC;         // 모터 홀센서 C상 입력 상태 (Active Low)
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

/**
 * @brief      이산신호 출력 처리 (시스템 상태에 따른 외부 LED 등 제어)
 * @param      void
 * @return     void
 */
void Dio_UpdateOutput(void);

#endif // CSU_DIO_H
