/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : hal_DspInit.c
    Version          : 00.02
    Description      : DSP 초기화 (CM 코어 통신 버퍼 포함)
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 09. (F2838x SPI 핀 매크로 이름 수정)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 02. - CM 코어 기동 시점(Initial_CmCore)을 동기화(IPC_sync) 직전 최고의 타이밍으로 대이동 교정
 * 2026. 06. 02. - 온도 센서 전용 1kHz 느린 트리거용 ePWM9 모듈 추가 기동 반영
 * 2026. 06. 02. - 이더넷 PHY 칩(DP83822) 하드웨어 리셋 핀(GPIO 147) 강제 해제 추가
 * 2026. 06. 02. - C2000Ware 공식 예제 기준 이더넷 클럭 공급(setEnetClk) 및 GPIO108 PWDN 해제 설정 추가
 * 2026. 06. 02. - IPC_sync 대기 전에 이더넷 PHY 기동(initEmacGpioPins)이 되도록 기동 시퀀스 순서 최우선 개편
 * 2026. 06. 02. - 실험 A: GPIO108을 일반 GPIO 입력/풀업으로 원복하여 PHY 자체 스트랩 설정 보호
 * 2026. 06. 02. - 정석 Active-Low 리셋 시퀀스 복구 및 GPIO119 강력한 푸시풀 출력(STD) 모드 융합 적용
 * 2026. 06. 04. - IPC 동기화(Initial_IPC) 호출을 DSP_Initialization 내부에서 main.c로 상향 이동
 * 2026. 06. 04. - CM 하드폴트 원천 박멸을 위해 미존재 인터럽트 권한 양도 API 삭제 및 초기화 안정화
 */


/* ************************** [[    include    ]]    *********************************************************** */
#include "hal_DspInit.h"


/* ************************** [[    define     ]]    *********************************************************** */


/* ************************** [[    global     ]]    *********************************************************** */


/* ************************** [[    static prototype    ]]    *************************************************** */
static void Initial_GPIO(void);
static void Init_GpioDin(void);
static void Init_GpioDout(void);

static void InitialPeripherals(void);

// Helper functions for peripheral initialization (Complexity reduction)
static void initSystemAnalogAdc(void);
static void initSystemPwm(void);
static void initSystemUserInterface(void);
static void initSystemCommunications(void);
static void initW6100GpioPins(void);  /* W6100 하드웨어 리셋 및 제어 핀 초기화 */


/* ************************** [[    function    ]]    *********************************************************** */
/*
@funtion    void DSP_Initialization(void)
@brief      DSP 초기화 수행의 진입점
@param      void
@return     void
@remark 
    - CM 코어와 IPC 하드웨어를 동기화시키고 각종 페리페럴을 초기화합니다.
*/
void DSP_Initialization(void)
{
    // 시스템 및 주변회로 클럭 설정
    Device_init();

    /* --- W6100 제어 핀 및 SPI A 포트 초기화 --- */
    initW6100GpioPins();

    Initial_GPIO();

    // 주변회로 인터럽트 확장 회로(PIE) 및 관련 레지스터 초기화 / CPU 인터럽트 비-활성화
    Interrupt_initModule();

    // PIE 벡터 테이블 초기화 및 기본 인터럽트 서비스 루틴 연결
    Interrupt_initVectorTable();

    // 주변 장치 하드웨어 초기화 미리 수행 (타이머 및 기타 통신망 셋업)
    InitialPeripherals();

    // 실시간 디버깅 활성화, 전역 인터럽트 스위치 ON
    ERTM;   // Debug Enable Mask 비트 설정 (실시간 디버깅이 가능하도록 ST1 레지스터의 /DBGM 비트를 0으로 클리어)
    EINT;   // 전역 인터럽트 스위치 ON (/INTM ON)
}

/*
@funtion    static void Initial_GPIO(void)
@brief      GPIO 초기화 (DIN / DOUT)
@param      void
@return     static void
@remark 
    - GPIO 입력 및 출력을 개별 단위로 나누어 초기화합니다.
*/
static void Initial_GPIO(void)
{
    Init_GpioDin();
    Init_GpioDout();
}

/*
@funtion    static void Init_GpioDin(void)
@brief      디지털 입력 GPIO 설정
@param      void
@return     static void
@remark 
    - 특정 핀을 풀업(Pull-up) 입력 모드로 구성합니다.
*/
static void Init_GpioDin(void)
{
    // GPIO 1: 입력 설정 (GND 체크용)
    GPIO_setPinConfig(GPIO_1_GPIO1);
    GPIO_setPadConfig(1u, GPIO_PIN_TYPE_PULLUP);
    GPIO_setDirectionMode(1u, GPIO_DIR_MODE_IN);

    // --- 모터 드라이버 피드백 및 홀 센서 ---
    // DRV_nFAULT (GPIO 10)
    GPIO_setPinConfig(GPIO_10_GPIO10);
    GPIO_setPadConfig(10U, GPIO_PIN_TYPE_PULLUP);
    GPIO_setDirectionMode(10U, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(10U, GPIO_QUAL_ASYNC);

    // HALL_A_IN (GPIO 11)
    GPIO_setPinConfig(GPIO_11_GPIO11);
    GPIO_setPadConfig(11U, GPIO_PIN_TYPE_PULLUP);
    GPIO_setDirectionMode(11U, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(11U, GPIO_QUAL_ASYNC);

    // HALL_B_IN (GPIO 12)
    GPIO_setPinConfig(GPIO_12_GPIO12);
    GPIO_setPadConfig(12U, GPIO_PIN_TYPE_PULLUP);
    GPIO_setDirectionMode(12U, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(12U, GPIO_QUAL_ASYNC);

    // HALL_C_IN (GPIO 13)
    GPIO_setPinConfig(GPIO_13_GPIO13);
    GPIO_setPadConfig(13U, GPIO_PIN_TYPE_PULLUP);
    GPIO_setDirectionMode(13U, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(13U, GPIO_QUAL_ASYNC);

    // --- 시스템 모니터링 및 스위치 입력 ---
    // nLIMIT1_NO (GPIO 36)
    GPIO_setPinConfig(GPIO_36_GPIO36);
    GPIO_setPadConfig(36U, GPIO_PIN_TYPE_PULLUP);
    GPIO_setDirectionMode(36U, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(36U, GPIO_QUAL_ASYNC);

    // nLIMIT1_NC (GPIO 37)
    GPIO_setPinConfig(GPIO_37_GPIO37);
    GPIO_setPadConfig(37U, GPIO_PIN_TYPE_PULLUP);
    GPIO_setDirectionMode(37U, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(37U, GPIO_QUAL_ASYNC);

    // nLIMIT2_NO (GPIO 38)
    GPIO_setPinConfig(GPIO_38_GPIO38);
    GPIO_setPadConfig(38U, GPIO_PIN_TYPE_PULLUP);
    GPIO_setDirectionMode(38U, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(38U, GPIO_QUAL_ASYNC);

    // nLIMIT2_NC (GPIO 39)
    GPIO_setPinConfig(GPIO_39_GPIO39);
    GPIO_setPadConfig(39U, GPIO_PIN_TYPE_PULLUP);
    GPIO_setDirectionMode(39U, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(39U, GPIO_QUAL_ASYNC);

    // PM_n24V (GPIO 40)
    GPIO_setPinConfig(GPIO_40_GPIO40);
    GPIO_setPadConfig(40U, GPIO_PIN_TYPE_PULLUP);
    GPIO_setDirectionMode(40U, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(40U, GPIO_QUAL_ASYNC);

    // CABLE_LOOP (GPIO 46)
    GPIO_setPinConfig(GPIO_46_GPIO46);
    GPIO_setPadConfig(46U, GPIO_PIN_TYPE_PULLUP);
    GPIO_setDirectionMode(46U, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(46U, GPIO_QUAL_ASYNC);

    // [하드웨어 연결 정보 메모] 
    // - PonRST 신호는 easyDSP 리셋핀, 내부 3.3V 레귤레이터(TPS70445PWP) 리셋핀, 그리고 DSP XRS_N 과 직결됨.
    // - 전원 인가 및 easyDSP 리셋 동작 시 하드웨어적으로 동시에 연동됨.
    // - nBOOT 핀은 GPIO 72에 연결되어 있으며 Boot ROM에서 사용하므로 별도의 GPIO 초기화는 생략함.
}

/*
@funtion    static void Init_GpioDout(void)
@brief      디지털 출력 GPIO 설정
@param      void
@return     static void
@remark 
    - 보드 내 상태 표시용 LED 등을 제어하기 위한 출력 핀을 초기화합니다.
*/
static void Init_GpioDout(void)
{
    hal_Led_InitGpio();

    // --- 모터 드라이버 제어 출력 (DRV8343) ---
    // DRV_ENABLE (GPIO 2 / EPWM1A)
    GPIO_setPinConfig(GPIO_2_GPIO2);
    GPIO_setPadConfig(2U, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(2U, GPIO_DIR_MODE_OUT);
    GPIO_writePin(2U, 0U); // Active High 이므로 기본 Low (OFF)

    // DRV_DIR (GPIO 3 / EPWM2A)
    GPIO_setPinConfig(GPIO_3_GPIO3);
    GPIO_setPadConfig(3U, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(3U, GPIO_DIR_MODE_OUT);
    GPIO_writePin(3U, 0U); // Active High 이므로 기본 Low (OFF)

    // // DRV_nBRAKE (GPIO 4 / EPWM2B)
    // nBRAKE 사용하지 않으므로, 주석처리
    // GPIO_setPinConfig(GPIO_4_GPIO4);
    // GPIO_setPadConfig(4U, GPIO_PIN_TYPE_STD);
    // GPIO_setDirectionMode(4U, GPIO_DIR_MODE_OUT);
    // GPIO_writePin(4U, 1U); // Active Low 이므로 기본 High (OFF)

    // --- 외부 장비 표시 및 제어용 IO ---
    // LED_nNORMAL (GPIO 31 할당)
    GPIO_setPinConfig(GPIO_31_GPIO31);
    GPIO_setPadConfig(31U, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(31U, GPIO_DIR_MODE_OUT);
    GPIO_writePin(31U, 1U); // Active Low 이므로 기본 High (OFF)

    // LED_nFAULT (GPIO 32 할당)
    GPIO_setPinConfig(GPIO_32_GPIO32);
    GPIO_setPadConfig(32U, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(32U, GPIO_DIR_MODE_OUT);
    GPIO_writePin(32U, 1U); // Active Low 이므로 기본 High (OFF)

    // DSP_BRAKE (GPIO 35 할당)
    GPIO_setPinConfig(GPIO_35_GPIO35);
    GPIO_setPadConfig(35U, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(35U, GPIO_DIR_MODE_OUT);
    GPIO_writePin(35U, 0U); // Active High 이므로 기본 Low (OFF)
}

/*
@funtion    static void InitialPeripherals(void)
@brief      DSP 주변 디바이스 초기화
@param      void
@return     static void
@remark 
    - ADC, PWM, UI, 통신 채널 등을 일괄 초기화하는 래퍼 함수입니다.
*/
static void InitialPeripherals(void)
{
    initSystemAnalogAdc();
    initSystemPwm();
    initSystemUserInterface();
    initSystemCommunications();
}

/*
@funtion    static void initSystemAnalogAdc(void)
@brief      ADC 등 아날로그 입력 하드웨어 초기화
@param      void
@return     static void
*/
static void initSystemAnalogAdc(void)
{
    InitialAdc();
    Initial_Adc();
}

/*
@funtion    static void initSystemPwm(void)
@brief      EPWM 등 펄스 폭 변조 제어 하드웨어 초기화
@param      void
@return     static void
*/
static void initSystemPwm(void)
{
    // initEPWM8(); // EPWM1 트리거 사용으로 불필요 (삭제 또는 미정의)
    // initEPWM9(); // 온도 센서 전용 1kHz 느린 주기 ADC 트리거용 ePWM9 추가 기동 (EPWM1으로 통합)
    Initial_Epwm7a();
}

/*
@funtion    static void initSystemUserInterface(void)
@brief      사용자 인터페이스 장치(LED 등) 초기화
@param      void
@return     static void
*/
static void initSystemUserInterface(void)
{
    Initial_Led();
}

/*
@funtion    static void initSystemCommunications(void)
@brief      SPI, SCI, Timer 등 시스템 통신 초기화
@param      void
@return     static void
*/
static void initSystemCommunications(void)
{
    Initial_SPI();
    Initial_SCI();
    Initial_TIMER();
    Initial_EpwmTimer();  /* EPWM1 기반 타이머 활성화 */
}

/*
@funtion    static void initW6100GpioPins(void)
@brief      W6100 이더넷 컨트롤러용 SPI A 및 제어 핀 초기화
@param      void
@return     static void
@remark
    - W6100 연결 핀:
      SIMO: GPIO16, SOMI: GPIO17, CLK: GPIO18
      nCS:  GPIO19 (수동 제어를 위해 일반 GPIO 출력)
      INTn: GPIO20 (입력, 풀업)
      RSTn: GPIO21 (출력, High 유지, Active-Low)
*/
static void initW6100GpioPins(void)
{
    /* --- SPI A 핀 (16, 17, 18) --- */
    GPIO_setPinConfig(GPIO_16_SPIA_SIMO);
    GPIO_setPadConfig(16U, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(16U, GPIO_QUAL_ASYNC);

    GPIO_setPinConfig(GPIO_17_SPIA_SOMI);
    GPIO_setPadConfig(17U, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(17U, GPIO_QUAL_ASYNC);

    GPIO_setPinConfig(GPIO_18_SPIA_CLK);
    GPIO_setPadConfig(18U, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(18U, GPIO_QUAL_ASYNC);

    /* --- nCS (GPIO19) 수동 제어 --- */
    GPIO_setPinConfig(GPIO_19_GPIO19);
    GPIO_setDirectionMode(19U, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(19U, GPIO_PIN_TYPE_PULLUP);
    GPIO_writePin(19U, 1U); // CS High (비활성)

    /* --- INTn (GPIO20) --- */
    GPIO_setPinConfig(GPIO_20_GPIO20);
    GPIO_setDirectionMode(20U, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(20U, GPIO_PIN_TYPE_PULLUP);

    /* --- RSTn (GPIO21) 하드웨어 리셋 --- */
    GPIO_setPinConfig(GPIO_21_GPIO21);
    GPIO_setDirectionMode(21U, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(21U, GPIO_PIN_TYPE_PULLUP);
    
    // W6100 하드웨어 리셋 시퀀스
    GPIO_writePin(21U, 0U);
    DEVICE_DELAY_US(10000U); // 10ms
    GPIO_writePin(21U, 1U);
    DEVICE_DELAY_US(50000U); // PLL 안정화 대기 50ms
}
