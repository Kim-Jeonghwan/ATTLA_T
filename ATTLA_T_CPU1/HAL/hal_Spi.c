/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : hal_Spi.c
    Version          : 00.01
    Description      : SSI 엔코더 및 W6100 통신용 SPI 하드웨어 제어
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 09. (함수명에서 hal_ 접두어 제거)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 
 * 
 */


/* DESCRIPTION
 * 
 * 
 */


/* ************************** [[   include  ]]  *********************************************************** */
#include "hal_Spi.h"

/* ************************** [[   define   ]]  *********************************************************** */
// #define SSI_SIMO_SPIB	63u // SPI SIMOB
#define ENCODER_SOMI_GPIC	51u // SPI SOMIC
#define ENCODER_CLK_GPIC	52u // SPI CLKC
// #define SSI_CS			66u // Chip Select



/* ************************** [[   global   ]]  *********************************************************** */


/* ************************** [[   static prototype  ]]  ************************************************** */

static void InitSpia(void);
static void InitSpic(void);
static void InitSpid(void);



/* ************************** [[  function  ]]  *********************************************************** */
/*
@funtion    void Initial_SPI(void)
@brief      SPI 드라이버 초기화
@param      void
@return     void
@remark 
    - W6100용 SPI-A 모듈과 SSI 엔코더용 SPI-C 모듈 초기화를 호출합니다.
    - FRAM용 SPI-D 모듈 초기화를 호출합니다.
*/
void Initial_SPI(void) {
	InitSpia(); // W6100
    InitSpic(); // SSI
    InitSpid(); // FRAM
}


/*
@funtion    static void InitSpic(void)
@brief      SSI 엔코더 통신용 SPI-C 모듈 초기화 및 GPIO 설정
@param      void
@return     static void
@remark
    - GPIO 51(SOMI), 52(CLK)를 SPI-C 기능으로 할당합니다.
    - Master 모드, Mode 2(POL1PHA0), 1MHz 속도 및 16비트 워드 규격으로 초기화합니다.
*/
static void InitSpic(void)
{
    EALLOW; // 보호 레지스터 쓰기 허용

    // 핀 설정
    // GPIO_setPinConfig(GPIO_63_SPIB_SIMO);
    // GPIO_setPadConfig(SSI_SIMO_SPIB, GPIO_PIN_TYPE_STD);
    // GPIO_setQualificationMode(SSI_SIMO_SPIB, GPIO_QUAL_ASYNC);

    GPIO_setPinConfig(GPIO_51_SPIC_SOMI);
    GPIO_setPadConfig(ENCODER_SOMI_GPIC, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(ENCODER_SOMI_GPIC, GPIO_QUAL_ASYNC);

    GPIO_setPinConfig(GPIO_52_SPIC_CLK);
    GPIO_setPadConfig(ENCODER_CLK_GPIC, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(ENCODER_CLK_GPIC, GPIO_QUAL_ASYNC);

    // GPIO_setPinConfig(GPIO_66_GPIO66);
    // GPIO_setPadConfig(SSI_CS, GPIO_PIN_TYPE_STD);
    // GPIO_setQualificationMode(SSI_CS, GPIO_QUAL_SYNC);
    // GPIO_setDirectionMode(SSI_CS, GPIO_DIR_MODE_OUT);
    // GPIO_setMasterCore(SSI_CS, GPIO_CORE_CPU1);



	// SPI 초기화. 1MHz SPICLK, Mode-2(POL1PHA0), 16비트 워드 크기 설정.
    SPI_disableModule(SPIC_BASE);
    SPI_setConfig(SPIC_BASE, 
					DEVICE_LSPCLK_FREQ, 
					SPI_PROT_POL1PHA0,                                          // SSI엔코더는 보통 클럭이 High로 대기하다가 첫 번째 하강 엣지에서 데이터를 내보내는 Mode 2 나 Mode 3 많이 씀 (현재 모드2)
					SPI_MODE_MASTER, 
					1000000u,                                                   // 일단 1MHz(260126) - 필요 시 10MHz로 변경
					16);                                                        // 8 비트 : 8, 16 비트 : 16
    SPI_disableFIFO(SPIC_BASE);
    SPI_setEmulationMode(SPIC_BASE, SPI_EMULATION_STOP_AFTER_TRANSMIT);
    SPI_enableModule(SPIC_BASE);

    EDIS;   // 보호 레지스터 쓰기 금지
}

/*
@funtion    static void InitSpia(void)
@brief      W6100 이더넷 통신용 SPI-A 모듈 초기화
@param      void
@return     static void
@remark
    - Master 모드, Mode 0(POL0PHA0), 20MHz 속도 및 8비트 워드 규격으로 초기화합니다.
*/
static void InitSpia(void)
{
    // SPI 초기화. 20MHz SPICLK, Mode-0(POL0PHA0), 8비트 워드 크기 설정.
    SPI_disableModule(SPIA_BASE);
    SPI_setConfig(SPIA_BASE, 
					DEVICE_LSPCLK_FREQ, 
					SPI_PROT_POL0PHA0, // WIZnet은 Mode 0 또는 Mode 3 지원
					SPI_MODE_MASTER, 
					20000000u,         // 20MHz 클럭
					8);                // 8비트 통신
    SPI_disableFIFO(SPIA_BASE);
    SPI_setEmulationMode(SPIA_BASE, SPI_EMULATION_FREE_RUN);
    SPI_enableModule(SPIA_BASE);
}

/* --- W6100 SPI Callback Wrappers --- */
void cs_sel(void) 
{ 
    // GPIO 19 핀을 Low 상태로 출력하여 W6100 칩을 활성화(Select) 합니다.
    GPIO_writePin(19U, 0U); 
}

void cs_desel(void) 
{ 
    // GPIO 19 핀을 High 상태로 출력하여 W6100 칩을 비활성화(Deselect) 합니다.
    GPIO_writePin(19U, 1U); 
}

uint8_t spi_read_byte(void) 
{
    // C2000 SPI 모듈에서 데이터를 읽기 위해 더미 데이터(0x0000)를 송신하여 클럭을 발생시킵니다.
    SPI_writeDataBlockingNonFIFO(SPIA_BASE, 0x0000);
    // 송수신이 완료되면 수신 버퍼에서 데이터를 읽어와 하위 8비트만 반환합니다.
    return (uint8_t)(SPI_readDataBlockingNonFIFO(SPIA_BASE) & 0xFF);
}

void spi_write_byte(uint8_t wb) 
{
    // C2000 SPI의 Non-FIFO 8비트 송신 시에는 데이터를 SPITXBUF의 상위 바이트(좌측)에 정렬해야 합니다.
    SPI_writeDataBlockingNonFIFO(SPIA_BASE, ((uint16_t)wb) << 8);
    // 송신에 의해 발생한 수신 데이터를 읽어서 버퍼(SPIRXBUF)를 비워줍니다. (오버런 에러 방지)
    SPI_readDataBlockingNonFIFO(SPIA_BASE); 
}

/* --- FRAM (SPI-D) Functions --- */

/*
@funtion    static void InitSpid(void)
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
    GPIO_setPadConfig(91u, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(91u, GPIO_QUAL_ASYNC);

    GPIO_setPinConfig(GPIO_92_SPID_SOMI);
    GPIO_setPadConfig(92u, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(92u, GPIO_QUAL_ASYNC);

    GPIO_setPinConfig(GPIO_93_SPID_CLK);
    GPIO_setPadConfig(93u, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(93u, GPIO_QUAL_ASYNC);

    GPIO_setPinConfig(GPIO_94_GPIO94);
    GPIO_setPadConfig(FRAM_CS_GPIO, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(FRAM_CS_GPIO, GPIO_QUAL_SYNC);
    GPIO_setDirectionMode(FRAM_CS_GPIO, GPIO_DIR_MODE_OUT);
    GPIO_setMasterCore(FRAM_CS_GPIO, GPIO_CORE_CPU1);

    // Spi Init. Use a 1MHz SPICLK, Mode-3(POL1PHA0 in C2000), and 8-bit word size.
    SPI_disableModule(SPID_BASE);
    SPI_setConfig(SPID_BASE, DEVICE_LSPCLK_FREQ, SPI_PROT_POL1PHA0, SPI_MODE_MASTER, 1000000u, 8u);
    SPI_disableFIFO(SPID_BASE);
    SPI_setEmulationMode(SPID_BASE, SPI_EMULATION_STOP_AFTER_TRANSMIT);
    SPI_enableModule(SPID_BASE);

    EDIS;

    // 기본 CS High 상태 유지
    Spid_CsHigh();
}

/*
@funtion    void Spid_CsLow(void)
@brief      FRAM CS 핀을 Low 상태로 만듭니다.
@param      void
@return     void
*/
void Spid_CsLow(void)
{
    GPIO_writePin(FRAM_CS_GPIO, 0u);
}

/*
@funtion    void Spid_CsHigh(void)
@brief      FRAM CS 핀을 High 상태로 만듭니다.
@param      void
@return     void
*/
void Spid_CsHigh(void)
{
    GPIO_writePin(FRAM_CS_GPIO, 1u);
}

/*
@funtion    uint16_t Spid_Transmit(uint16_t data)
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
