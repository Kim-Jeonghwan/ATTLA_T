/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_LimitSwitch.c
    Version          : 00.00
    Description      : 리미트 스위치 상태 감지 및 고장 진단 모듈 구현
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 22. (최초 작성)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 22. - 파일 생성 및 기본 구조 작성
 */

#include "csu_LimitSwitch.h"

// 리미트 스위치 데이터 전역 인스턴스
stLimitSwitchConfig xLimitSwitchConfig;
stLimitSwitchState xLimitSwitch;

/**
 * @brief      리미트 스위치 관리 모듈 초기화 (임시 기본값 셋업)
 * @param      void
 * @return     void
 */
void LimitSwitch_Init(void)
{
    /* 
     * [사용자 리뷰 필요(권장 초기값 미정)]
     * 타겟 위치 및 범위(Tolerance)는 임의의 기본값입니다.
     * 나중에 올바른 파라미터가 결정되면 아래 값들을 수정해야 합니다. 
     */
    xLimitSwitchConfig.targetPos1 = 0.0f;           // 목표 1 임의 기본값
    xLimitSwitchConfig.targetPos2 = 15840.0f;       // 목표 2 임의 기본값 (기구부 최대 각도)
    xLimitSwitchConfig.nearTolerance1 = 100.0f;     // 목표 1 근접 판정 범위 임의 기본값
    xLimitSwitchConfig.nearTolerance2 = 100.0f;     // 목표 2 근접 판정 범위 임의 기본값
    
    // 1번 스위치를 Target 1에, 2번 스위치를 Target 2에 기본 매핑
    xLimitSwitchConfig.mappedSwitchForTarget1 = 1U;
    xLimitSwitchConfig.mappedSwitchForTarget2 = 2U;
    
    // 상태 초기화
    xLimitSwitch.isFaultActive = false;
    xLimitSwitch.faultCode = LS_FAULT_NONE;
}

/**
 * @brief      리미트 스위치 상태 주기적 점검 및 고장 판단 로직
 * @param      void
 * @return     void
 */
void LimitSwitch_CheckFaults(void)
{
    // 1. 물리적 단선 또는 고장 판단 (NO와 NC가 같으면 고장)
    if (xDio.limit1No == xDio.limit1Nc)
    {
        xLimitSwitch.isFaultActive = true;
        xLimitSwitch.faultCode = LS_FAULT_SW1_BROKEN;
        return; // 즉시 리턴하여 메인 제어 루프에서 고장 조치하도록 함
    }
    
    if (xDio.limit2No == xDio.limit2Nc)
    {
        xLimitSwitch.isFaultActive = true;
        xLimitSwitch.faultCode = LS_FAULT_SW2_BROKEN;
        return;
    }
    
    // 2. 현재 모터 위치를 기반으로 한 논리 연동 점검
    float32_t currentPos = xMotorCtrl.currentPosition;
    
    // ==========================================
    // Target 1 평가 로직
    // ==========================================
    float32_t diff1 = currentPos - xLimitSwitchConfig.targetPos1;
    bool isNearTarget1 = ((diff1 >= -xLimitSwitchConfig.nearTolerance1) && (diff1 <= xLimitSwitchConfig.nearTolerance1));
    uint16_t mappedSwNo1 = (xLimitSwitchConfig.mappedSwitchForTarget1 == 1U) ? xDio.limit1No : xDio.limit2No;
    
    if (isNearTarget1 && (mappedSwNo1 != 1U))
    {
        // 에러 조건: 목표 근처인데 매핑된 스위치 NO가 1이 아님
        xLimitSwitch.isFaultActive = true;
        xLimitSwitch.faultCode = LS_FAULT_POS1_MISMATCH;
        return;
    }
    if (!isNearTarget1 && (mappedSwNo1 == 1U))
    {
        // 에러 조건: 목표에서 충분히 벗어났는데 매핑된 스위치 NO가 1임
        xLimitSwitch.isFaultActive = true;
        xLimitSwitch.faultCode = LS_FAULT_POS1_MISMATCH;
        return;
    }
    
    // ==========================================
    // Target 2 평가 로직
    // ==========================================
    float32_t diff2 = currentPos - xLimitSwitchConfig.targetPos2;
    bool isNearTarget2 = ((diff2 >= -xLimitSwitchConfig.nearTolerance2) && (diff2 <= xLimitSwitchConfig.nearTolerance2));
    uint16_t mappedSwNo2 = (xLimitSwitchConfig.mappedSwitchForTarget2 == 2U) ? xDio.limit2No : xDio.limit1No;
    
    if (isNearTarget2 && (mappedSwNo2 != 1U))
    {
        // 에러 조건: 목표 근처인데 매핑된 스위치 NO가 1이 아님
        xLimitSwitch.isFaultActive = true;
        xLimitSwitch.faultCode = LS_FAULT_POS2_MISMATCH;
        return;
    }
    if (!isNearTarget2 && (mappedSwNo2 == 1U))
    {
        // 에러 조건: 목표에서 충분히 벗어났는데 매핑된 스위치 NO가 1임
        xLimitSwitch.isFaultActive = true;
        xLimitSwitch.faultCode = LS_FAULT_POS2_MISMATCH;
        return;
    }
    
    // ==========================================
    // 정상 상태
    // ==========================================
    xLimitSwitch.isFaultActive = false;
    xLimitSwitch.faultCode = LS_FAULT_NONE;
}
