/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Debug_cpu1.c
    Version          : 00.06
    Description      : 노트북 디버깅망 동적 IP 라우팅 프로토콜 및 텔레메트리 구현
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 26. (모듈명 변경 및 디버깅 전용망 분리)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 26. - csu_Ethernet_cpu1 에서 csu_Debug_cpu1 으로 모듈명 변경
 * 2026. 06. 26. - 체계 연동(FC) 통제안(Boot Ack, CBIT, IBIT 등) 로직 전면 삭제
 * 2026. 06. 26. - 노트북 통신을 위한 동적 IP 타겟팅 및 100ms 텔레메트리(상태 전송) 로직 구현
 */

#include "csu_Debug_cpu1.h"

/* 글로벌 상태 구조체 인스턴스 */
stDbgControl xDbgCtrl;

/*
@function   Debug_ProtocolInit(void)
@brief      디버그 통신 상태머신 초기화
*/
void Debug_ProtocolInit(void)
{
    xDbgCtrl.isActive = 0U;
    
    // 기본 타겟을 임의의 노트북 IP로 설정 (첫 패킷 수신 전까지는 송신하지 않음)
    xDbgCtrl.targetIp[0] = 192;
    xDbgCtrl.targetIp[1] = 168;
    xDbgCtrl.targetIp[2] = 200;
    xDbgCtrl.targetIp[3] = 100;
    xDbgCtrl.targetPort = PORT_UDP_DBG;
    
    xDbgCtrl.TimeoutCount = 0U;
    xDbgCtrl.TickCount100ms = 0U;
}

/* ========================================================================== */
/* [직렬화 / 역직렬화 헬퍼 함수]                                              */
/* ========================================================================== */

static void Serialize_Uint32(uint32_t val, uint8_t* buf, uint16_t* offset) {
    buf[(*offset)++] = (uint8_t)(val & 0xFF);
    buf[(*offset)++] = (uint8_t)((val >> 8) & 0xFF);
    buf[(*offset)++] = (uint8_t)((val >> 16) & 0xFF);
    buf[(*offset)++] = (uint8_t)((val >> 24) & 0xFF);
}

static void Serialize_Uint16(uint16_t val, uint8_t* buf, uint16_t* offset) {
    buf[(*offset)++] = (uint8_t)(val & 0xFF);
    buf[(*offset)++] = (uint8_t)((val >> 8) & 0xFF);
}

static void Serialize_Uint8(uint8_t val, uint8_t* buf, uint16_t* offset) {
    buf[(*offset)++] = (uint8_t)(val & 0xFF);
}

static uint32_t Deserialize_Uint32(uint8_t* buf, uint16_t* offset) {
    uint32_t val = (uint32_t)(buf[*offset] & 0xFF) | 
                   ((uint32_t)(buf[*offset + 1] & 0xFF) << 8) | 
                   ((uint32_t)(buf[*offset + 2] & 0xFF) << 16) | 
                   ((uint32_t)(buf[*offset + 3] & 0xFF) << 24);
    *offset += 4;
    return val;
}

static uint16_t Deserialize_Uint16(uint8_t* buf, uint16_t* offset) {
    uint16_t val = (uint16_t)(buf[*offset] & 0xFF) | 
                   ((uint16_t)(buf[*offset + 1] & 0xFF) << 8);
    *offset += 2;
    return val;
}

static uint8_t Deserialize_Uint8(uint8_t* buf, uint16_t* offset) {
    uint8_t val = (uint8_t)(buf[*offset] & 0xFF);
    *offset += 1;
    return val;
}

/*
@function   Debug_CalculateChecksum(const uint8_t *pData, uint16_t length)
@brief      마지막 2바이트를 제외한 모든 바이트를 합산하여 하위 2바이트 반환
*/
uint16_t Debug_CalculateChecksum(const uint8_t *pData, uint16_t length)
{
    uint32_t sum = 0;
    uint16_t i;
    
    if (length <= 2) return 0;
    
    for (i = 0; i < (length - 2); i++)
    {
        sum += pData[i];
    }
    
    return (uint16_t)(sum & 0xFFFF);
}

/*
@function   Debug_SendMessage(uint8_t code, uint8_t *pData, uint16_t dataLen)
@brief      페이로드 패킷 조립 및 전송 (캡처된 동적 IP 타겟으로 송신)
*/
void Debug_SendMessage(uint8_t code, uint8_t *pData, uint16_t dataLen)
{
    uint16_t offset = 0;
    uint16_t totalLen = 12 + dataLen + 2; // Header(12) + Data + Checksum(2)
    uint16_t checksum;
    
    if (totalLen > sizeof(xDbgCtrl.TxBuffer)) return; // 버퍼 초과 방지
    if (xDbgCtrl.isActive == 0U) return; // 활성화되지 않았으면 송신 불가
    
    // 1. 헤더 직렬화
    Serialize_Uint32(0x00000000, xDbgCtrl.TxBuffer, &offset); // Timestamp
    Serialize_Uint8(DBG_MY_ID, xDbgCtrl.TxBuffer, &offset);
    Serialize_Uint8(DBG_PC_ID, xDbgCtrl.TxBuffer, &offset);
    Serialize_Uint8(code, xDbgCtrl.TxBuffer, &offset);
    Serialize_Uint8(0xFF, xDbgCtrl.TxBuffer, &offset);        // Request_ACK (미사용)
    Serialize_Uint8(0, xDbgCtrl.TxBuffer, &offset);           // Priority
    Serialize_Uint8(1, xDbgCtrl.TxBuffer, &offset);           // Send_Count
    Serialize_Uint16(dataLen, xDbgCtrl.TxBuffer, &offset);
    
    // 2. 페이로드 복사
    if (dataLen > 0 && pData != NULL) {
        uint16_t i;
        for (i = 0; i < dataLen; i++) {
            Serialize_Uint8(pData[i], xDbgCtrl.TxBuffer, &offset);
        }
    }
    
    // 3. 체크섬 연산 및 직렬화
    checksum = Debug_CalculateChecksum(xDbgCtrl.TxBuffer, totalLen);
    Serialize_Uint16(checksum, xDbgCtrl.TxBuffer, &offset);
    
    // 4. 전송 (동적 캡처된 PC IP, Port 사용)
    sendto_W6x00(SOCK_UDP_DBG, xDbgCtrl.TxBuffer, totalLen, xDbgCtrl.targetIp, xDbgCtrl.targetPort, 4);
}

/*
@function   Debug_StateMachine(void)
@brief      메인 제어 루프 등에서 100ms 주기로 호출되는 텔레메트리 자동 송신
*/
void Debug_StateMachine(void)
{
    // 하드웨어(W6100) 미연결 시 동작 중지
    if (isW6100Connected == 0) {
        return;
    }

    xDbgCtrl.TickCount100ms++;
    
    if (xDbgCtrl.isActive == 1U)
    {
        // 1. 통신 두절 감시 (5초 동안 요청 패킷이 안오면 연결 해제)
        xDbgCtrl.TimeoutCount++;
        if (xDbgCtrl.TimeoutCount >= DBG_DISCONNECT_LIMIT)
        {
            xDbgCtrl.isActive = 0U;
            xDbgCtrl.TimeoutCount = 0U;
            return;
        }
        
        // 2. 주기적 텔레메트리 송신
        stDbgTelemetry tData;
        tData.systemTick = xDbgCtrl.TickCount100ms;
        
        // 임시 모터 데이터 매핑 (구체적인 구조는 향후 확정)
        tData.currentA = 0.0f; // 실제 측정 변수로 대체 요망
        tData.currentB = 0.0f;
        tData.currentC = 0.0f;
        tData.faultStatus = xBit.informAll; // 현재 BIT 진단 결과 (xBit.informAll 등)

        uint8_t payload[20];
        uint16_t offset = 0;
        Serialize_Uint32(tData.systemTick, payload, &offset);
        
        // C28x DSP에서 float는 32비트. 임시로 uint32_t 캐스팅 직렬화 적용
        uint32_t ca = *((uint32_t*)&tData.currentA);
        uint32_t cb = *((uint32_t*)&tData.currentB);
        uint32_t cc = *((uint32_t*)&tData.currentC);
        
        Serialize_Uint32(ca, payload, &offset);
        Serialize_Uint32(cb, payload, &offset);
        Serialize_Uint32(cc, payload, &offset);
        Serialize_Uint32(tData.faultStatus, payload, &offset);

        Debug_SendMessage(DBG_CODE_TELEMETRY, payload, offset);
    }
}

/*
@function   Debug_ParsePacket(uint8_t *pRxBuf, uint16_t length, uint8_t *pSenderIp, uint16_t senderPort)
@brief      수신 패킷 역직렬화 파싱 및 PC 동적 IP 캡처
*/
void Debug_ParsePacket(uint8_t *pRxBuf, uint16_t length, uint8_t *pSenderIp, uint16_t senderPort)
{
    // 최소 길이 (Header 12 + Checksum 2) 검증
    if (length < 14) return;
    
    // [동적 IP 라우팅] 송신자(PC) IP와 Port를 백업하여 응답용 타겟으로 설정
    xDbgCtrl.isActive = 1U;
    xDbgCtrl.TimeoutCount = 0U;
    
    xDbgCtrl.targetIp[0] = pSenderIp[0];
    xDbgCtrl.targetIp[1] = pSenderIp[1];
    xDbgCtrl.targetIp[2] = pSenderIp[2];
    xDbgCtrl.targetIp[3] = pSenderIp[3];
    xDbgCtrl.targetPort = senderPort;
    
    uint16_t offset = 0;
    
    // 1. 역직렬화 (헤더 파싱)
    stDbgHeader header;
    header.Timestamp   = Deserialize_Uint32(pRxBuf, &offset);
    header.Source_ID   = Deserialize_Uint8(pRxBuf, &offset);
    header.Dest_ID     = Deserialize_Uint8(pRxBuf, &offset);
    header.Code        = Deserialize_Uint8(pRxBuf, &offset);
    header.Request_ACK = Deserialize_Uint8(pRxBuf, &offset);
    header.Priority    = Deserialize_Uint8(pRxBuf, &offset);
    header.Send_Count  = Deserialize_Uint8(pRxBuf, &offset);
    header.Data_Length = Deserialize_Uint16(pRxBuf, &offset);
    
    // 2. 체크섬 검증
    uint16_t csOffset = length - 2;
    uint16_t recvChecksum = Deserialize_Uint16(pRxBuf, &csOffset);
    uint16_t calcChecksum = Debug_CalculateChecksum(pRxBuf, length);
    
    if (recvChecksum != calcChecksum)
    {
        return; // 오류 시 무시
    }
    
    // 3. 명령어(Code) 분기 처리
    switch (header.Code)
    {
        case DBG_CODE_REQ_STATE:
            // 별도의 1회성 상태 요청인 경우 텔레메트리 수동 송신 처리 (필요 시 구현)
            break;
            
        case DBG_CODE_CLR_FAULT:
            // PC로부터의 폴트 리셋(Clear Fault) 명령 처리
            Bit_FaultReset(1U);
            break;
            
        default:
            break;
    }
}
