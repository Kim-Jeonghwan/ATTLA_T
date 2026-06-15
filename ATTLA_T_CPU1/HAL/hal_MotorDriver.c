/**********************************************************************
 Nexcom Co., Ltd.
 Filename         : hal_MotorDriver.c
 Version          : 00.01
 Description      : DRV8343 모터 드라이버 하드웨어 초기화 및 SPI 통신
 Programmer       : Kim Jeonghwan
 Last Updated     : 2026. 06. 15. (SPI 초기화 로직 hal_Spi.c로 분리)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 15. - SPI 초기화 코드를 hal_Spi.c (InitSpib)로 분리 및 이관
 * 2026. 06. 11. - DRV_ENABLE GPIO 2 제어 로직 추가 (unresolved symbol 에러 해결)
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 모터 방향 제어용 MotorDriver_SetDir() 함수 추가
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */


#include "hal_MotorDriver.h"

/*
@function    void MotorDriver_Init_Hardware(void)
@brief      DRV8343 모터 드라이버 SPI-B GPIO 설정, 모듈 설정 및 1x PWM 모드 설정
@param      void
@return     void
*/
void MotorDriver_Init_Hardware(void)
{
    //-----------------------------------------------------------------------
    // 1. GPIO 핀 설정 (EN_GATE)
    //-----------------------------------------------------------------------
    // GPIO 2: DRV_ENABLE (Active High 출력)
    GPIO_setDirectionMode(DRV8343_EN_GATE_PIN, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(DRV8343_EN_GATE_PIN, GPIO_PIN_TYPE_STD);
    GPIO_setMasterCore(DRV8343_EN_GATE_PIN, GPIO_CORE_CPU1);

    // DRV8343 wake up 대기
    DEVICE_DELAY_US(2000U);

    //-----------------------------------------------------------------------
    // 2. DRV8343 1x PWM 모드 설정 (Control 2 Register)
    //-----------------------------------------------------------------------
    uint16_t ctrl2 = MotorDriver_ReadReg(DRV8343_REG_CONTROL_2);
    
    // Bits 6:5 (PWM_MODE) 클리어 후 10b (1x PWM) 설정
    ctrl2 &= ~(0x03U << 5);
    ctrl2 |= DRV8343_CTRL2_PWM_MODE_1X;
    
    MotorDriver_WriteReg(DRV8343_REG_CONTROL_2, ctrl2);
    DEVICE_DELAY_US(10U);
}

/*
@function    uint16_t MotorDriver_ReadReg(uint16_t addr)
@brief      DRV8343 레지스터 SPI 읽기 연산 수행
@param      addr: 읽어올 레지스터 주소
@return     수신된 레지스터 값 (하위 11비트 유효 데이터)
*/
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

/*
@function    void MotorDriver_WriteReg(uint16_t addr, uint16_t data)
@brief      DRV8343 레지스터 SPI 쓰기 연산 수행
@param      addr: 기록할 레지스터 주소
@param      data: 기록할 레지스터 값 (하위 11비트 유효 데이터)
@return     void
*/
void MotorDriver_WriteReg(uint16_t addr, uint16_t data)
{
    // DRV8343 SPI Write Frame: Bit 15 = 0 (Write), Bits 14:11 = Addr, Bits 10:0 = Data
    uint16_t txWord = ((addr & 0x0F) << 11) | (data & 0x07FF);
    
    SPI_writeDataBlockingNonFIFO(SPIB_BASE, txWord);
    
    // Write 시에는 응답 데이터를 무시하지만 FIFO를 비우기 위해 읽어줌
    SPI_readDataBlockingNonFIFO(SPIB_BASE);
}

/*
@function    void MotorDriver_SetDir(bool bForward)
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

/*
@function    void MotorDriver_Enable(bool enable)
@brief      DRV8343 모터 드라이버 활성화/비활성화 (EN_GATE 핀 제어)
@param      enable: true(활성화 - High), false(비활성화 - Low)
@return     void
*/
void MotorDriver_Enable(bool enable)
{
    if (enable)
    {
        GPIO_writePin(DRV8343_EN_GATE_PIN, 1U);
    }
    else
    {
        GPIO_writePin(DRV8343_EN_GATE_PIN, 0U);
    }
}
