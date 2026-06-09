/**********************************************************************
 Nexcom Co., Ltd.
 Filename         : hal_Encoder.c
 Version          : 00.01
 Description      : AksIM-2 엔코더 제어를 위한 HAL (하드웨어 초기화 및 pm_bissc 연동)
 Programmer       : Kim Jeonghwan
 Last Updated     : 2026. 06. 09. (신규 생성)
**********************************************************************/

#include "hal_Encoder.h"
#include "hal_Adc.h"

//---------------------------------------------------------------------------
// 전역 변수
//---------------------------------------------------------------------------
uint32_t encRawData = 0;

//---------------------------------------------------------------------------
// Encoder_Init_Hardware
//---------------------------------------------------------------------------
void Encoder_Init_Hardware(void)
{
    //-----------------------------------------------------------------------
    // 1. GPIO 핀 설정 (SPI-C)
    //-----------------------------------------------------------------------
    // GPIO 51: SPIC_SOMI (엔코더 데이터 수신)
    GPIO_setPinConfig(GPIO_51_SPIC_SOMI);
    GPIO_setDirectionMode(ENC_DATA_PIN, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(ENC_DATA_PIN, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(ENC_DATA_PIN, GPIO_QUAL_ASYNC);
    
    // GPIO 52: SPIC_CLK (엔코더 클럭 송신)
    GPIO_setPinConfig(GPIO_52_SPIC_CLK);
    GPIO_setDirectionMode(ENC_CLK_PIN, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(ENC_CLK_PIN, GPIO_PIN_TYPE_STD);
    
    //-----------------------------------------------------------------------
    // 2. SPI-C 모듈 초기화 (SSI 통신 규격)
    //-----------------------------------------------------------------------
    // SPI 리셋
    SPI_disableModule(SPIC_BASE);
    
    // SPI 마스터 모드 설정, 5MHz 통신 예시 (LSPCLK가 50MHz일 경우)
    // AksIM-2 등 SSI 엔코더는 보통 CPOL=0, CPHA=1 또는 CPOL=1, CPHA=0 등을 사용합니다. (확인 필요)
    // 여기서는 기본적으로 Data on Falling Edge (CPOL=0, CPHA=1) 로 설정합니다.
    SPI_setConfig(SPIC_BASE, DEVICE_LSPCLK_FREQ, SPI_PROT_POL0PHA1,
                  SPI_MODE_MASTER, 1000000, 16); // 1MHz, 16-bit word
    
    // FIFO 활성화
    SPI_enableFIFO(SPIC_BASE);
    SPI_clearInterruptStatus(SPIC_BASE, SPI_INT_RXFF | SPI_INT_TXFF);
    SPI_setFIFOInterruptLevel(SPIC_BASE, SPI_FIFO_TXEMPTY, SPI_FIFO_RX1);
    
    // SPI 모듈 활성화
    SPI_enableModule(SPIC_BASE);
}

//---------------------------------------------------------------------------
// Encoder_ReadSpiData
//---------------------------------------------------------------------------
// SSI 통신은 더미 데이터를 전송하며 클럭을 발생시켜 수신합니다.
uint32_t Encoder_ReadSpiData(void)
{
    uint16_t highWord, lowWord;
    
    // 32비트 프레임을 읽기 위해 16비트씩 두 번 전송 및 수신
    SPI_writeDataBlockingNonFIFO(SPIC_BASE, 0xFFFF);
    highWord = SPI_readDataBlockingNonFIFO(SPIC_BASE);
    
    SPI_writeDataBlockingNonFIFO(SPIC_BASE, 0xFFFF);
    lowWord = SPI_readDataBlockingNonFIFO(SPIC_BASE);
    
    encRawData = ((uint32_t)highWord << 16) | lowWord;
    
    return encRawData;
}

