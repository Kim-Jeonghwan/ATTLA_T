/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Dio.c
    Version          : 00.06
    Description      : 이산신호(DIO) 입력 처리 및 디바운싱 필터 모듈 (CSU)
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 07. 01. (초기화 구문 상세 한글 주석 추가)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 07. 01. - 초기화 구문 상세 한글 주석 추가 (코딩 규칙 적용)
 * 2026. 06. 30. - 시스템 상태 연동 DIO 출력 제어 (Dio_UpdateOutput) 함수 추가
 * 2026. 06. 30. - 디바운싱 내부 카운터(cnt_) 변수명 리팩토링 적용
 * 2026. 06. 30. - stDioState 멤버 변수명 리팩토링 (Active Low 표기 적용)
 * 2026. 06. 12. - 매크로 상수명 추상화 (DIO_CNT_DEBOUNCE_REF) 적용
 * 2026. 06. 12. - 파일 생성 및 디바운싱 로직 구현
 * 2026. 06. 12. - 홀센서(Hall A, B, C) 핀 입력 스캔 및 디바운싱 추가
 */

#include "csu_Dio.h"
#include "csu_Bit.h" // 상태(xBit.informAll) 조회를 위한 포함

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
    xDio.nLimit1No = 1U;      // 리미트 스위치 1번 NO 기본값 초기화
    xDio.nLimit1Nc = 1U;      // 리미트 스위치 1번 NC 기본값 초기화
    xDio.nLimit2No = 1U;      // 리미트 스위치 2번 NO 기본값 초기화
    xDio.nLimit2Nc = 1U;      // 리미트 스위치 2번 NC 기본값 초기화
    xDio.Pmn24V = 1U;         // 브레이크 전압 이상 유무 기본값 초기화
    xDio.nCableLoop = 1U;     // 케이블 루프백 기본값 초기화
    xDio.DrvnFault = 1U;      // 드라이버 결함 핀 기본값 초기화
    xDio.nHallA = 1U;         // 홀센서 A상 기본값 초기화
    xDio.nHallB = 1U;         // 홀센서 B상 기본값 초기화
    xDio.nHallC = 1U;         // 홀센서 C상 기본값 초기화
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
    // 각 핀의 이전 상태 유지를 위한 정적 카운터 변수들
    static uint16_t cnt_nLimit1No = 0U;
    static uint16_t cnt_nLimit1Nc = 0U;
    static uint16_t cnt_nLimit2No = 0U;
    static uint16_t cnt_nLimit2Nc = 0U;
    static uint16_t cnt_Pmn24V = 0U;
    static uint16_t cnt_nCableLoop = 0U;
    static uint16_t cnt_DrvnFault = 0U;
    static uint16_t cnt_nHallA = 0U;
    static uint16_t cnt_nHallB = 0U;
    static uint16_t cnt_nHallC = 0U;

    // GPIO 포트에서 실제 값을 읽어 디바운싱 처리 후 상태 구조체에 반영
    
    // 리미트 스위치
    Dio_Debounce(GPIO_readPin(36U), &xDio.nLimit1No, &cnt_nLimit1No);
    Dio_Debounce(GPIO_readPin(37U), &xDio.nLimit1Nc, &cnt_nLimit1Nc);
    
    Dio_Debounce(GPIO_readPin(38U), &xDio.nLimit2No, &cnt_nLimit2No);
    Dio_Debounce(GPIO_readPin(39U), &xDio.nLimit2Nc, &cnt_nLimit2Nc);
    
    // 시스템 감시
    Dio_Debounce(GPIO_readPin(40U), &xDio.Pmn24V, &cnt_Pmn24V);
    Dio_Debounce(GPIO_readPin(46U), &xDio.nCableLoop, &cnt_nCableLoop);
    Dio_Debounce(GPIO_readPin(10U), &xDio.DrvnFault, &cnt_DrvnFault);
    
    // 모터 홀센서
    Dio_Debounce(GPIO_readPin(11U), &xDio.nHallA, &cnt_nHallA);
    Dio_Debounce(GPIO_readPin(12U), &xDio.nHallB, &cnt_nHallB);
    Dio_Debounce(GPIO_readPin(13U), &xDio.nHallC, &cnt_nHallC);
}

/**
 * @brief      이산신호 출력 처리 (시스템 상태에 따른 외부 LED 등 제어)
 * @param      void
 * @return     void
 */
void Dio_UpdateOutput(void)
{
    // PBIT, CBIT, IBIT 결과가 반영된 xBit.informAll 확인
    if (xBit.informAll == 0U)
    {
        // 정상 상태: LednNormal(GPIO31) 출력 0(켜짐), LednFault(GPIO32) 출력 1(꺼짐)
        GPIO_writePin(31U, 0U);
        GPIO_writePin(32U, 1U);
    }
    else
    {
        // 비정상 상태: LednNormal(GPIO31) 출력 1(꺼짐), LednFault(GPIO32) 출력 0(켜짐)
        GPIO_writePin(31U, 1U);
        GPIO_writePin(32U, 0U);
    }
}

