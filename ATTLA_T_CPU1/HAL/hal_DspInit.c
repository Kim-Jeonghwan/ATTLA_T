/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : hal_DspInit.c
    Version          : 00.15
    Description      : DSP 초기화 및 GPIO/인터럽트 기본 설정
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 07. 01. (초기화 구문 상세 한글 주석 추가)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 07. 01. - 초기화 구문 상세 한글 주석 추가 (코딩 규칙 적용)
 * 2026. 06. 30. - GPIO 145 제어권을 CPU1으로 회수 및 LED_nG 용도로 변경
 * 2026. 06. 26. - 시스템 제어 및 스위치 입력/출력 핀 할당 매직넘버 상수화 적용
 * 2026. 06. 26. - 하드웨어 스위치 및 브레이크 제어 핀맵 재할당 (GPIO 73, 74, 76~80)
 * 2026. 06. 24. - EMAC 관련 17개 핀에 대해 CM 코어로 제어권 이양 추가
 * 2026. 06. 15. - 기존 hal_Led.c 파일 삭제 및 LED 핀 초기화 코드를 Init_GpioDout에 통합 관리
 * 2026. 06. 15. - W6100 SPI-A 통신 핀 및 CS 설정(GPIO 16~19)을 hal_Spi.c의 InitSpia()로 이관
 * 2026. 06. 12. - DSP_BRAKE 핀을 Active High(전원 인가 시 브레이크 잠금 해제)로 정정 및 주석 수정
 * 2026. 06. 12. - 내부 온도 센서 미사용에 따른 관련 주석(ePWM9) 삭제
 * 2026. 06. 12. - GPIO 1 입력 설정(GND 체크용) 제거
 * 2026. 06. 12. - CM 및 IPC 관련 주석 제거
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 02. - 이더넷 PHY 칩(DP83822) 하드웨어 리셋 핀(GPIO 147) 강제 해제 추가
 * 2026. 06. 02. - C2000Ware 공식 예제 기준 이더넷 클럭 공급(setEnetClk) 및 GPIO108 PWDN 해제 설정 추가
 * 2026. 06. 02. - 실험 A: GPIO108을 일반 GPIO 입력/풀업으로 원복하여 PHY 자체 스트랩 설정 보호
 * 2026. 06. 02. - 정석 Active-Low 리셋 시퀀스 복구 및 GPIO119 강력한 푸시풀 출력(STD) 모드 융합 적용
 * 2026. 06. 11. - DSP_Initialization 함수명을 System_Initialization으로 변경
 * 2026. 06. 11. - DSP_BRAKE 핀 제어 로직 회로도 스펙(Active High 오기재 수정 전) 일치화
 * 2026. 06. 11. - 함수명 접두어(csu_, hal_) 제거 리팩토링
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
static void initEmacGpioPins(void);   /* DP83822 이더넷 PHY MII 핀 MUX 설정 */
static void Initial_CmCore(void);     /* CM 코어 클럭 인가 및 부팅 기동 */

// 주변장치 초기화용 헬퍼 함수 (정적 시험 메트릭: 복잡도 감소 목적)
static void initSystemAnalogAdc(void);
static void initSystemPwm(void);
static void initSystemUserInterface(void);
static void initSystemCommunications(void);
static void initW6100GpioPins(void);  /* W6100 하드웨어 리셋 및 제어 핀 초기화 */


/* ************************** [[    function    ]]    *********************************************************** */
/*
@function    void System_Initialization(void)
@brief      DSP 및 주변장치 초기화 수행의 진입점
@param      void
@return     void
@remark 
    - 각종 페리페럴을 초기화합니다.
*/
void System_Initialization(void)
{
    // 시스템 및 주변회로 클럭 설정 (PLL, 워치독 비활성화 등 코어 필수 하드웨어 기동)
    Device_init();

    /* --- [최우선 조치] 동기화 대기 전에 물리 이더넷 PHY부터 즉시 기상 --- */
    initEmacGpioPins(); // CM 코어 부팅 전 이더넷 관련 GPIO MUX 권한을 CM으로 이양하고 PHY 칩 리셋 해제

    /* --- W6100 제어 핀 및 SPI A 포트 초기화 --- */
    initW6100GpioPins(); // W6100용 SPI 포트와 하드웨어 리셋, 인터럽트 입력 핀 초기화

    Initial_GPIO(); // 시스템 입출력(스위치, LED, 모터 드라이버 제어) GPIO 포트 초기화

    // 주변회로 인터럽트 확장 회로(PIE) 및 관련 레지스터 초기화 / CPU 인터럽트 비-활성화 (전역 인터럽트 마스킹 상태)
    Interrupt_initModule();

    // PIE 벡터 테이블 초기화 및 기본 인터럽트 서비스 루틴 연결 (ISR 함수 주소 매핑)
    Interrupt_initVectorTable();

    // 주변 장치 하드웨어 초기화 미리 수행 (타이머, ADC, SPI 등 필수 통신망 셋업 및 모듈 드라이버 연동)
    InitialPeripherals();

    /* --- CM 코어 기동 전 IPC 상태 사전 청소 --- */
    Initial_IPC_Clear(); // 양방향 통신 플래그 초기화로 쓰레기값에 의한 오동작 차단

    /* --- [정석 타이밍 적용] 모든 준비 완료 후 CM 코어 기동 --- */
    Initial_CmCore(); // CM 코어 클럭 인가 및 부트롬 분기

    // 실시간 디버깅 활성화, 전역 인터럽트 스위치 ON
    ERTM;   // Debug Enable Mask 비트 설정 (실시간 디버깅이 가능하도록 ST1 레지스터의 /DBGM 비트를 0으로 클리어)
    EINT;   // 전역 인터럽트 스위치 ON (/INTM ON - 최하단에서 일괄적으로 락 해제)
}

/*
@function    static void Initial_GPIO(void)
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
@function    static void Init_GpioDin(void)
@brief      디지털 입력 GPIO 설정
@param      void
@return     static void
@remark 
    - 특정 핀을 풀업(Pull-up) 입력 모드로 구성합니다.
*/
static void Init_GpioDin(void)
{
    // --- 모터 드라이버 피드백 및 홀 센서 ---
    // DRV_nFAULT (GPIO 10)
    GPIO_setPinConfig(GPIO_10_GPIO10);
    GPIO_setPadConfig(GPIO_PIN_DRV_nFAULT, GPIO_PIN_TYPE_PULLUP);
    GPIO_setDirectionMode(GPIO_PIN_DRV_nFAULT, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(GPIO_PIN_DRV_nFAULT, GPIO_QUAL_ASYNC);

    // HALL_A_IN (INLA, GPIO 11)
    GPIO_setPinConfig(GPIO_11_GPIO11);
    GPIO_setPadConfig(GPIO_PIN_HALL_A_IN, GPIO_PIN_TYPE_PULLUP);
    GPIO_setDirectionMode(GPIO_PIN_HALL_A_IN, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(GPIO_PIN_HALL_A_IN, GPIO_QUAL_ASYNC);

    // HALL_B_IN (INHB, GPIO 12)
    GPIO_setPinConfig(GPIO_12_GPIO12);
    GPIO_setPadConfig(GPIO_PIN_HALL_B_IN, GPIO_PIN_TYPE_PULLUP);
    GPIO_setDirectionMode(GPIO_PIN_HALL_B_IN, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(GPIO_PIN_HALL_B_IN, GPIO_QUAL_ASYNC);

    // HALL_C_IN (INLB, GPIO 13)
    GPIO_setPinConfig(GPIO_13_GPIO13);
    GPIO_setPadConfig(GPIO_PIN_HALL_C_IN, GPIO_PIN_TYPE_PULLUP);
    GPIO_setDirectionMode(GPIO_PIN_HALL_C_IN, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(GPIO_PIN_HALL_C_IN, GPIO_QUAL_ASYNC);

    // --- 시스템 모니터링 및 스위치 입력 ---
    // nLIMIT1_NO (GPIO 73, ※TX_D0 중복 방지 테스트 임시, 향후 75로 변경)
    GPIO_setPinConfig(GPIO_73_GPIO73);
    GPIO_setPadConfig(GPIO_PIN_nLIMIT1_NO, GPIO_PIN_TYPE_PULLUP);
    GPIO_setDirectionMode(GPIO_PIN_nLIMIT1_NO, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(GPIO_PIN_nLIMIT1_NO, GPIO_QUAL_ASYNC);

    // nLIMIT1_NC (GPIO 76)
    GPIO_setPinConfig(GPIO_76_GPIO76);
    GPIO_setPadConfig(GPIO_PIN_nLIMIT1_NC, GPIO_PIN_TYPE_PULLUP);
    GPIO_setDirectionMode(GPIO_PIN_nLIMIT1_NC, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(GPIO_PIN_nLIMIT1_NC, GPIO_QUAL_ASYNC);

    // nLIMIT2_NO (GPIO 77)
    GPIO_setPinConfig(GPIO_77_GPIO77);
    GPIO_setPadConfig(GPIO_PIN_nLIMIT2_NO, GPIO_PIN_TYPE_PULLUP);
    GPIO_setDirectionMode(GPIO_PIN_nLIMIT2_NO, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(GPIO_PIN_nLIMIT2_NO, GPIO_QUAL_ASYNC);

    // nLIMIT2_NC (GPIO 78)
    GPIO_setPinConfig(GPIO_78_GPIO78);
    GPIO_setPadConfig(GPIO_PIN_nLIMIT2_NC, GPIO_PIN_TYPE_PULLUP);
    GPIO_setDirectionMode(GPIO_PIN_nLIMIT2_NC, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(GPIO_PIN_nLIMIT2_NC, GPIO_QUAL_ASYNC);

    // PM_n24V (GPIO 79)
    GPIO_setPinConfig(GPIO_79_GPIO79);
    GPIO_setPadConfig(GPIO_PIN_PM_n24V, GPIO_PIN_TYPE_PULLUP);
    GPIO_setDirectionMode(GPIO_PIN_PM_n24V, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(GPIO_PIN_PM_n24V, GPIO_QUAL_ASYNC);

    // nCABLE_LOOP (GPIO 80)
    GPIO_setPinConfig(GPIO_80_GPIO80);
    GPIO_setPadConfig(GPIO_PIN_nCABLE_LOOP, GPIO_PIN_TYPE_PULLUP);
    GPIO_setDirectionMode(GPIO_PIN_nCABLE_LOOP, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(GPIO_PIN_nCABLE_LOOP, GPIO_QUAL_ASYNC);

    // [하드웨어 연결 정보 메모] 
    // - PonRST 신호는 easyDSP 리셋핀, 내부 3.3V 레귤레이터(TPS70445PWP) 리셋핀, 그리고 DSP XRS_N 과 직결됨.
    // - 전원 인가 및 easyDSP 리셋 동작 시 하드웨어적으로 동시에 연동됨.
    // - nBOOT 핀은 GPIO 72에 연결되어 있으며 Boot ROM에서 사용하므로 별도의 GPIO 초기화는 생략함.
}

/*
@function    static void Init_GpioDout(void)
@brief      디지털 출력 GPIO 설정
@param      void
@return     static void
@remark 
    - 보드 내 상태 표시용 LED 등을 제어하기 위한 출력 핀을 초기화합니다.
*/
static void Init_GpioDout(void)
{
    // --- 모터 드라이버 제어 출력 (DRV8343) ---
    // DRV_ENABLE (GPIO 2 / EPWM2A)
    GPIO_setPinConfig(GPIO_2_GPIO2);
    GPIO_setPadConfig(GPIO_PIN_DRV_ENABLE, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_DRV_ENABLE, GPIO_DIR_MODE_OUT);
    GPIO_writePin(GPIO_PIN_DRV_ENABLE, 0U); // Active High 이므로 기본 Low (OFF)

    // DRV_DIR (INHC, GPIO 3 / EPWM2B)
    GPIO_setPinConfig(GPIO_3_GPIO3);
    GPIO_setPadConfig(GPIO_PIN_DRV_DIR, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_DRV_DIR, GPIO_DIR_MODE_OUT);
    GPIO_writePin(GPIO_PIN_DRV_DIR, 0U); // Active High 이므로 기본 Low (OFF)

    // DRV_nBRAKE (INLC, GPIO 4 / EPWM3A)
    // DRV8343 1x PWM 모드 구동을 위해 브레이크 해제 상태(High)로 초기화
    GPIO_setPinConfig(GPIO_4_GPIO4);
    GPIO_setPadConfig(GPIO_PIN_DRV_nBRAKE, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_DRV_nBRAKE, GPIO_DIR_MODE_OUT);
    GPIO_writePin(GPIO_PIN_DRV_nBRAKE, 1U); // Active Low 이므로 High 출력이 브레이크 해제 (Normal 구동) 상태임

    // --- 외부 장비 표시 및 제어용 IO ---
    // LED_nNORMAL (GPIO 31 할당)
    GPIO_setPinConfig(GPIO_31_GPIO31);
    GPIO_setPadConfig(GPIO_PIN_LED_nNORMAL, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_LED_nNORMAL, GPIO_DIR_MODE_OUT);
    GPIO_writePin(GPIO_PIN_LED_nNORMAL, 1U); // Active Low 이므로 기본 High (OFF)

    // LED_nFAULT (GPIO 32 할당)
    GPIO_setPinConfig(GPIO_32_GPIO32);
    GPIO_setPadConfig(GPIO_PIN_LED_nFAULT, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_LED_nFAULT, GPIO_DIR_MODE_OUT);
    GPIO_writePin(GPIO_PIN_LED_nFAULT, 1U); // Active Low 이므로 기본 High (OFF)

    // BRAKE (GPIO 74 할당)
    GPIO_setPinConfig(GPIO_74_GPIO74);
    GPIO_setPadConfig(GPIO_PIN_BRAKE, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_BRAKE, GPIO_DIR_MODE_OUT);
    GPIO_writePin(GPIO_PIN_BRAKE, 0U); // Active High(전원 인가 시 잠금 해제). 따라서 기본 Low 출력으로 전원을 차단하여 기계적 잠금 상태(Fail-Safe) 유지
}

/*
@function    static void InitialPeripherals(void)
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

    // 전역 상태 제어 구조체 초기화
    Control_Init();
    Bit_Init();

    // 주변장치 드라이버 초기화 (PBIT 진행을 위해 선행)
    Fram_Init();
    Encoder_Init();
    MotorDriver_Init_Hardware(); // DRV8343 초기화 (1x PWM 모드 설정 포함)
    MotorCtrl_Init();            // 모터 제어기 (PID, DIR 핀 등) 초기화
}

/*
@function    static void initSystemAnalogAdc(void)
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
@function    static void initSystemPwm(void)
@brief      EPWM 등 펄스 폭 변조 제어 하드웨어 초기화 (EPWM1은 initSystemCommunications에서 초기화됨)
@param      void
@return     static void
*/
static void initSystemPwm(void)
{
    // EPWM1 기반 100us 타이머 및 모터 PWM(GPIO0)은 initSystemCommunications에서 호출됩니다.
}

/*
@function    static void initSystemUserInterface(void)
@brief      사용자 인터페이스 장치(LED 등) 초기화
@param      void
@return     static void
*/
static void initSystemUserInterface(void)
{
    Initial_Led();
}

/*
@function    static void initSystemCommunications(void)
@brief      SPI, SCI, Timer 등 시스템 통신 초기화
@param      void
@return     static void
*/
static void initSystemCommunications(void)
{
    Initial_SPI();
    Initial_TIMER();
    Initial_EpwmTimer();  /* EPWM1 기반 타이머 활성화 */
}

/*
@function    static void initW6100GpioPins(void)
@brief      W6100 이더넷 컨트롤러용 SPI A 및 제어 핀 초기화
@param      void
@return     static void
@remark
    - W6100 연결 핀 중 SPI 핀(16~19)은 hal_Spi.c의 InitSpia()에서 초기화합니다.
    - 본 함수는 제어 핀(INTn, RSTn)만 초기화합니다.
      INTn: GPIO20 (입력, 풀업)
      RSTn: GPIO21 (출력, High 유지, Active-Low)
*/
static void initW6100GpioPins(void)
{
    /* --- INTn (GPIO20) --- */
    GPIO_setPinConfig(GPIO_20_GPIO20);
    GPIO_setDirectionMode(GPIO_PIN_W6100_INTn, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(GPIO_PIN_W6100_INTn, GPIO_PIN_TYPE_PULLUP);

    /* --- RSTn (GPIO21) 하드웨어 리셋 --- */
    GPIO_setPinConfig(GPIO_21_GPIO21);
    GPIO_setDirectionMode(GPIO_PIN_W6100_RSTn, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(GPIO_PIN_W6100_RSTn, GPIO_PIN_TYPE_PULLUP);
    
    // W6100 하드웨어 리셋 시퀀스
    GPIO_writePin(GPIO_PIN_W6100_RSTn, 0U);
    DEVICE_DELAY_US(10000U); // 10ms
    GPIO_writePin(GPIO_PIN_W6100_RSTn, 1U);
    DEVICE_DELAY_US(50000U); // PLL 안정화 대기 50ms
}

/*
@function    static void initEmacGpioPins(void)
@brief      EMAC MII 모드 GPIO 핀 MUX 설정 (CPU1 마스터 권한 제어)
@param      void
@return     void
@remark
    - TBD: 이더넷 PHY 연결용 할당 핀 MUX 설정 및 클럭 공급
*/
static void initEmacGpioPins(void)
{
    /* --- 이더넷 모듈 클럭 공급 설정 (공식 예제 기준 100MHz 세팅) --- */
    SysCtl_setEnetClk(SYSCTL_ENETCLKOUT_DIV_2, SYSCTL_SOURCE_SYSPLL);

    /* --- TX 경로 --- */
    GPIO_setPinConfig(GPIO_44_ENET_MII_TX_CLK);    /* TBD: TX 클럭 */
    GPIO_setMasterCore(GPIO_PIN_MII_TX_CLK, GPIO_CORE_CM);
    GPIO_setPinConfig(GPIO_118_ENET_MII_TX_EN);    /* TBD: TX Enable */
    GPIO_setMasterCore(GPIO_PIN_MII_TX_EN, GPIO_CORE_CM);
    GPIO_setPinConfig(GPIO_75_ENET_MII_TX_DATA0);  /* TBD: TX Data bit0 */
    GPIO_setMasterCore(GPIO_PIN_MII_TX_D0, GPIO_CORE_CM);
    GPIO_setPinConfig(GPIO_122_ENET_MII_TX_DATA1); /* TBD: TX Data bit1 */
    GPIO_setMasterCore(GPIO_PIN_MII_TX_D1, GPIO_CORE_CM);
    GPIO_setPinConfig(GPIO_123_ENET_MII_TX_DATA2); /* TBD: TX Data bit2 */
    GPIO_setMasterCore(GPIO_PIN_MII_TX_D2, GPIO_CORE_CM);
    GPIO_setPinConfig(GPIO_124_ENET_MII_TX_DATA3); /* TBD: TX Data bit3 */
    GPIO_setMasterCore(GPIO_PIN_MII_TX_D3, GPIO_CORE_CM);

    /* --- RX 경로 --- */
    GPIO_setPinConfig(GPIO_111_ENET_MII_RX_CLK);   /* TBD: RX 클럭 */
    GPIO_setMasterCore(GPIO_PIN_MII_RX_CLK, GPIO_CORE_CM);
    GPIO_setPinConfig(GPIO_112_ENET_MII_RX_DV);    /* TBD: RX Data Valid */
    GPIO_setMasterCore(GPIO_PIN_MII_RX_DV, GPIO_CORE_CM);
    GPIO_setPinConfig(GPIO_113_ENET_MII_RX_ERR);   /* TBD: RX Error */
    GPIO_setMasterCore(GPIO_PIN_MII_RX_ERR, GPIO_CORE_CM);
    GPIO_setPinConfig(GPIO_114_ENET_MII_RX_DATA0); /* TBD: RX Data bit0 */
    GPIO_setMasterCore(GPIO_PIN_MII_RX_D0, GPIO_CORE_CM);
    GPIO_setPinConfig(GPIO_115_ENET_MII_RX_DATA1); /* TBD: RX Data bit1 */
    GPIO_setMasterCore(GPIO_PIN_MII_RX_D1, GPIO_CORE_CM);
    GPIO_setPinConfig(GPIO_116_ENET_MII_RX_DATA2); /* TBD: RX Data bit2 */
    GPIO_setMasterCore(GPIO_PIN_MII_RX_D2, GPIO_CORE_CM);
    GPIO_setPinConfig(GPIO_117_ENET_MII_RX_DATA3); /* TBD: RX Data bit3 */
    GPIO_setMasterCore(GPIO_PIN_MII_RX_D3, GPIO_CORE_CM);

    /* --- MDIO 관리 인터페이스 --- */
    GPIO_setPinConfig(GPIO_105_ENET_MDIO_CLK);     /* TBD: MDC 클럭 */
    GPIO_setMasterCore(GPIO_PIN_MII_MDC_CLK, GPIO_CORE_CM);
    GPIO_setPinConfig(GPIO_106_ENET_MDIO_DATA);    /* TBD: MDIO 데이터 */
    GPIO_setMasterCore(GPIO_PIN_MII_MDIO_DATA, GPIO_CORE_CM);

    /* --- CRS / COL 선택적 MII 신호 --- */
    GPIO_setPinConfig(GPIO_109_ENET_MII_CRS);      /* TBD: CRS */
    GPIO_setMasterCore(GPIO_PIN_MII_CRS, GPIO_CORE_CM);
    GPIO_setPinConfig(GPIO_110_ENET_MII_COL);      /* TBD: COL */
    GPIO_setMasterCore(GPIO_PIN_MII_COL, GPIO_CORE_CM);

    /* --- PWDN/INT 핀 (GPIO108): 일반 GPIO 입력 및 풀업 설정 --- */
    GPIO_setPinConfig(GPIO_108_GPIO108);           /* TBD */
    GPIO_setDirectionMode(GPIO_PIN_ENET_PHY_PWDN_INT, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(GPIO_PIN_ENET_PHY_PWDN_INT, GPIO_PIN_TYPE_PULLUP);
    GPIO_setMasterCore(GPIO_PIN_ENET_PHY_PWDN_INT, GPIO_CORE_CM);

    /* --- PHY 하드웨어 리셋 (GPIO119, Active-Low) --- */
    GPIO_setPinConfig(GPIO_119_GPIO119);           /* TBD */
    GPIO_setDirectionMode(GPIO_PIN_ENET_PHY_RESET, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(GPIO_PIN_ENET_PHY_RESET, GPIO_PIN_TYPE_STD);
    
    // CPU1이 직접 리셋을 1회 수행한 뒤 권한을 그대로 유지 (CM으로 넘기면 CM의 DAT 초기값 0이 반영되어 Reset이 계속 Low로 묶이는 현상 방지)
    GPIO_writePin(GPIO_PIN_ENET_PHY_RESET, 0U);    /* Active-Low 리셋 강제 인가 (0V) */
    DEVICE_DELAY_US(10000U);                       /* 10ms 대기 */
    GPIO_writePin(GPIO_PIN_ENET_PHY_RESET, 1U);    /* 리셋 해제 (3.3V) */
    // GPIO_setMasterCore(GPIO_PIN_ENET_PHY_RESET, GPIO_CORE_CM); // 삭제: CPU1이 소유해야 리셋이 High로 유지됨

    /* --- nG 상태 표시용 LED 핀 방향 설정 (CPU1 제어) --- */
    GPIO_setPinConfig(GPIO_145_GPIO145);
    GPIO_setDirectionMode(GPIO_PIN_LED_nG, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(GPIO_PIN_LED_nG, GPIO_PIN_TYPE_STD);
    GPIO_setMasterCore(GPIO_PIN_LED_nG, GPIO_CORE_CPU1);

    // 공통/임시용 GPIO 초기화 함수 호출
    hal_Common_InitTempGpio();
}

/*
@function    static void Initial_CmCore(void)
@brief      CM 코어 AuxPLL 클럭 인가 및 CM 기동
@param      void
@return     void
*/
static void Initial_CmCore(void)
{
    /* CM 클럭 활성화 (AUXPLL 기반 125MHz 설정) */
    SysCtl_setCMClk(SYSCTL_CMCLKOUT_DIV_1, SYSCTL_SOURCE_AUXPLL);

#ifdef _FLASH
    /* Flash 부팅 모드로 CM 기동 */
    Device_bootCM(BOOTMODE_BOOT_TO_FLASH_SECTOR0);
#else
    /* RAM 부팅 모드로 CM 기동 */
    Device_bootCM(BOOTMODE_BOOT_TO_S0RAM);
#endif
}
