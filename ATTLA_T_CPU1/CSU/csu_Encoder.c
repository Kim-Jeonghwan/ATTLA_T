/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Encoder.c
    Version          : 00.08
    Description      : AksIM-2 엔코더 어플리케이션 기능 처리 모듈
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 07. 01. (초기화 구문 상세 한글 주석 추가)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 07. 01. - 초기화 구문 상세 한글 주석 추가 (코딩 규칙 적용)
 * 2026. 06. 30. - FRAM CRC-16 계산(CCITT) 및 저장/로드 로직 추가
 * 2026. 06. 30. - Encoder_SetZero 시 안전 인터락(MotorCtrl_IsSafeToZeroSet) 검증 로직 반영
 * 2026. 06. 12. - 롤오버 및 스케일 매직넘버 상수화하여 헤더(.h)로 분리 (글로벌 룰 적용)
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 전역 변수를 stEncoderState 구조체(xEncoder)로 통합하여 네임스페이스 및 상태 관리 개선
 * 2026. 06. 11. - 컴파일러 표준(C89/C90)에 맞게 for 문 변수 선언 위치 변경 및 미사용 변수 경고 해결
 * 2026. 06. 11. - Encoder_LoadOffset 신규 작성 및 Encoder_Init 연동
 * 2026. 06. 11. - Encoder_SetZero 호출 시 FRAM 기록 연동
 * 2026. 06. 11. - Dynamic Parsing, 0x43 다항식 CRC 검증 로직 구현
 * 2026. 06. 11. - 34비트 오프셋 연산, 롤오버 처리 및 기계각 스케일링 함수 구현
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */


#include "csu_Encoder.h"

//---------------------------------------------------------------------------
// 전역 변수
//---------------------------------------------------------------------------
stEncoderState xEncoder;

//---------------------------------------------------------------------------
// 내부 함수 프로토타입
//---------------------------------------------------------------------------
static uint8_t Encoder_CalcCrc6(uint64_t data36);
static uint16_t Encoder_CalcCrc16(const uint8_t* data, uint16_t length);

/**
 * @function Encoder_Init
 * @brief    엔코더 상태 구조체 변수 초기화 및 하드웨어(SPI-C) 기동
 * @param    void
 * @return   void
 */
void Encoder_Init(void)
{
    // 구조체 명시적 초기화
    xEncoder.fullFrame = 0;      // 64비트 전체 수신 프레임 0으로 초기화
    xEncoder.rawPos = 0;         // 34비트 위치 원시값 0으로 초기화
    xEncoder.errBit = 0;         // 에러 비트 기본값 0(에러 상태) 초기화
    xEncoder.warnBit = 0;        // 경고 비트 기본값 0 초기화
    xEncoder.crcRecv = 0;        // 수신된 CRC 값 0으로 초기화
    xEncoder.crcCalc = 0;        // 자체 계산 CRC 값 0으로 초기화
    xEncoder.offset = 0;         // 영점 오프셋 값 0으로 초기화
    xEncoder.position = 0;       // 최종 연산된 위치값 0으로 초기화
    xEncoder.angleDeg = 0.0f;    // 환산 기계각 0.0도로 초기화
    xEncoder.isValid = false;    // 데이터 유효성 기본값 비유효(false)로 초기화

    // HAL 초기화 호출 (SPI-C 설정 및 100ms 지연)
    Encoder_Init_Hardware();
    
    // 초기화 직후 FRAM에서 기존에 저장된 오프셋 값을 불러와 즉각 연산에 반영
    Encoder_LoadOffset();
}

/**
 * @function Encoder_LoadOffset
 * @brief    FRAM 비휘발성 영역에서 기존 영점 오프셋(8바이트) 로드
 * @param    void
 * @return   void
 */
void Encoder_LoadOffset(void)
{
    uint64_t loadedOffset = 0;
    uint16_t i;
    uint8_t buffer[10];
    
    // 10바이트 읽기 (8바이트 오프셋 + 2바이트 CRC)
    for (i = 0; i < 10; i++)
    {
        buffer[i] = (uint8_t)(Fram_ReadByte(ENC_OFFSET_FRAM_ADDR + i) & 0xFF);
    }
    
    // 수신된 8바이트에 대해 CRC 계산
    uint16_t calcCrc = Encoder_CalcCrc16(buffer, 8);
    uint16_t rcvCrc = ((uint16_t)buffer[8] << 8) | buffer[9];
    
    if (calcCrc == rcvCrc)
    {
        for (i = 0; i < 8; i++)
        {
            loadedOffset |= ((uint64_t)buffer[i] << (i * 8));
        }
        xEncoder.offset = loadedOffset;
    }
    else
    {
        // CRC 오류 시 안전을 위해 0으로 초기화
        xEncoder.offset = 0;
    }
}

/**
 * @function Encoder_UpdatePosition
 * @brief    SPI-C 통신을 통해 엔코더 데이터를 수신하고 위치 및 기계각 환산
 * @param    void
 * @return   void
 * @remark   100us 시스템 운용 파이프라인 주기 내에서 호출됨
 */
void Encoder_UpdatePosition(void)
{
    // SPI-C 통신을 통해 64비트 원시(Raw) 데이터 수신
    uint64_t rawData64 = Encoder_ReadSpiData();
    xEncoder.fullFrame = rawData64;
    int16_t startBitPos = -1;
    
    int16_t i;
    // 1. Dynamic Parsing (Start 비트 탐색)
    // MSB부터 LSB 방향으로 처음 등장하는 '1'의 위치를 찾음
    for (i = 63; i >= 0; i--)
    {
        if ((rawData64 >> i) & 1ULL)
        {
            startBitPos = i;
            break;
        }
    }
    
    // Start 비트가 존재하고, 이후 42비트(CDS 1 + Pos 34 + Err 1 + Warn 1 + CRC 6) 데이터 확보 가능 시
    if (startBitPos >= 42)
    {
        // 2. 데이터 추출
        // startBitPos: Start (1)
        // startBitPos - 1: CDS (1, 무시)
        // startBitPos - 2 ~ startBitPos - 35: Position (34 bits)
        uint64_t extPos = (rawData64 >> (startBitPos - 35)) & 0x3FFFFFFFFULL;
        xEncoder.rawPos = extPos;
        
        // startBitPos - 36: Error (1 bit)
        uint8_t errBit = (rawData64 >> (startBitPos - 36)) & 0x01;
        xEncoder.errBit = errBit;
        
        // startBitPos - 37: Warning (1 bit)
        uint8_t warnBit = (rawData64 >> (startBitPos - 37)) & 0x01;
        xEncoder.warnBit = warnBit;
        (void)warnBit;
        
        // startBitPos - 38 ~ startBitPos - 43: CRC (6 bits)
        uint8_t rcvCrc = (rawData64 >> (startBitPos - 43)) & 0x3F;
        xEncoder.crcRecv = rcvCrc;
        
        // 3. CRC-6 계산 및 검증
        // 검증 대상 데이터: Position(34) + Error(1) + Warning(1) = 36-bit
        uint64_t dataForCrc = (rawData64 >> (startBitPos - 37)) & 0xFFFFFFFFFULL;
        uint8_t calcCrc = Encoder_CalcCrc6(dataForCrc);
        xEncoder.crcCalc = calcCrc;
        uint8_t invertedCrc = (~calcCrc) & 0x3F;
        
        // Error 비트는 Active Low (0: 에러 발생, 1: 정상)
        if ((invertedCrc == rcvCrc) && (errBit == 1))
        {
            xEncoder.isValid = true;
            
            // 4. 소프트웨어 제로셋(오프셋) 적용
            if (xEncoder.rawPos >= xEncoder.offset)
            {
                xEncoder.position = xEncoder.rawPos - xEncoder.offset;
            }
            else
            {
                // 34-bit 롤오버 처리 (0x3FFFFFFFF + 1 = 0x400000000ULL)
                xEncoder.position = (ENC_ROLLOVER_34BIT + xEncoder.rawPos) - xEncoder.offset;
            }
            
            // 5. 기계각 스케일링 변환
            // 18-bit 싱글턴 해상도 기준 상수 (360 / 262144 = 0.001373291015625)
            xEncoder.angleDeg = (float32_t)xEncoder.position * ENC_SCALE_18BIT_DEG;
        }
        else
        {
            xEncoder.isValid = false; // CRC 오류 또는 Error 발생 시 데이터 갱신 보류 (이전 값 유지)
        }
    }
    else
    {
        xEncoder.isValid = false; // 유효한 Start 비트 및 길이 확보 실패
    }
}

/**
 * @function Encoder_CalcCrc6
 * @brief    다항식 0x43 (x^6 + x + 1)을 사용한 CRC-6 계산 수행
 * @param    data36 : 검증 대상 36비트 원시 데이터
 * @return   계산된 6비트 CRC 값
 */
static uint8_t Encoder_CalcCrc6(uint64_t data36)
{
    uint8_t crc = 0;
    int8_t i;
    // 36비트 데이터를 최상위 비트(35번 비트)부터 처리
    for (i = 35; i >= 0; i--)
    {
        uint8_t bit = (data36 >> i) & 1;
        uint8_t crc_msb = (crc >> 5) & 1;
        crc <<= 1;
        
        if (bit ^ crc_msb)
        {
            crc ^= 0x03; // x^6 위치 비트는 제거되고 하위 x^1 + 1 만 적용 (0x03)
        }
        crc &= 0x3F; // 6비트 유지
    }
    return crc;
}

/**
 * @function Encoder_SetZero
 * @brief    현재 물리 위치를 영점(Zero)으로 설정하고 FRAM에 오프셋 기록
 * @param    void
 * @return   void
 */
void Encoder_SetZero(void)
{
    uint16_t i;
    uint8_t buffer[10];
    uint16_t crc;
    
    // 안전 인터락 확인: 안전 조건이 아니면 무시
    if (MotorCtrl_IsSafeToZeroSet() == false)
    {
        return;
    }
    
    xEncoder.offset = xEncoder.rawPos;
    
    // 8바이트 버퍼에 담기 (Little Endian 방식)
    for (i = 0; i < 8; i++)
    {
        buffer[i] = (uint8_t)((xEncoder.offset >> (i * 8)) & 0xFF);
    }
    
    // CRC-16 계산
    crc = Encoder_CalcCrc16(buffer, 8);
    buffer[8] = (uint8_t)((crc >> 8) & 0xFF); // CRC MSB
    buffer[9] = (uint8_t)(crc & 0xFF);        // CRC LSB
    
    // FRAM에 10바이트 기록
    for (i = 0; i < 10; i++)
    {
        Fram_WriteByte(ENC_OFFSET_FRAM_ADDR + i, (uint16_t)buffer[i]);
    }
}

/**
 * @function Encoder_CalcCrc16
 * @brief    CRC-16-CCITT (다항식 0x1021) 계산
 * @param    data : 데이터 배열 포인터
 * @param    length : 데이터 길이
 * @return   계산된 16비트 CRC 값
 */
static uint16_t Encoder_CalcCrc16(const uint8_t* data, uint16_t length)
{
    uint16_t crc = 0xFFFF;
    uint16_t i;
    int8_t j;
    
    for (i = 0; i < length; i++)
    {
        crc ^= ((uint16_t)data[i] << 8);
        for (j = 0; j < 8; j++)
        {
            if (crc & 0x8000)
            {
                crc = (crc << 1) ^ 0x1021;
            }
            else
            {
                crc <<= 1;
            }
        }
    }
    return crc;
}
