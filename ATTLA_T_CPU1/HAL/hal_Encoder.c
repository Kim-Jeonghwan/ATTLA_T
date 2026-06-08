/**********************************************************************
 Nexcom Co., Ltd.
 Filename         : hal_Encoder.c
 Version          : 00.01
 Description      : AksIM-2 엔코더 제어를 위한 HAL (하드웨어 초기화 및 pm_bissc 연동)
 Programmer       : Kim Jeonghwan
 Last Updated     : 2026. 06. 09. (신규 생성)
**********************************************************************/

#include "hal_Encoder.h"
#include "pm_bissc_include.h"

//---------------------------------------------------------------------------
// 전역 변수
//---------------------------------------------------------------------------
PM_bissc_scdStruct encoderScdParams;
PM_bissc_cdStruct encoderCdParams;
PM_bissc_encoderStruct encoderData;

// 외부 모듈(pm_bissc_source.c)에서 정의된 수신 버퍼
extern volatile uint32_t scdRxData[PM_BISSC_SPI_FIFO_MAX_LEVEL];

//---------------------------------------------------------------------------
// 내부 함수 프로토타입
//---------------------------------------------------------------------------
__interrupt void Encoder_SpiRxISR(void);

//---------------------------------------------------------------------------
// Encoder_Init_Hardware
//---------------------------------------------------------------------------
void Encoder_Init_Hardware(void)
{
    //-----------------------------------------------------------------------
    // 1. GPIO 핀 설정
    //-----------------------------------------------------------------------
    // GPIO 25: SPI-B SOMI (엔코더 데이터 수신)
    GPIO_setPinConfig(GPIO_25_SPIB_SOMI);
    GPIO_setDirectionMode(HAL_ENC_DATA_PIN, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(HAL_ENC_DATA_PIN, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(HAL_ENC_DATA_PIN, GPIO_QUAL_ASYNC);
    
    // GPIO 26: OUTPUT X-BAR 3 (CLB에서 생성한 MA 클럭 송신)
    GPIO_setPinConfig(GPIO_26_OUTPUTXBAR3);
    GPIO_setDirectionMode(HAL_ENC_CLK_PIN, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(HAL_ENC_CLK_PIN, GPIO_PIN_TYPE_STD);
    
    //-----------------------------------------------------------------------
    // 2. 내부 라우팅 설정 (X-BAR)
    //-----------------------------------------------------------------------
    // CLB2_OUT4를 OUTPUT X-BAR 3로 연결하여 GPIO 26 핀으로 출력되도록 구성
    XBAR_setOutputMuxConfig(OUTPUTXBAR_BASE, XBAR_OUTPUT3, XBAR_OUT_MUX05_CLB2_OUT4);
    XBAR_enableOutputMux(OUTPUTXBAR_BASE, XBAR_OUTPUT3, XBAR_MUX05);
    
    //-----------------------------------------------------------------------
    // 3. pm_bissc 라이브러리 파라미터 초기화
    //-----------------------------------------------------------------------
    // BiSS-C 프로토콜 기본 설정 (AksIM-2 사양)
    encoderScdParams.crcBits = 6;              // CRC 폴리노미얼 크기
    encoderScdParams.crcPoly = 0x43;           // 0x43 (X^6 + X + 1)
    encoderScdParams.crcStart = 0;             // 초기값 0
    encoderScdParams.positionMTBits = 16;      // 멀티턴 카운터 16-bit
    encoderScdParams.positionSTBits = 18;      // 싱글턴 분해능 18-bit
    
    encoderCdParams.crcBits = 4;
    encoderCdParams.crcPoly = 0x03;            // 0x03 (X^4 + X + 1)
    encoderCdParams.crcStart = 0;
    
    // 라이브러리의 파라미터 셋업 함수 호출
    PM_bissc_initParams(&encoderScdParams, &encoderCdParams);
    
    //-----------------------------------------------------------------------
    // 4. CLB 타일 초기화 및 주파수 설정
    //-----------------------------------------------------------------------
    // CLB 타일 초기화
    PM_bissc_setupPeriph();
    
    // 통신 주파수 설정 (예: 5MHz)
    // 시스템 200MHz / (5MHz * 2) = freqDiv 값 설정 (20)
    // 자세한 공식은 SYSCLK에 따라 다를 수 있으나 통상적으로 적용
    PM_bissc_setFreq(20);

    //-----------------------------------------------------------------------
    // 5. 인터럽트 설정 (SPI RX)
    //-----------------------------------------------------------------------
    Interrupt_register(PM_BISSC_INT_SPI_RX, &Encoder_SpiRxISR);
    Interrupt_enable(PM_BISSC_INT_SPI_RX);
}

//---------------------------------------------------------------------------
// Encoder_SpiRxISR (SPI 수신 인터럽트)
//---------------------------------------------------------------------------
// BiSS-C 통신 완료 시 (FIFO 수신 완료) 발생하여 수신된 데이터를 버퍼에 저장
__interrupt void Encoder_SpiRxISR(void)
{
    uint16_t i;
    
    // SPI FIFO에 들어온 데이터를 읽어서 scdRxData 버퍼에 저장
    for(i = 0; i < encoderScdParams.spiFIFOLevel; i++)
    {
        scdRxData[i] = SPI_readDataNonBlocking(PM_BISSC_SPI);
    }
    
    // 데이터 수신 완료 플래그 세트
    encoderScdParams.dataReady = true;

    // SPI RX 인터럽트 클리어 및 그룹 ACK
    SPI_clearInterruptStatus(PM_BISSC_SPI, SPI_INT_RXFF);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP6); // SPI_B RX는 PIE Group 6
}
