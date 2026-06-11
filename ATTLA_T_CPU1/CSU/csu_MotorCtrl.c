/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_MotorCtrl.c
    Version          : 00.05
    Description      : 1x PWM 모드 기반 모터 제어 모듈
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 11. (위치 제어기 4ms 주기 분리 - 대역폭 최적화)
**********************************************************************/

/*
 * Modification History
 * --------------------
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

// 모터 제어 모듈의 전체 상태
stMotorCtrlState xMotorCtrl;

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

    // 하위 Driver 상태 초기화
    MotorDriver_Init();
    
    // PID 초기화 (Kp, Ki, Kd, dt, max_out, min_out)
    // 전류 제어기는 100us (0.0001s) 루프에서 동작
    PID_Init(&currPid, 2.0f, 0.05f, 0.0f, 0.0001f, 100.0f, 0.0f);     // Current 출력은 절대 Duty 크기 (0~100)
    
    // 속도 제어기는 1ms (0.001s) 루프에서 동작
    PID_Init(&speedPid, 0.5f, 0.01f, 0.0f, 0.001f, LIMIT_CURRENT_MAX, LIMIT_CURRENT_MIN);    // Speed 출력은 타겟 전류량 (최대 ±10.0A)
    
    // 위치 제어기는 기계적 관성을 고려하여 4ms (0.004s) 루프에서 동작
    PID_Init(&posPid, 1.0f, 0.0f, 0.0f, 0.004f, LIMIT_SPEED_MAX, LIMIT_SPEED_MIN);           // Position 출력은 Speed 명령
    
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
    // 18비트(싱글턴)가 1회전(360도)에 해당하므로, 360 / 2^18 = 0.001373291015625 (정확한 이진 소수점)
    // 이를 곱하여 모터 축 기준 기계각(기구 단 아님) Degree 계산
    xMotorCtrl.currentPosition = (float32_t)xEncoder.position * 0.001373291f; 
    
    // 속도 계산 (1ms 분주(Decimation) 방식 적용, 이산 오차 최소화)
    static Uint16 speedCalcCnt = 0U;
    static float32_t prevPos = 0.0f;
    
    speedCalcCnt++;
    if (speedCalcCnt >= 10U)
    {
        float32_t posDiff = xMotorCtrl.currentPosition - prevPos;
        
        // 단순 미분을 통한 RPM 계산 (1ms 주기 기준)
        // RPM = (Delta Deg / 360) * (1 / 0.001) * 60 = Delta Deg * 166.6667
        xMotorCtrl.currentSpeedRpm = posDiff * 166.6667f;
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
        if (outputDuty > 100.0f) outputDuty = 100.0f;
    }
    else
    {
        // 역방향
        MotorDriver_SetDir(false);
        outputDuty = -outputDuty;
        if (outputDuty > 100.0f) outputDuty = 100.0f;
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

    if (xMotorCtrl.mode == MOTOR_MODE_STOP)
    {
        MotorCtrl_SetOutput(0.0f);
        currPid.integral = 0.0f;
        speedPid.integral = 0.0f;
        posPid.integral = 0.0f;
    }
    else
    {
        static float32_t currentCmd = 0.0f;
        static float32_t speedCmd = 0.0f;
        static Uint16 loop1msCnt = 0U;
        static Uint16 loop4msDivider = 0U;
        
        loop1msCnt++;
        if (loop1msCnt >= 10U)
        {
            // [4ms 제어 루프] 최외곽 루프: 위치 제어 연산
            loop4msDivider++;
            if (loop4msDivider >= 4U)
            {
                if (xMotorCtrl.mode == MOTOR_MODE_POS_CTRL)
                {
                    // 위치 명령 소프트 리미트 적용
                    xMotorCtrl.targetPosition = CLAMP_F32(xMotorCtrl.targetPosition, LIMIT_POS_MIN, LIMIT_POS_MAX);
                    speedCmd = PID_Calculate(&posPid, xMotorCtrl.targetPosition, xMotorCtrl.currentPosition);
                }
                loop4msDivider = 0U;
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
        float32_t currentCmdAbs = (currentCmd >= 0.0f) ? currentCmd : -currentCmd;
        float32_t currentFdbkAbs = (xAdc.isenMotLpf >= 0.0f) ? xAdc.isenMotLpf : -xAdc.isenMotLpf;
        
        float32_t dutyAbs = PID_Calculate(&currPid, currentCmdAbs, currentFdbkAbs);
        float32_t duty = (currentCmd >= 0.0f) ? dutyAbs : -dutyAbs;
        
        MotorCtrl_SetOutput(duty);
    }
}
