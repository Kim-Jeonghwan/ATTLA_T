/**********************************************************************
 * Nexcom Co., Ltd.
 * Filename         : hal_Encoder.h
 * Version          : 00.02
 * Description      : AksIM-2 엔코더 제어를 위한 HAL (하드웨어 초기화 및 SPI 통신)
 * Programmer       : Kim Jeonghwan
 * Last Updated     : 2026. 06. 11. (주석 표준화 및 레거시 코드 정리)
 **********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 64비트 수신 반환형 변경 및 불필요 변수 제거
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */


#ifndef HAL_ENCODER_H_
#define HAL_ENCODER_H_

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------------
// 매크로 정의
//---------------------------------------------------------------------------
#define ENC_DATA_PIN        51      // SPIC_SOMI
#define ENC_CLK_PIN         52      // SPIC_CLK

//---------------------------------------------------------------------------
// 함수 프로토타입
//---------------------------------------------------------------------------
/**
 * @brief      엔코더용 SPI-C 포트 및 FIFO, 전원 안정화 대기 하드웨어 설정
 * @param      void
 * @return     void
 */
extern void Encoder_Init_Hardware(void);

/**
 * @brief      RX FIFO를 활용하여 엔코더로부터 64비트 원시 데이터 블로킹 수신
 * @param      void
 * @return     uint64_t 수신된 64비트 원시 데이터 전체
 */
extern uint64_t Encoder_ReadSpiData(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_ENCODER_H_ */
