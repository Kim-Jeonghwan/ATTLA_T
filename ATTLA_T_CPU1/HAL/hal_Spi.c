/**********************************************************************
 Nexcom Co., Ltd.
 Filename         : hal_Spi.c
 Version          : 00.05
 Description      : SSI 엔코더, W6100, FRAM, 모터 드라이버 통신용 SPI 하드웨어 제어
 Programmer       : Kim Jeonghwan
 Last Updated     : 2026. 06. 15. (W6100 SPI 핀 설정 이관 추가)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 15. - hal_DspInit.c에 있던 W6100 SPI-A 통신 핀(16~19) 설정을 InitSpia로 이관
 * 2026. 06. 15. - SPI 매직넘버 상수화, 핀 네이밍 규칙 통일, InitSpib 통합
 * 2026. 06. 12. - 엔코더 SPI 통신용 핀 매크로를 헤더(.h)로 이동 (글로벌 룰 적용)
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - SPIC_BASE 엔코더 통신 클럭 주파수 2.5MHz로 변경
 */


/* DESCRIPTION
 * 
 * 
 */


/* ************************** [[   include  ]]  *********************************************************** */
#include "hal_Spi.h"

/* ************************** [[   define   ]]  *********************************************************** */


/* ************************** [[   global   ]]  *********************************************************** */


/* ************************** [[   static prototype  ]]  ************************************************** */

static void InitSpia(void);
static void InitSpib(void);
static void InitSpic(void);
static void InitSpid(void);



/* ************************** [[  function  ]]  *********************************************************** */
/*
@function    void Initial_SPI(void)
@brief      SPI 드라이버 초기화
@param      void
@return     void
@remark 
    - W6100용 SPI-A 모듈과 SSI 엔코더용 SPI-C 모듈 초기화를 호출합니다.
    - FRAM용 SPI-D 모듈 초기화를 호출합니다.
*/
void Initial_SPI(void) {
	InitSpia(); // W6100
    InitSpib(); // Motor Driver
    InitSpic(); // SSI
    InitSpid(); // FRAM
}


/*
@function    static void InitSpic(void)
@brief      SSI 엔코더 통신용 SPI-C 모듈 초기화 및 GPIO 설정
@param      void
@return     static void
@remark
    - GPIO 51(SOMI), 52(CLK)를 SPI-C 기능으로 할당합니다.
    - Master 모드, Mode 2(POL1PHA0), 2.5MHz 속도 및 16비트 워드 규격으로 초기화합니다.
*/
static void InitSpic(void)
{
    EALLOW; // 보호 레지스터 쓰기 허용

    // 핀 설정
    // GPIO_setPinConfig(GPIO_63_SPIB_SIMO);
    // GPIO_setPadConfig(SSI_SIMO_SPIB, GPIO_PIN_TYPE_STD);
    // GPIO_setQualificationMode(SSI_SIMO_SPIB, GPIO_QUAL_ASYNC);

    GPIO_setPinConfig(GPIO_51_SPIC_SOMI);
    GPIO_setPadConfig(SPIC_SSI_SOMI_PIN, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(SPIC_SSI_SOMI_PIN, GPIO_QUAL_ASYNC);

    GPIO_setPinConfig(GPIO_52_SPIC_CLK);
    GPIO_setPadConfig(SPIC_SSI_CLK_PIN, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(SPIC_SSI_CLK_PIN, GPIO_QUAL_ASYNC);

    // GPIO_setPinConfig(GPIO_66_GPIO66);
    // GPIO_setPadConfig(SSI_CS, GPIO_PIN_TYPE_STD);
    // GPIO_setQualificationMode(SSI_CS, GPIO_QUAL_SYNC);
    // GPIO_setDirectionMode(SSI_CS, GPIO_DIR_MODE_OUT);
    // GPIO_setMasterCore(SSI_CS, GPIO_CORE_CPU1);



	// SPI 초기화. Mode-2(POL1PHA0)
    SPI_disableModule(SPIC_BASE);
    SPI_setConfig(SPIC_BASE, 
					DEVICE_LSPCLK_FREQ, 
					SPI_PROT_POL1PHA0,                                          // SSI엔코더는 보통 클럭이 High로 대기하다가 첫 번째 하강 엣지에서 데이터를 내보내는 Mode 2 나 Mode 3 많이 씀 (현재 모드2)
					SPI_MODE_MASTER, 
					SPIC_SSI_BAUDRATE,                                          // 2.5MHz
					SPIC_SSI_DATA_WIDTH);                                       // 16 비트
    SPI_disableFIFO(SPIC_BASE);
    SPI_setEmulationMode(SPIC_BASE, SPI_EMULATION_STOP_AFTER_TRANSMIT);
    SPI_enableModule(SPIC_BASE);

    EDIS;   // 보호 레지스터 쓰기 금지
}

/*
@function    static void InitSpia(void)
@brief      W6100 이더넷 통신용 SPI-A 모듈 초기화
@param      void
@return     static void
@remark
    - Master 모드, Mode 0(POL0PHA0), 20MHz 속도 및 8비트 워드 규격으로 초기화합니다.
*/
static void InitSpia(void)
{
    EALLOW;
    
    // GPIO 16: SPIA_SIMO
    GPIO_setPinConfig(GPIO_16_SPIA_SIMO);
    GPIO_setPadConfig(SPIA_W6100_SIMO_PIN, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(SPIA_W6100_SIMO_PIN, GPIO_QUAL_ASYNC);

    // GPIO 17: SPIA_SOMI
    GPIO_setPinConfig(GPIO_17_SPIA_SOMI);
    GPIO_setPadConfig(SPIA_W6100_SOMI_PIN, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(SPIA_W6100_SOMI_PIN, GPIO_QUAL_ASYNC);

    // GPIO 18: SPIA_CLK
    GPIO_setPinConfig(GPIO_18_SPIA_CLK);
    GPIO_setPadConfig(SPIA_W6100_CLK_PIN, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(SPIA_W6100_CLK_PIN, GPIO_QUAL_ASYNC);

    // GPIO 19: SPIA_STE (Chip Select, 수동 제어용)
    GPIO_setPinConfig(GPIO_19_GPIO19);
    GPIO_setDirectionMode(SPIA_W6100_CS_PIN, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(SPIA_W6100_CS_PIN, GPIO_PIN_TYPE_PULLUP);
    GPIO_writePin(SPIA_W6100_CS_PIN, 1U); // CS High (비활성)

    // SPI 초기화. Mode-0(POL0PHA0)
    SPI_disableModule(SPIA_BASE);
    SPI_setConfig(SPIA_BASE, 
					DEVICE_LSPCLK_FREQ, 
					SPI_PROT_POL0PHA0, // WIZnet은 Mode 0 또는 Mode 3 지원
					SPI_MODE_MASTER, 
					SPIA_W6100_BAUDRATE,
					SPIA_W6100_DATA_WIDTH);
    SPI_disableFIFO(SPIA_BASE);
    SPI_setEmulationMode(SPIA_BASE, SPI_EMULATION_FREE_RUN);
    SPI_enableModule(SPIA_BASE);

    EDIS;
}

/*
@function    static void InitSpib(void)
@brief      모터 드라이버(DRV8343) 통신용 SPI-B 모듈 초기화 및 GPIO 설정
@param      void
@return     static void
@remark
    - GPIO 58(CLK), 59(STE), 60(SIMO), 61(SOMI)를 SPI-B 기능으로 할당합니다.
    - Master 모드, Mode 1(POL0PHA1), 1MHz 속도 및 16비트 워드 규격으로 초기화합니다.
*/
static void InitSpib(void)
{
    EALLOW;

    // GPIO 58: SPIB_CLK
    GPIO_setPinConfig(GPIO_58_SPIB_CLK);
    GPIO_setDirectionMode(SPIB_MOTOR_CLK_PIN, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(SPIB_MOTOR_CLK_PIN, GPIO_PIN_TYPE_STD);

    // GPIO 59: SPIB_STE (Chip Select)
    GPIO_setPinConfig(GPIO_59_SPIB_STEN);
    GPIO_setDirectionMode(SPIB_MOTOR_STE_PIN, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(SPIB_MOTOR_STE_PIN, GPIO_PIN_TYPE_STD);

    // GPIO 60: SPIB_SIMO
    GPIO_setPinConfig(GPIO_60_SPIB_SIMO);
    GPIO_setDirectionMode(SPIB_MOTOR_SIMO_PIN, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(SPIB_MOTOR_SIMO_PIN, GPIO_PIN_TYPE_STD);

    // GPIO 61: SPIB_SOMI
    GPIO_setPinConfig(GPIO_61_SPIB_SOMI);
    GPIO_setDirectionMode(SPIB_MOTOR_SOMI_PIN, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(SPIB_MOTOR_SOMI_PIN, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(SPIB_MOTOR_SOMI_PIN, GPIO_QUAL_ASYNC);

    // SPI 모듈 초기화
    SPI_disableModule(SPIB_BASE);
    SPI_setConfig(SPIB_BASE, DEVICE_LSPCLK_FREQ, SPI_PROT_POL0PHA1,
                  SPI_MODE_MASTER, SPIB_MOTOR_BAUDRATE, SPIB_MOTOR_DATA_WIDTH);
    
    // DRV8343은 CS Low일 때 활성화되므로 기본 STE 설정을 따름
    SPI_enableFIFO(SPIB_BASE);
    SPI_clearInterruptStatus(SPIB_BASE, SPI_INT_RXFF | SPI_INT_TXFF);
    SPI_enableModule(SPIB_BASE);

    EDIS;
    SPI_disableFIFO(SPIA_BASE);
    SPI_setEmulationMode(SPIA_BASE, SPI_EMULATION_FREE_RUN);
    SPI_enableModule(SPIA_BASE);
}

/* --- W6100 SPI Callback Wrappers --- */

/*
@function    void cs_sel(void)
@brief      W6100 칩 선택 (Chip Select Low) 외부 SPI 콜백
@param      void
@return     void
*/
void cs_sel(void) 
{ 
    // GPIO W6100 칩을 활성화(Select) 합니다.
    GPIO_writePin(SPIA_W6100_CS_PIN, 0U); 
}

/*
@function    void cs_desel(void)
@brief      W6100 칩 해제 (Chip Select High) 외부 SPI 콜백
@param      void
@return     void
*/
void cs_desel(void) 
{ 
    // GPIO W6100 칩을 비활성화(Deselect) 합니다.
    GPIO_writePin(SPIA_W6100_CS_PIN, 1U); 
}

/*
@function    uint8_t spi_read_byte(void)
@brief      W6100 외부 SPI 데이터 1바이트 읽기 콜백
@param      void
@return     uint8_t (수신된 데이터)
*/
uint8_t spi_read_byte(void) 
{
    // C2000 SPI 모듈에서 데이터를 읽기 위해 더미 데이터(0x0000)를 송신하여 클럭을 발생시킵니다.
    SPI_writeDataBlockingNonFIFO(SPIA_BASE, 0x0000);
    // 송수신이 완료되면 수신 버퍼에서 데이터를 읽어와 하위 8비트만 반환합니다.
    return (uint8_t)(SPI_readDataBlockingNonFIFO(SPIA_BASE) & 0xFF);
}

/*
@function    void spi_write_byte(uint8_t wb)
@brief      W6100 외부 SPI 데이터 1바이트 쓰기 콜백
@param      wb: 전송할 8비트 데이터
@return     void
*/
void spi_write_byte(uint8_t wb) 
{
    // C2000 SPI의 Non-FIFO 8비트 송신 시에는 데이터를 SPITXBUF의 상위 바이트(좌측)에 정렬해야 합니다.
    SPI_writeDataBlockingNonFIFO(SPIA_BASE, ((uint16_t)wb) << 8);
    // 송신에 의해 발생한 수신 데이터를 읽어서 버퍼(SPIRXBUF)를 비워줍니다. (오버런 에러 방지)
    SPI_readDataBlockingNonFIFO(SPIA_BASE); 
}

/* --- FRAM (SPI-D) Functions --- */

/*
@function    static void InitSpid(void)
@brief      FRAM 통신용 SPI-D 모듈 초기화 및 GPIO 설정
@param      void
@return     static void
@remark
    - GPIO 91(SIMO), 92(SOMI), 93(CLK)를 SPI-D 기능으로 할당합니다.
    - GPIO 94를 CS 핀(출력)으로 설정합니다.
    - Master 모드, Mode 3(POL1PHA1, 또는 POL1PHA0 참고코드 기준), 1MHz 속도 및 8비트 워드 규격으로 초기화합니다.
*/
static void InitSpid(void)
{
    EALLOW;
 
    // Pin Set
    GPIO_setPinConfig(GPIO_91_SPID_SIMO);
    GPIO_setPadConfig(SPID_FRAM_SIMO_PIN, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(SPID_FRAM_SIMO_PIN, GPIO_QUAL_ASYNC);
 
    GPIO_setPinConfig(GPIO_92_SPID_SOMI);
    GPIO_setPadConfig(SPID_FRAM_SOMI_PIN, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(SPID_FRAM_SOMI_PIN, GPIO_QUAL_ASYNC);
 
    GPIO_setPinConfig(GPIO_93_SPID_CLK);
    GPIO_setPadConfig(SPID_FRAM_CLK_PIN, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(SPID_FRAM_CLK_PIN, GPIO_QUAL_ASYNC);
 
    GPIO_setPinConfig(GPIO_94_GPIO94);
    GPIO_setPadConfig(SPID_FRAM_CS_PIN, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(SPID_FRAM_CS_PIN, GPIO_QUAL_SYNC);
    GPIO_setDirectionMode(SPID_FRAM_CS_PIN, GPIO_DIR_MODE_OUT);
    GPIO_setMasterCore(SPID_FRAM_CS_PIN, GPIO_CORE_CPU1);
 
    // Spi Init. Mode-3(POL1PHA0 in C2000)
    SPI_disableModule(SPID_BASE);
    SPI_setConfig(SPID_BASE, DEVICE_LSPCLK_FREQ, SPI_PROT_POL1PHA0, SPI_MODE_MASTER, SPID_FRAM_BAUDRATE, SPID_FRAM_DATA_WIDTH);
    SPI_disableFIFO(SPID_BASE);
    SPI_setEmulationMode(SPID_BASE, SPI_EMULATION_STOP_AFTER_TRANSMIT);
    SPI_enableModule(SPID_BASE);
 
    EDIS;
 
    // 기본 CS High 상태 유지
    Spid_CsHigh();
}

/*
@function    void Spid_CsLow(void)
@brief      FRAM CS 핀을 Low 상태로 만듭니다.
@param      void
@return     void
*/
void Spid_CsLow(void)
{
    GPIO_writePin(SPID_FRAM_CS_PIN, 0u);
}

/*
@function    void Spid_CsHigh(void)
@brief      FRAM CS 핀을 High 상태로 만듭니다.
@param      void
@return     void
*/
void Spid_CsHigh(void)
{
    GPIO_writePin(SPID_FRAM_CS_PIN, 1u);
}

/*
@function    uint16_t Spid_Transmit(uint16_t data)
@brief      SPI-D 모듈을 사용하여 1바이트 데이터를 송수신합니다.
@param      data : 전송할 8비트 데이터
@return     수신된 8비트 데이터
@remark
    - 8비트 Non-FIFO 모드에서는 전송 데이터를 상위 바이트(<< 8)에 배치해야 합니다.
*/
uint16_t Spid_Transmit(uint16_t data)
{
    // 데이터를 송신 (상위 8비트로 정렬)
    SPI_writeDataBlockingNonFIFO(SPID_BASE, data << 8u);
    
    // 수신된 데이터를 읽음 (하위 8비트 마스킹)
    return (SPI_readDataBlockingNonFIFO(SPID_BASE) & 0xFFu);
}
