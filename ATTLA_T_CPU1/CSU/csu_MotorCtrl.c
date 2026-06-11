/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_MotorCtrl.c
    Version          : 00.01
    Description      : 1x PWM 모드 기반 모터 제어 모듈
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 11. (주석 표준화 및 레거시 코드 정리)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 상태 변수들을 stMotorCtrlState 구조체(xMotorCtrl)로 통합
 * 2026. 06. 11. - Driverlib 직접 호출 제거 및 HAL 추상화, Include 정리
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */


#include "csu_MotorCtrl.h"

// 모터 제어 모듈의 전체 상태
stMotorCtrlState xMotorCtrl;

// PID 제어기 인스턴스
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
    // 100us = 0.0001s
    PID_Init(&speedPid, 0.5f, 0.01f, 0.0f, 0.0001f, 100.0f, -100.0f);
    PID_Init(&posPid, 1.0f, 0.0f, 0.0f, 0.0001f, 3000.0f, -3000.0f); // Position 출력은 Speed 명령
    
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
    
    // 속도 계산 (이전 위치와의 차이를 이용한 차분 연산 또는 엔코더의 속도 레지스터 읽기)
    static float32_t prevPos = 0.0f;
    float32_t posDiff = xMotorCtrl.currentPosition - prevPos;
    
    // 단순 미분을 통한 RPM 계산 (100us 주기 기준)
    // RPM = (Delta Deg / 360) * (1 / 0.0001) * 60 = Delta Deg * 1666.6667
    xMotorCtrl.currentSpeedRpm = posDiff * 1666.6667f;
    prevPos = xMotorCtrl.currentPosition;
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
        speedPid.integral = 0.0f;
        posPid.integral = 0.0f;
    }
    else if (xMotorCtrl.mode == MOTOR_MODE_SPEED_CTRL)
    {
        float32_t duty = PID_Calculate(&speedPid, xMotorCtrl.targetSpeedRpm, xMotorCtrl.currentSpeedRpm);
        MotorCtrl_SetOutput(duty);
    }
    else if (xMotorCtrl.mode == MOTOR_MODE_POS_CTRL)
    {
        // 위치 제어기 출력이 목표 속도가 됨
        float32_t speedCmd = PID_Calculate(&posPid, xMotorCtrl.targetPosition, xMotorCtrl.currentPosition);
        float32_t duty = PID_Calculate(&speedPid, speedCmd, xMotorCtrl.currentSpeedRpm);
        MotorCtrl_SetOutput(duty);
    }
}
