/**********************************************************************
 Nexcom Co., Ltd.
 Filename         : hal_Encoder.c
 Version          : 00.03
 Description      : AksIM-2 엔코더 제어를 위한 HAL (하드웨어 초기화 및 SPI 통신)
 Programmer       : Kim Jeonghwan
 Last Updated     : 2026. 06. 11. (주석 표준화 및 레거시 코드 정리)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 64-bit FIFO 연속 수신 구조 적용 (BiSS 타임아웃 회피) 및 100ms 안정화 지연 추가
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */


#include "hal_Encoder.h"

/**
 * @function Encoder_Init_Hardware
 * @brief    엔코더용 SPI-C 포트 및 FIFO, 전원 안정화 대기 하드웨어 설정
 * @param    void
 * @return   void
 */
void Encoder_Init_Hardware(void)
{
    // 센서 전원 안정화 대기 (100ms)
    DEVICE_DELAY_US(100000);

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
    
    // SPI 마스터 모드 설정, 2.5MHz 통신
    // Data on Falling Edge (CPOL=0, CPHA=1)
    SPI_setConfig(SPIC_BASE, DEVICE_LSPCLK_FREQ, SPI_PROT_POL0PHA1,
                  SPI_MODE_MASTER, 2500000, 16); // 2.5MHz, 16-bit word
    
    // FIFO 활성화
    SPI_enableFIFO(SPIC_BASE);
    SPI_clearInterruptStatus(SPIC_BASE, SPI_INT_RXFF | SPI_INT_TXFF);
    SPI_setFIFOInterruptLevel(SPIC_BASE, SPI_FIFO_TXEMPTY, SPI_FIFO_RX4);
    
    // SPI 모듈 활성화
    SPI_enableModule(SPIC_BASE);
}

/**
 * @function Encoder_ReadSpiData
 * @brief    RX FIFO를 활용하여 엔코더로부터 64비트 원시 데이터 블로킹 수신
 * @param    void
 * @return   수신된 64비트 원시 데이터 전체
 */
uint64_t Encoder_ReadSpiData(void)
{
    uint16_t w1, w2, w3, w4;
    uint64_t rawData64;
    
    // BiSS 타임아웃(13.5us) 방어를 위해 FIFO를 사용하여 16비트 데이터를 4번 연속 푸시
    SPI_writeDataBlockingFIFO(SPIC_BASE, 0xFFFF);
    SPI_writeDataBlockingFIFO(SPIC_BASE, 0xFFFF);
    SPI_writeDataBlockingFIFO(SPIC_BASE, 0xFFFF);
    SPI_writeDataBlockingFIFO(SPIC_BASE, 0xFFFF);
    
    // RX FIFO에 4개의 데이터(총 64비트)가 모두 찰 때까지 대기
    while(SPI_getRxFIFOStatus(SPIC_BASE) < SPI_FIFO_RX4)
    {
        // Blocking wait
    }
    
    // 4개의 16비트 워드 일괄 수신
    w1 = SPI_readDataBlockingFIFO(SPIC_BASE);
    w2 = SPI_readDataBlockingFIFO(SPIC_BASE);
    w3 = SPI_readDataBlockingFIFO(SPIC_BASE);
    w4 = SPI_readDataBlockingFIFO(SPIC_BASE);
    
    // 64비트 변수로 통합
    rawData64 = ((uint64_t)w1 << 48) | ((uint64_t)w2 << 32) | ((uint64_t)w3 << 16) | (uint64_t)w4;
    
    return rawData64;
}
