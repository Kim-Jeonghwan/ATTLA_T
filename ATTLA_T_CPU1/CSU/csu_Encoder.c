/**********************************************************************
 Nexcom Co., Ltd.
 Filename         : csu_Encoder.c
 Version          : 00.01
 Description      : AksIM-2 엔코더 어플리케이션 기능 처리 모듈
 Programmer       : Kim Jeonghwan
 Last Updated     : 2026. 06. 09. (신규 생성)
**********************************************************************/

#include "csu_Encoder.h"

//---------------------------------------------------------------------------
// 전역 변수
//---------------------------------------------------------------------------
PM_bissc_registerStruct encRegAccess;

typedef enum {
    ENC_SEQ_IDLE = 0,
    ENC_SEQ_WRITE_KEY,
    ENC_SEQ_WAIT_KEY,
    ENC_SEQ_WRITE_CMD,
    ENC_SEQ_WAIT_CMD
} EncoderSeqState_e;

static EncoderSeqState_e seqState = ENC_SEQ_IDLE;
static uint16_t pendingCmd = 0;

//---------------------------------------------------------------------------
// Encoder_Init
//---------------------------------------------------------------------------
void Encoder_Init(void)
{
    // HAL 초기화 호출
    Encoder_Init_Hardware();
    
    seqState = ENC_SEQ_IDLE;
}

//---------------------------------------------------------------------------
// Encoder_UpdatePosition (1ms 또는 EPWM 인터럽트에서 주기적 호출)
//---------------------------------------------------------------------------
void Encoder_UpdatePosition(void)
{
    // 1. 단일 사이클 트랜잭션 준비
    PM_bissc_setupSCDTransaction(&encoderScdParams);
    
    // 2. 통신 개시 (클럭 발생 시작)
    // PM_bissc_setFreq 에서 설정한 freqDiv를 그대로 사용 (예: 20)
    PM_bissc_startOperation(&encoderScdParams, 20);
    
    // 3. 통신이 완료될 때까지 잠시 대기 (SPI FIFO를 통해 수신)
    // 주의: 통신 주파수가 빠르므로 몇 us 이내에 완료됩니다.
    // (실제 적용 시에는 블로킹 없이 타이머로 체크하거나 인터럽트를 활용할 수도 있습니다)
    while(encoderScdParams.dataReady == false)
    {
        // SPI RX 인터럽트 등에서 dataReady를 true로 바꿔주거나, 
        // 여기서 폴링 방식으로 FIFO 상태를 체크하여 읽어와야 합니다.
        // *본 라이브러리 구조상 `pm_bissc_source.c`의 원본 예제에서는 SPI RX 인터럽트(INT_SPIB_RX)
        // 안에서 scdRxData 배열에 데이터를 넣고 dataReady=true 처리합니다.
        
        // 일단 무한루프 방지를 위해 브레이크 (실제로는 인터럽트와 연동 필요)
        break; 
    }
    
    // 4. 수신된 데이터 파싱 및 에러/위치값 갱신
    if (encoderScdParams.dataReady == true)
    {
        PM_bissc_receivePosition(&encoderScdParams, &encoderCdParams, &encoderData);
        encoderScdParams.dataReady = false;
    }
}

//---------------------------------------------------------------------------
// Encoder_ProcessCDTasks (백그라운드 루프에서 주기적 호출)
//---------------------------------------------------------------------------
void Encoder_ProcessCDTasks(void)
{
    // 라이브러리의 상태 머신 구동 (매 주기 1비트씩 통신)
    PM_bissc_doCDTasks(&encoderCdParams, &encRegAccess);
    
    // 내부 시퀀스 제어 (Key -> Cmd 연속 전송 로직)
    switch(seqState)
    {
        case ENC_SEQ_IDLE:
            break;
            
        case ENC_SEQ_WRITE_KEY:
            if (encoderCdParams.cdState == PM_BISSC_CD_NONE)
            {
                encRegAccess.address = ENC_REG_KEY;
                encRegAccess.txData = ENC_KEY_UNLOCK;
                encRegAccess.accessType = PM_BISSC_REGISTER_WRITE;
                encoderCdParams.cdState = PM_BISSC_CD_NEW_REQUEST;
                seqState = ENC_SEQ_WAIT_KEY;
            }
            break;
            
        case ENC_SEQ_WAIT_KEY:
            if (encoderCdParams.cdState == PM_BISSC_CD_NONE) // 쓰기 완료
            {
                if (encoderCdParams.cdStatus == PM_BISSC_CD_WRITE_PASS)
                {
                    seqState = ENC_SEQ_WRITE_CMD;
                }
                else
                {
                    seqState = ENC_SEQ_IDLE; // 실패 시 초기화
                }
            }
            break;
            
        case ENC_SEQ_WRITE_CMD:
            encRegAccess.address = ENC_REG_CMD;
            encRegAccess.txData = pendingCmd;
            encRegAccess.accessType = PM_BISSC_REGISTER_WRITE;
            encoderCdParams.cdState = PM_BISSC_CD_NEW_REQUEST;
            seqState = ENC_SEQ_WAIT_CMD;
            break;
            
        case ENC_SEQ_WAIT_CMD:
            if (encoderCdParams.cdState == PM_BISSC_CD_NONE) // 완료
            {
                seqState = ENC_SEQ_IDLE;
            }
            break;
    }
}

//---------------------------------------------------------------------------
// Encoder_SaveParameters (비휘발성 메모리 저장)
//---------------------------------------------------------------------------
void Encoder_SaveParameters(void)
{
    if (seqState == ENC_SEQ_IDLE)
    {
        pendingCmd = ENC_CMD_SAVE_PARAMS;
        seqState = ENC_SEQ_WRITE_KEY;
    }
}

//---------------------------------------------------------------------------
// Encoder_StartCalibration (셀프 캘리브레이션 시작)
//---------------------------------------------------------------------------
void Encoder_StartCalibration(void)
{
    if (seqState == ENC_SEQ_IDLE)
    {
        pendingCmd = ENC_CMD_START_CALIB;
        seqState = ENC_SEQ_WRITE_KEY;
    }
}

//---------------------------------------------------------------------------
// Encoder_RequestTemperature (온도 읽기 요청)
//---------------------------------------------------------------------------
void Encoder_RequestTemperature(void)
{
    // Big Endian 16bit 이므로 0x4C (High), 0x4D (Low) 순차적으로 읽어야 함
    // 우선 High 바이트 요청 예시
    if (encoderCdParams.cdState == PM_BISSC_CD_NONE && seqState == ENC_SEQ_IDLE)
    {
        encRegAccess.address = ENC_REG_TEMP_H;
        encRegAccess.accessType = PM_BISSC_REGISTER_READ;
        encoderCdParams.cdState = PM_BISSC_CD_NEW_REQUEST;
    }
}
