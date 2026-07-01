/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_LimitSwitch.c
    Version          : 00.04
    Description      : 리미트 스위치 상태 감지 및 고장 진단 모듈 구현
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 07. 01. (초기화 구문 상세 한글 주석 추가)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 07. 01. - 초기화 구문 상세 한글 주석 추가 (코딩 규칙 적용)
 * 2026. 06. 30. - xDio 참조 변수명 리팩토링 (Active Low 표기 적용)
 * 2026. 06. 30. - Active Low(0U) 감지 기준으로 로직 전면 수정
 * 2026. 06. 30. - 리미트 데드존 및 오프셋 제한 로직 구현 (100us ISR 기준 딜레이)
 * 2026. 06. 22. - 파일 생성 및 기본 구조 작성
 */

#include "csu_LimitSwitch.h"

// 리미트 스위치 데이터 전역 인스턴스
stLimitSwitchConfig xLimitSwitchConfig;
stLimitSwitchState xLimitSwitch;
stLimitSwitchLimit xLimitSwitchLimit;

/**
 * @brief      리미트 스위치 관리 모듈 초기화 (기본 매핑 및 오프셋 초기화)
 * @param      void
 * @return     void
 */
void LimitSwitch_Init(void)
{
    // 양방향 매핑 스위치 기본 설정 (추후 TBD)
    xLimitSwitchConfig.mappedSwitchForPosDir = 1U; // 1번 스위치를 양의 방향(Positive) 제한용으로 매핑 할당
    xLimitSwitchConfig.mappedSwitchForNegDir = 2U; // 2번 스위치를 음의 방향(Negative) 제한용으로 매핑 할당
    
    // 상태 및 오프셋 초기화
    xLimitSwitch.isFaultActive = false;            // 고장 없음 상태로 초기화
    xLimitSwitch.faultCode = LS_FAULT_NONE;        // 고장 코드 없음으로 초기화
    
    xLimitSwitch.activeDirection = 0U;             // 감지된 방향 없음(0)으로 초기화
    xLimitSwitch.isLimitReached = false;           // 제한 거리 초과 도달 안함 상태로 초기화
    xLimitSwitch.limitBasePos = 0.0f;              // 최초 스위치 감지 기준 위치 0.0으로 초기화
    
    // 튜닝 파라미터 초기화
    xLimitSwitchLimit.offsetCount = 1000.0f;       // 리미트 감지 후 추가 진입 허용 거리를 1000.0으로 초기화
    xLimitSwitchLimit.deadzoneCount = 100.0f;      // 데드존 최소 탈출 거리를 100.0으로 초기화
    xLimitSwitchLimit.sensorErrorTimeMs = 50U;     // 센서 이상 진단 유지 시간을 50ms로 초기화
    xLimitSwitchLimit.sensorErrorTick100us = (50U * 10U); // 50ms를 100us 틱 카운트로 변환하여(500) 초기화
}

/**
 * @brief      리미트 스위치 상태 주기적 점검 및 고장/오프셋 판단 로직 (100us ISR 주기 호출)
 * @param      void
 * @return     void
 */
void LimitSwitch_CheckFaults(void)
{
    static uint16_t errorTickCount = 0U; // 100us 주기로 누적되는 에러 카운터
    
    // 1. 단선 및 동시 감지 상태 평가 (물리적 신호 결함 점검)
    bool isFaultCondition = false;
    LimitSwitchFaultCode_t currentFault = LS_FAULT_NONE;
    
    // 스위치 1 NO-NC 단선 여부 점검 (둘 다 1이거나 0일 경우 결함)
    if (xDio.nLimit1No == xDio.nLimit1Nc)
    {
        isFaultCondition = true;
        currentFault = LS_FAULT_SW1_BROKEN;
    }
    // 스위치 2 NO-NC 단선 여부 점검
    else if (xDio.nLimit2No == xDio.nLimit2Nc)
    {
        isFaultCondition = true;
        currentFault = LS_FAULT_SW2_BROKEN;
    }
    // 양방향 리미트 동시 감지 여부 점검 (물리적으로 불가능한 상황)
    else if ((xDio.nLimit1No == 0U) && (xDio.nLimit2No == 0U))
    {
        isFaultCondition = true;
        currentFault = LS_FAULT_SIMULTANEOUS;
    }

    // 에러 상태 유지 시간(SENSOR_ERROR_TIME_MS) 확인 (100us 루프 기준 SENSOR_ERROR_TICK_100US 사용)
    if (isFaultCondition == true)
    {
        errorTickCount++;
        if (errorTickCount >= xLimitSwitchLimit.sensorErrorTick100us)
        {
            xLimitSwitch.isFaultActive = true;
            xLimitSwitch.faultCode = currentFault;
            // 고장 시에는 리미트 오프셋 판단 로직을 무시하고 즉각적인 정지 조치가 취해지도록 리턴
            return; 
        }
    }
    else
    {
        // 정상 상태일 경우 카운터 및 폴트 초기화
        errorTickCount = 0U;
        xLimitSwitch.isFaultActive = false;
        xLimitSwitch.faultCode = LS_FAULT_NONE;
    }
    
    // 2. 오프셋 기반 제한 거리 판단 및 데드존 방어 로직
    float32_t currentPos = xMotorCtrl.currentPosition;
    
    // 현재 각 방향별 설정된 스위치 번호에 맞춰 NO 상태 획득
    uint16_t posSwNo = (xLimitSwitchConfig.mappedSwitchForPosDir == 1U) ? xDio.nLimit1No : xDio.nLimit2No;
    uint16_t negSwNo = (xLimitSwitchConfig.mappedSwitchForNegDir == 2U) ? xDio.nLimit2No : xDio.nLimit1No;
    
    // [Positive 방향 감지 시]
    if (posSwNo == 0U)
    {
        if (xLimitSwitch.activeDirection != 1U)
        {
            // 신규 진입 시 베이스 위치 갱신
            xLimitSwitch.activeDirection = 1U;
            xLimitSwitch.limitBasePos = currentPos;
            xLimitSwitch.isLimitReached = false;
        }
        else
        {
            // 이미 진입된 상태일 경우 설정된 LIMIT_OFFSET_COUNT 초과 진입 비교 (양수 방향)
            if ((currentPos - xLimitSwitch.limitBasePos) >= xLimitSwitchLimit.offsetCount)
            {
                xLimitSwitch.isLimitReached = true;
            }
            else
            {
                xLimitSwitch.isLimitReached = false;
            }
        }
    }
    // [Negative 방향 감지 시]
    else if (negSwNo == 0U)
    {
        if (xLimitSwitch.activeDirection != 2U)
        {
            // 신규 진입 시 베이스 위치 갱신
            xLimitSwitch.activeDirection = 2U;
            xLimitSwitch.limitBasePos = currentPos;
            xLimitSwitch.isLimitReached = false;
        }
        else
        {
            // 이미 진입된 상태일 경우 설정된 LIMIT_OFFSET_COUNT 초과 진입 비교 (음의 방향 이동이므로 기준위치 - 현재위치)
            if ((xLimitSwitch.limitBasePos - currentPos) >= xLimitSwitchLimit.offsetCount)
            {
                xLimitSwitch.isLimitReached = true;
            }
            else
            {
                xLimitSwitch.isLimitReached = false;
            }
        }
    }
    // [스위치 해제 상태]
    else
    {
        // 데드존 방지 로직 적용: 진동에 의한 스위치 찰나의 해제 현상을 무시
        if (xLimitSwitch.activeDirection == 1U)
        {
            // Positive 방향에서 탈출하는 상황 (현재 위치가 감소해야 함)
            if ((xLimitSwitch.limitBasePos - currentPos) >= xLimitSwitchLimit.deadzoneCount)
            {
                xLimitSwitch.activeDirection = 0U;
                xLimitSwitch.isLimitReached = false;
            }
        }
        else if (xLimitSwitch.activeDirection == 2U)
        {
            // Negative 방향에서 탈출하는 상황 (현재 위치가 증가해야 함)
            if ((currentPos - xLimitSwitch.limitBasePos) >= xLimitSwitchLimit.deadzoneCount)
            {
                xLimitSwitch.activeDirection = 0U;
                xLimitSwitch.isLimitReached = false;
            }
        }
        else
        {
            // 완전히 벗어난 평시 상태
            xLimitSwitch.activeDirection = 0U;
            xLimitSwitch.isLimitReached = false;
        }
    }
}
