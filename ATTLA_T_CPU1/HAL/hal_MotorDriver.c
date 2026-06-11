/**********************************************************************
 Nexcom Co., Ltd.
 Filename         : hal_MotorDriver.c
 Version          : 00.00
 Description      : DRV8343 모터 드라이버 하드웨어 초기화 및 SPI 통신
 Programmer       : Kim Jeonghwan
 Last Updated     : 2026. 06. 11. (모터 방향 제어 추상화 함수 추가)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 11. - 모터 방향 제어용 MotorDriver_SetDir() 함수 추가
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */


#include "hal_MotorDriver.h"

//---------------------------------------------------------------------------
// MotorDriver_Init_Hardware
//---------------------------------------------------------------------------
void MotorDriver_Init_Hardware(void)
{
    //-----------------------------------------------------------------------
    // 1. GPIO 핀 설정 (SPI-B)
    //-----------------------------------------------------------------------
    // GPIO 58: SPIB_CLK
    GPIO_setPinConfig(GPIO_58_SPIB_CLK);
    GPIO_setDirectionMode(HAL_MOTORDRIVER_SPI_CLK_PIN, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(HAL_MOTORDRIVER_SPI_CLK_PIN, GPIO_PIN_TYPE_STD);

    // GPIO 59: SPIB_STE (Chip Select)
    GPIO_setPinConfig(GPIO_59_SPIB_STEN);
    GPIO_setDirectionMode(HAL_MOTORDRIVER_SPI_STE_PIN, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(HAL_MOTORDRIVER_SPI_STE_PIN, GPIO_PIN_TYPE_STD);

    // GPIO 60: SPIB_SIMO
    GPIO_setPinConfig(GPIO_60_SPIB_SIMO);
    GPIO_setDirectionMode(HAL_MOTORDRIVER_SPI_SIMO_PIN, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(HAL_MOTORDRIVER_SPI_SIMO_PIN, GPIO_PIN_TYPE_STD);

    // GPIO 61: SPIB_SOMI
    GPIO_setPinConfig(GPIO_61_SPIB_SOMI);
    GPIO_setDirectionMode(HAL_MOTORDRIVER_SPI_SOMI_PIN, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(HAL_MOTORDRIVER_SPI_SOMI_PIN, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(HAL_MOTORDRIVER_SPI_SOMI_PIN, GPIO_QUAL_ASYNC);

    //-----------------------------------------------------------------------
    // 2. SPI-B 모듈 초기화 (DRV8343 통신 규격)
    //-----------------------------------------------------------------------
    SPI_disableModule(SPIB_BASE);

    // DRV8343은 일반적으로 Data on Falling Edge, Setup on Rising Edge (CPOL=0, CPHA=1)
    // 또는 CPOL=0, CPHA=0을 지원합니다. (데이터시트 확인 필요, 통상 1MHz 16-bit 사용)
    SPI_setConfig(SPIB_BASE, DEVICE_LSPCLK_FREQ, SPI_PROT_POL0PHA1,
                  SPI_MODE_MASTER, 1000000, 16);
    
    // DRV8343은 CS Low일 때 활성화되므로 기본 STE 설정을 따름
    SPI_enableFIFO(SPIB_BASE);
    SPI_clearInterruptStatus(SPIB_BASE, SPI_INT_RXFF | SPI_INT_TXFF);
    
    SPI_enableModule(SPIB_BASE);

    // DRV8343 wake up 대기
    DEVICE_DELAY_US(2000U);

    //-----------------------------------------------------------------------
    // 3. DRV8343 1x PWM 모드 설정 (Control 2 Register)
    //-----------------------------------------------------------------------
    uint16_t ctrl2 = MotorDriver_ReadReg(DRV8343_REG_CONTROL_2);
    
    // Bits 6:5 (PWM_MODE) 클리어 후 10b (1x PWM) 설정
    ctrl2 &= ~(0x03U << 5);
    ctrl2 |= DRV8343_CTRL2_PWM_MODE_1X;
    
    MotorDriver_WriteReg(DRV8343_REG_CONTROL_2, ctrl2);
    DEVICE_DELAY_US(10U);
}
//---------------------------------------------------------------------------
// MotorDriver_ReadReg
//---------------------------------------------------------------------------
uint16_t MotorDriver_ReadReg(uint16_t addr)
{
    // DRV8343 SPI Read Frame: Bit 15 = 1 (Read), Bits 14:11 = Addr, Bits 10:0 = Don't care
    uint16_t txWord = (1U << 15) | ((addr & 0x0F) << 11);
    uint16_t rxWord = 0;
    
    SPI_writeDataBlockingNonFIFO(SPIB_BASE, txWord);
    rxWord = SPI_readDataBlockingNonFIFO(SPIB_BASE);
    
    // 수신된 데이터 중 하위 11비트 반환
    return (rxWord & 0x07FF);
}

//---------------------------------------------------------------------------
// MotorDriver_WriteReg
//---------------------------------------------------------------------------
void MotorDriver_WriteReg(uint16_t addr, uint16_t data)
{
    // DRV8343 SPI Write Frame: Bit 15 = 0 (Write), Bits 14:11 = Addr, Bits 10:0 = Data
    uint16_t txWord = ((addr & 0x0F) << 11) | (data & 0x07FF);
    
    SPI_writeDataBlockingNonFIFO(SPIB_BASE, txWord);
    
    // Write 시에는 응답 데이터를 무시하지만 FIFO를 비우기 위해 읽어줌
    SPI_readDataBlockingNonFIFO(SPIB_BASE);
}

//---------------------------------------------------------------------------
// MotorDriver_SetDir
//---------------------------------------------------------------------------
/*
@funtion    void MotorDriver_SetDir(bool bForward)
@brief      모터 정/역방향 GPIO 출력 설정
@param      bForward: true(정방향), false(역방향)
@return     void
*/
void MotorDriver_SetDir(bool bForward)
{
    if (bForward)
    {
        GPIO_writePin(DRV8343_DIR_PIN, 1U);
    }
    else
    {
        GPIO_writePin(DRV8343_DIR_PIN, 0U);
    }
}
