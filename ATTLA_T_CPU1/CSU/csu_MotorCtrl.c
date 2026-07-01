/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_MotorCtrl.c
    Version          : 00.16
    Description      : 1x PWM 모드 기반 모터 제어 모듈
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 07. 01. (speedPid 구조체 멤버명 오타 수정)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 07. 01. - speedPid 구조체 멤버명(maxOut, minOut -> maxOutput, minOutput) 오타 수정
 * 2026. 06. 30. - 엔코더 영점 설정 안전 인터락 타이머(safeToZeroTimerTick) 로직 및 판단 함수 추가
 * 2026. 06. 30. - 전자식 브레이크 지연 시퀀스 (기동/정지 100us ISR 기준 딜레이) 추가
 * 2026. 06. 30. - 리미트 스위치 감지 시 전류 한도 동적 조정 및 차단 로직 구현
 * 2026. 06. 23. - 코딩 규칙 및 구조 불일치 사항 리팩토링 반영
 * 2026. 06. 22. - 전류 제어 루프 절댓값 로직 제거 및 4상한 제어 개선
 * 2026. 06. 22. - 위치 제어 루프 변수명 및 주석의 특정 주기(4ms/5ms) 표기 제거
 * 2026. 06. 22. - BIT 결함 플래그(xBit.faultFlagSet) 기반 모터 Fail-Safe 정지 로직 추가
 * 2026. 06. 22. - 리미트 스위치 감지 호출 및 고장 시 정지 로직 추가
 * 2026. 06. 22. - PID 계수를 xPidGain 구조체로 묶어 관리하도록 변경
 * 2026. 06. 22. - PID 파라미터 전역 변수 초기화 및 제어 루프 내 실시간 반영 로직 추가
 * 2026. 06. 22. - 속도 제어기 PI-IP 혼합 계수(Ks) 초기화 연동
 * 2026. 06. 16. - 브레이크 핀 (Active High) 정방향 제어 로직 모드에 연동 적용
 * 2026. 06. 12. - 10U 및 4U 매직넘버를 DECIMATION_SPEED_CTRL, DECIMATION_POS_CTRL 매크로로 치환
 * 2026. 06. 12. - PID 파라미터, 모터 한도, 스케일 상수 헤더(.h)로 이동 (글로벌 룰 적용)
 * 2026. 06. 11. - 위치 제한 클램핑 로직 및 속도/전류 PID 한계치(Soft Limit) 적용
 * 2026. 06. 11. - 위치 제어기를 4ms 루프로 추가 분리하여 기구적 대역폭 안정성 확보
 * 2026. 06. 11. - 속도 및 위치 제어기 1ms 루프 분리 (Multi-Rate 캐스케이드 구조 개선)
 * 2026. 06. 11. - 전류 크기(Magnitude) 제어기(currPid) 추가 및 Cascade 제어루프 편입
 * 2026. 06. 11. - 1ms Decimation 속도 연산 적용 (계측 노이즈 저감)
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 상태 변수들을 stMotorCtrlState 구조체(xMotorCtrl)로 통합
 * 2026. 06. 11. - Driverlib 직접 호출 제거 및 HAL 추상화, Include 정리
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */


#include "csu_MotorCtrl.h"

// --- 실시간 튜닝용 전역 변수 초기화 ---
stPidGain xPidGain = {
    { 1.0f, 0.0f, 0.0f, 1.0f },     // pos (Kp, Ki, Kd, Ks)
    { 0.5f, 0.01f, 0.0f, 0.25f },   // spd (Kp, Ki, Kd, Ks)
    { 2.0f, 0.05f, 0.0f, 1.0f }     // curr (Kp, Ki, Kd, Ks)
};
// -------------------------------------

// 모터 제어 모듈의 전체 상태
stMotorCtrlState xMotorCtrl;
stMotorCtrlLimit xMotorCtrlLimit;

// PID 제어기 인스턴스
PID_Controller_t currPid;
PID_Controller_t speedPid;
PID_Controller_t posPid;


/*
@function    void MotorCtrl_Init(void)
@brief      모터 제어기 초기화
@param      void
@return     void
*/
void MotorCtrl_Init(void)
{
    // 구조체 명시적 초기화
    xMotorCtrl.mode = MOTOR_MODE_STOP;
    xMotorCtrl.targetSpeedRpm = 0.0f;
    xMotorCtrl.targetPosition = 0.0f;
    xMotorCtrl.currentSpeedRpm = 0.0f;
    xMotorCtrl.currentPosition = 0.0f;
    
    // 브레이크 초기 상태
    xMotorCtrl.brakeState = BRAKE_STATE_LOCKED;
    xMotorCtrl.brakeTimerTick = 0U;
    xMotorCtrl.safeToZeroTimerTick = 0U;

    // 튜닝 파라미터 초기화
    xMotorCtrlLimit.posMin = 0.0f;
    xMotorCtrlLimit.posMax = 15840.0f;
    xMotorCtrlLimit.speedMax = 3240.0f;
    xMotorCtrlLimit.speedMin = -3240.0f;
    xMotorCtrlLimit.currentMax = 9.34f;
    xMotorCtrlLimit.currentMin = -9.34f;
    xMotorCtrlLimit.currentRatio = 0.25f;
    xMotorCtrlLimit.brakeReleaseDelayMs = 150U;
    xMotorCtrlLimit.brakeEngageDelayMs = 100U;
    xMotorCtrlLimit.brakeReleaseTick100us = (150U * 10U);
    xMotorCtrlLimit.brakeEngageTick100us = (100U * 10U);
    xMotorCtrlLimit.safeZeroSetTick100us = 5000U;

    // 하위 Driver 상태 초기화
    MotorDriver_Init();
    
    // PID 초기화 (Kp, Ki, Kd, Ks, dt, max_out, min_out)
    // 전류 제어기는 100us (0.0001s) 루프에서 동작 (표준 PI)
    PID_Init(&currPid, xPidGain.curr.Kp, xPidGain.curr.Ki, xPidGain.curr.Kd, xPidGain.curr.Ks, PID_CURR_DT, MOTOR_DUTY_MAX, -MOTOR_DUTY_MAX);     // Current 출력은 부호가 포함된 Duty
    
    // 속도 제어기는 속도 루프 주기(PID_SPD_DT)에서 동작 (PI-IP 혼합 제어)
    PID_Init(&speedPid, xPidGain.spd.Kp, xPidGain.spd.Ki, xPidGain.spd.Kd, xPidGain.spd.Ks, PID_SPD_DT, xMotorCtrlLimit.currentMax, xMotorCtrlLimit.currentMin);    // Speed 출력은 타겟 전류량 (최대 ±10.0A)
    
    // 위치 제어기는 기계적 관성을 고려하여 위치 루프 주기(PID_POS_DT)에서 동작 (표준 PD)
    PID_Init(&posPid, xPidGain.pos.Kp, xPidGain.pos.Ki, xPidGain.pos.Kd, xPidGain.pos.Ks, PID_POS_DT, xMotorCtrlLimit.speedMax, xMotorCtrlLimit.speedMin);           // Position 출력은 Speed 명령
    
    // 방향 핀(INHC)은 hal_DspInit.c 의 Init_GpioDout() 에서 이미 초기화 완료됨
}

/*
@function    void MotorCtrl_UpdateFeedback(void)
@brief      엔코더 위치 및 속도 피드백 갱신
@param      void
@return     void
*/
void MotorCtrl_UpdateFeedback(void)
{
    // csu_Encoder 에서 읽어온 34비트 엔코더 데이터 사용
    // 기계각 스케일 환산 (상수 매크로 적용)
    xMotorCtrl.currentPosition = (float32_t)xEncoder.position * MOTOR_SCALE_POS_DEG; 
    
    // 속도 계산 (1ms 분주(Decimation) 방식 적용, 이산 오차 최소화)
    static Uint16 speedCalcCnt = 0U;
    static float32_t prevPos = 0.0f;
    
    speedCalcCnt++;
    if (speedCalcCnt >= DECIMATION_SPEED_CTRL)
    {
        float32_t posDiff = xMotorCtrl.currentPosition - prevPos;
        
        // 단순 미분을 통한 RPM 계산 (1ms 주기 기준)
        xMotorCtrl.currentSpeedRpm = posDiff * MOTOR_SCALE_SPEED_RPM;
        prevPos = xMotorCtrl.currentPosition;
        
        speedCalcCnt = 0U;
    }
}

/*
@function    void MotorCtrl_SetOutput(float32_t outputDuty)
@brief      1x PWM Duty 및 방향 설정
@param      outputDuty: -100.0 ~ 100.0 (%)
@return     void
*/
void MotorCtrl_SetOutput(float32_t outputDuty)
{
    if (outputDuty > 0.0f)
    {
        // 정방향
        MotorDriver_SetDir(true);
        if (outputDuty > MOTOR_DUTY_MAX) outputDuty = MOTOR_DUTY_MAX;
    }
    else
    {
        // 역방향
        MotorDriver_SetDir(false);
        outputDuty = -outputDuty;
        if (outputDuty > MOTOR_DUTY_MAX) outputDuty = MOTOR_DUTY_MAX;
    }

    // 추상화된 HAL 함수 호출로 PWM Duty 적용 및 SW Force 제어 위임
    Epwm_SetMotorDuty_1x(outputDuty);
}

/*
@function    void MotorCtrl_Run(void)
@brief      100us 주기 모터 제어 메인 루틴
@param      void
@return     void
*/
void MotorCtrl_Run(void)
{
    MotorCtrl_UpdateFeedback(); 
    
    // 엔코더 영점 설정 안전 인터락 타이머 갱신 (100us)
    if ((xMotorCtrl.currentSpeedRpm >= -1.0f) && (xMotorCtrl.currentSpeedRpm <= 1.0f) && 
        (xMotorCtrl.brakeState == BRAKE_STATE_LOCKED))
    {
        if (xMotorCtrl.safeToZeroTimerTick < xMotorCtrlLimit.safeZeroSetTick100us)
        {
            xMotorCtrl.safeToZeroTimerTick++;
        }
    }
    else
    {
        xMotorCtrl.safeToZeroTimerTick = 0U;
    }
    
    // 리미트 스위치 상태 점검 및 고장 판단
    LimitSwitch_CheckFaults();
    if (xLimitSwitch.isFaultActive == true)
    {
        // 고장 감지 시 즉시 정지 모드로 강제 전환
        xMotorCtrl.mode = MOTOR_MODE_FAULT_STOP;
    }

    // BIT 결함 플래그 상태 점검 및 고장 판단 (과전류, 과열, 과전압, Stall, 과속 등)
    if (xBit.faultFlagSet == 1U)
    {
        // 결함 감지 시 즉시 정지 모드로 강제 전환
        xMotorCtrl.mode = MOTOR_MODE_FAULT_STOP;
    }
    
    // ==============================================================
    // 1. 브레이크 상태 전이 판단 (State Machine)
    // ==============================================================
    if (xMotorCtrl.mode == MOTOR_MODE_FAULT_STOP)
    {
        xMotorCtrl.brakeState = BRAKE_STATE_LOCKED;
    }
    else if (xMotorCtrl.mode == MOTOR_MODE_STOP)
    {
        if (xMotorCtrl.brakeState == BRAKE_STATE_FREE)
        {
            // 정지 명령: 실제 속도가 거의 0에 수렴하면 체결 시퀀스 진입 (기계적 한계 고려)
            if ((xMotorCtrl.currentSpeedRpm > -1.0f) && (xMotorCtrl.currentSpeedRpm < 1.0f))
            {
                xMotorCtrl.brakeState = BRAKE_STATE_ENGAGING;
                xMotorCtrl.brakeTimerTick = 0U;
            }
            else
            {
                // 아직 속도가 있다면 감속을 위해 0 지령 유지
                xMotorCtrl.targetSpeedRpm = 0.0f;
                xMotorCtrl.targetPosition = xMotorCtrl.currentPosition;
            }
        }
    }
    else // MOTOR_MODE_SPEED_CTRL or MOTOR_MODE_POS_CTRL
    {
        if (xMotorCtrl.brakeState == BRAKE_STATE_LOCKED)
        {
            // 잠김 상태에서 기동 시 브레이크 해제 시퀀스(RELEASING) 시작
            xMotorCtrl.brakeState = BRAKE_STATE_RELEASING;
            xMotorCtrl.brakeTimerTick = 0U;
            
            // 토크 유지를 위해 현재 위치로 타겟 강제 정렬 (튐 현상 방지)
            xMotorCtrl.targetPosition = xMotorCtrl.currentPosition;
            xMotorCtrl.targetSpeedRpm = 0.0f;
        }
    }

    // ==============================================================
    // 2. 브레이크 상태에 따른 출력 및 타이머 처리
    // ==============================================================
    if (xMotorCtrl.brakeState == BRAKE_STATE_LOCKED)
    {
        // 브레이크 잠금 및 PID 완전 정지 (Servo Off)
        GPIO_writePin(GPIO_PIN_MOTOR_BRAKE, 0U); 
        MotorCtrl_SetOutput(0.0f);
        currPid.integral = 0.0f;
        speedPid.integral = 0.0f;
        posPid.integral = 0.0f;
        return; // PID 연산 생략 (안전성)
    }
    else if (xMotorCtrl.brakeState == BRAKE_STATE_RELEASING)
    {
        GPIO_writePin(GPIO_PIN_MOTOR_BRAKE, 1U); // 브레이크 해제 신호 출력
        xMotorCtrl.brakeTimerTick++;
        
        if (xMotorCtrl.brakeTimerTick >= xMotorCtrlLimit.brakeReleaseTick100us)
        {
            // 딜레이 경과 후 기계적 해제 완료로 간주, 완전 제어(FREE) 상태로 전이
            xMotorCtrl.brakeState = BRAKE_STATE_FREE;
        }
        else
        {
            // 딜레이 중에는 하중을 버티도록(Torque Pre-charge) 지령 고정
            xMotorCtrl.targetSpeedRpm = 0.0f;
            xMotorCtrl.targetPosition = xMotorCtrl.currentPosition;
        }
    }
    else if (xMotorCtrl.brakeState == BRAKE_STATE_ENGAGING)
    {
        // 정지 완료 시 체결 신호 출력 후 대기
        GPIO_writePin(GPIO_PIN_MOTOR_BRAKE, 0U); 
        xMotorCtrl.targetSpeedRpm = 0.0f;
        xMotorCtrl.targetPosition = xMotorCtrl.currentPosition;
        
        xMotorCtrl.brakeTimerTick++;
        if (xMotorCtrl.brakeTimerTick >= xMotorCtrlLimit.brakeEngageTick100us)
        {
            // 대기 완료 후 물리적 체결 완료 간주, 다음 주기부터 PID 차단
            xMotorCtrl.brakeState = BRAKE_STATE_LOCKED;
            return;
        }
    }
    else // BRAKE_STATE_FREE
    {
        // 정상 제어 중 브레이크 해제 유지
        GPIO_writePin(GPIO_PIN_MOTOR_BRAKE, 1U); 
    }
    
    // ==============================================================
    // 3. 정상 PID 연산 영역 (RELEASING, FREE, ENGAGING 에서 실행됨)
    // ==============================================================
    // 전역 변수 튜닝 파라미터 실시간 반영
    posPid.Kp = xPidGain.pos.Kp;
    posPid.Kd = xPidGain.pos.Kd;
    
    speedPid.Kp = xPidGain.spd.Kp;
    speedPid.Ki = xPidGain.spd.Ki;
    speedPid.Ks = xPidGain.spd.Ks;
    
    currPid.Kp = xPidGain.curr.Kp;
    currPid.Ki = xPidGain.curr.Ki;

    // --- 리미트 스위치 감지 상태에 따른 구속 전류 동적 제한 로직 ---
    float32_t activeMaxCurrent = xMotorCtrlLimit.currentMax;
    float32_t activeMinCurrent = xMotorCtrlLimit.currentMin;
    
    if (xLimitSwitch.isLimitReached == true)
    {
        // 오프셋 차단 거리 도달: 해당 방향의 전류(토크) 완전 차단 (0.0f)
        if (xLimitSwitch.activeDirection == 1U)
        {
            activeMaxCurrent = 0.0f; 
        }
        else if (xLimitSwitch.activeDirection == 2U)
        {
            activeMinCurrent = 0.0f;
        }
    }
    else if (xLimitSwitch.activeDirection != 0U)
    {
        // 스위치 감지 후 제한 거리로 이동 중: 진입 방향 전류를 비율로 억제 (충돌 방지)
        if (xLimitSwitch.activeDirection == 1U)
        {
            activeMaxCurrent = xMotorCtrlLimit.currentMax * xMotorCtrlLimit.currentRatio;
        }
        else if (xLimitSwitch.activeDirection == 2U)
        {
            activeMinCurrent = xMotorCtrlLimit.currentMin * xMotorCtrlLimit.currentRatio;
        }
    }
    
    // 속도 제어기(출력이 전류 지령임)의 상하한계 실시간 적용
    speedPid.maxOutput = activeMaxCurrent;
    speedPid.minOutput = activeMinCurrent;
    // -------------------------------------------------------------

    static float32_t currentCmd = 0.0f;
    static float32_t speedCmd = 0.0f;
    static Uint16 loop1msCnt = 0U;
    static Uint16 loopPosCtrlDivider = 0U;
    
    loop1msCnt++;
    if (loop1msCnt >= DECIMATION_SPEED_CTRL)
    {
        // [위치 제어 루프] 최외곽 루프: 위치 제어 연산 (5ms)
        loopPosCtrlDivider++;
        if (loopPosCtrlDivider >= DECIMATION_POS_CTRL)
        {
            if (xMotorCtrl.mode == MOTOR_MODE_POS_CTRL)
            {
                // 위치 명령 기구 리미트 범위 클램핑 적용
                xMotorCtrl.targetPosition = CLAMP_F32(xMotorCtrl.targetPosition, xMotorCtrlLimit.posMin, xMotorCtrlLimit.posMax);
                speedCmd = PID_Calculate(&posPid, xMotorCtrl.targetPosition, xMotorCtrl.currentPosition);
            }
            loopPosCtrlDivider = 0U;
        }

        // [1ms 제어 루프] 중간 루프: 속도 제어 연산
        if (xMotorCtrl.mode == MOTOR_MODE_SPEED_CTRL)
        {
            currentCmd = PID_Calculate(&speedPid, xMotorCtrl.targetSpeedRpm, xMotorCtrl.currentSpeedRpm);
        }
        else if (xMotorCtrl.mode == MOTOR_MODE_POS_CTRL)
        {
            currentCmd = PID_Calculate(&speedPid, speedCmd, xMotorCtrl.currentSpeedRpm);
        }
        
        loop1msCnt = 0U;
    }

    // [100us 제어 루프] 내부 루프: 실시간 하드웨어 전류 제어 연산
    float32_t duty = PID_Calculate(&currPid, currentCmd, xAdc.isenMotLpf);
    
    MotorCtrl_SetOutput(duty);

}

/*
@function    bool MotorCtrl_IsSafeToZeroSet(void)
@brief      엔코더 영점(Zero) 설정 안전 조건 충족 여부 확인
@param      void
@return     bool
*/
bool MotorCtrl_IsSafeToZeroSet(void)
{
    if (xMotorCtrl.safeToZeroTimerTick >= xMotorCtrlLimit.safeZeroSetTick100us)
    {
        return true;
    }
    return false;
}
