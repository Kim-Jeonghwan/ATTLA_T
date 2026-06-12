/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Dio.c
    Version          : 00.01
    Description      : 이산신호(DIO) 입력 처리 및 디바운싱 필터 모듈 (CSU)
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 12. (매크로 상수명 DIO_CNT_DEBOUNCE_REF 반영)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 12. - 매크로 상수명 추상화 (DIO_CNT_DEBOUNCE_REF) 적용
 * 2026. 06. 12. - 파일 생성 및 디바운싱 로직 구현
 * 2026. 06. 12. - 홀센서(Hall A, B, C) 핀 입력 스캔 및 디바운싱 추가
 */

#include "csu_Dio.h"

volatile stDioState xDio;

#pragma CODE_SECTION(Dio_UpdateInput, ".TI.ramfunc");

/**
 * @brief      이산신호(DIO) 상태 구조체 초기화
 * @param      void
 * @return     void
 */
void Dio_Init(void)
{
    // 구조체 변수 명시적 초기화 (기본 High 상태로 초기화)
    xDio.limit1No = 1U;
    xDio.limit1Nc = 1U;
    xDio.limit2No = 1U;
    xDio.limit2Nc = 1U;
    xDio.pm24V = 1U;
    xDio.cableLoop = 1U;
    xDio.drvFault = 1U;
    xDio.hallA = 1U;
    xDio.hallB = 1U;
    xDio.hallC = 1U;
}

/**
 * @brief      핀 입력을 디바운싱하여 안정된 상태를 반환 (대칭형 필터)
 * @param      rawValue       : 현재 핀에서 읽은 실제 값
 * @param      pFilteredValue : 필터링된 상태를 저장할 변수의 포인터
 * @param      pCount         : 디바운싱 누적 카운터를 저장할 변수의 포인터
 * @return     uint16_t       : 갱신된(또는 유지된) 필터링 값
 */
static uint16_t Dio_Debounce(uint16_t rawValue, volatile uint16_t* pFilteredValue, uint16_t* pCount)
{
    if (rawValue != *pFilteredValue)
    {
        (*pCount)++;
        if (*pCount >= DIO_CNT_DEBOUNCE_REF)
        {
            *pFilteredValue = rawValue;
            *pCount = 0U;
        }
    }
    else
    {
        *pCount = 0U;
    }
    return *pFilteredValue;
}

/**
 * @brief      이산신호 입력 상태 폴링 및 디바운싱 처리 (100us 주기 호출용)
 * @param      void
 * @return     void
 */
void Dio_UpdateInput(void)
{
    static uint16_t cnt_limit1No = 0U;
    static uint16_t cnt_limit1Nc = 0U;
    static uint16_t cnt_limit2No = 0U;
    static uint16_t cnt_limit2Nc = 0U;
    static uint16_t cnt_pm24V = 0U;
    static uint16_t cnt_cableLoop = 0U;
    static uint16_t cnt_drvFault = 0U;
    static uint16_t cnt_hallA = 0U;
    static uint16_t cnt_hallB = 0U;
    static uint16_t cnt_hallC = 0U;

    // 리미트 스위치 1
    Dio_Debounce(GPIO_readPin(36U), &xDio.limit1No, &cnt_limit1No);
    Dio_Debounce(GPIO_readPin(37U), &xDio.limit1Nc, &cnt_limit1Nc);

    // 리미트 스위치 2
    Dio_Debounce(GPIO_readPin(38U), &xDio.limit2No, &cnt_limit2No);
    Dio_Debounce(GPIO_readPin(39U), &xDio.limit2Nc, &cnt_limit2Nc);

    // 시스템 및 전원 감시
    Dio_Debounce(GPIO_readPin(40U), &xDio.pm24V, &cnt_pm24V);
    Dio_Debounce(GPIO_readPin(46U), &xDio.cableLoop, &cnt_cableLoop);
    Dio_Debounce(GPIO_readPin(10U), &xDio.drvFault, &cnt_drvFault);

    // 홀센서 (모니터링용 병렬 입력)
    Dio_Debounce(GPIO_readPin(11U), &xDio.hallA, &cnt_hallA);
    Dio_Debounce(GPIO_readPin(12U), &xDio.hallB, &cnt_hallB);
    Dio_Debounce(GPIO_readPin(13U), &xDio.hallC, &cnt_hallC);
}
