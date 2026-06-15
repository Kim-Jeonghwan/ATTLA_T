/**********************************************************************
 Nexcom Co., Ltd.
 Filename         : hal_MotorDriver.h
 Version          : 00.01
 Description      : DRV8343 모터 드라이버 하드웨어 초기화 (SPI-B)
 Programmer       : Kim Jeonghwan
 Last Updated     : 2026. 06. 15. (SPI 핀 매크로 제거 및 hal_Spi.h 통합)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 15. - SPI 핀 매크로 제거 및 hal_Spi.h로 통합 이관
 * 2026. 06. 11. - DRV_ENABLE GPIO 2 제어 로직 추가 (unresolved symbol 에러 해결)
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 모터 방향 제어용 함수 원형 및 매크로 추가
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */


#ifndef HAL_MOTORDRIVER_H_
#define HAL_MOTORDRIVER_H_

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

// (SPI-B 관련 핀 정의는 hal_Spi.h 로 이관됨)

//---------------------------------------------------------------------------
// 모터 제어용 하드웨어 핀 매크로
//---------------------------------------------------------------------------
#define DRV8343_EN_GATE_PIN               2U   // DRV8343 활성화 핀 (GPIO 2, Active High)
#define DRV8343_DIR_PIN                   3U   // 모터 회전 방향 제어 핀 (GPIO 3, Active High)

//---------------------------------------------------------------------------
// DRV8343 Register Addresses & Commands
//---------------------------------------------------------------------------
#define DRV8343_REG_FAULT_STATUS_1      0x00U
#define DRV8343_REG_FAULT_STATUS_2      0x01U
#define DRV8343_REG_CONTROL_1           0x02U
#define DRV8343_REG_CONTROL_2           0x03U
#define DRV8343_REG_CONTROL_3           0x04U
#define DRV8343_REG_CONTROL_4           0x05U
#define DRV8343_REG_CONTROL_5           0x06U
#define DRV8343_REG_CONTROL_6           0x07U

// 1x PWM Mode Setting (PWM_MODE = 10b in Control 2 Register, Bits 6:5)
#define DRV8343_CTRL2_PWM_MODE_1X       (0x02U << 5)

//---------------------------------------------------------------------------
// 함수 프로토타입
//---------------------------------------------------------------------------

/**
 * @brief      DRV8343 모터 드라이버 SPI-B GPIO 설정, 모듈 설정 및 1x PWM 모드 설정
 * @param      void
 * @return     void
 */
extern void MotorDriver_Init_Hardware(void);

/**
 * @brief      DRV8343 레지스터 SPI 읽기 연산 수행
 * @param      addr: 읽어올 레지스터 주소
 * @return     수신된 레지스터 값 (하위 11비트 유효 데이터)
 */
extern uint16_t MotorDriver_ReadReg(uint16_t addr);

/**
 * @brief      DRV8343 레지스터 SPI 쓰기 연산 수행
 * @param      addr: 기록할 레지스터 주소
 * @param      data: 기록할 레지스터 값 (하위 11비트 유효 데이터)
 * @return     void
 */
extern void MotorDriver_WriteReg(uint16_t addr, uint16_t data);

/**
 * @brief      모터 정/역방향 GPIO 출력 설정
 * @param      bForward: true(정방향), false(역방향)
 * @return     void
 */
extern void MotorDriver_SetDir(bool bForward);

/**
 * @brief      DRV8343 모터 드라이버 활성화/비활성화 제어 (EN_GATE GPIO 핀 제어)
 * @param      enable: true(활성화), false(비활성화)
 * @return     void
 */
extern void MotorDriver_Enable(bool enable);

#ifdef __cplusplus
}
#endif

#endif /* HAL_MOTORDRIVER_H_ */
