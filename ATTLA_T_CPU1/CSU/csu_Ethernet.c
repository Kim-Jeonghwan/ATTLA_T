/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Ethernet.c
    Version          : 00.04
    Description      : 이더넷(W6100) 연동통제안 프로토콜 및 상태 머신 구현
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 23. (코딩 규칙 및 구조 불일치 사항 리팩토링 반영)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 23. - 코딩 규칙 및 구조 불일치 사항 리팩토링 반영
 * 2026. 06. 22. - 체계 연동통제안(ICD) 반영: 상태 머신 이름 동기화 및 STATE_WAIT_BOOT_ACK 롤백 로직 적용
 * 2026. 06. 17. - 하드웨어 미연결 시 sendto_W6x00 내부 무한 루프(Stuck) 방지를 위해 Ethernet_StateMachine 동작 중지(return) 예외 처리
 * 2026. 06. 17. - 하드웨어 미연결 시 무한 루프(Stuck) 방지를 위해 Ethernet_ProtocolInit 내 중복 socket() 호출 제거
 * 2026. 06. 17. - 명명 규칙 위반 리팩토링 및 헤더 인클루드 수정
 * 2026. 06. 16. - 체계 연동 통제안 상세 로직(Heartbeat 전송, 270V 구동전원 파싱, 소켓 오픈) 반영
 * 2026. 06. 16. - 초기 구현 (체크섬, 파싱, 상태머신 1~5단계, CBIT/IBIT 통합)
 */

#include "csu_Ethernet.h"

/* 글로벌 상태 구조체 인스턴스 */
stEthControl xEthCtrl;

/* 화포통제컴퓨터 고정 IP 및 포트 (추정치) */
static uint8_t fc_ip[4] = {192, 168, 200, 1};
static uint16_t fc_port = PORT_UDP_COM;
static uint8_t fc_addrlen = 4;

/*
@function   Ethernet_ProtocolInit(void)
@brief      이더넷 상태머신 초기화
*/
void Ethernet_ProtocolInit(void)
{
    // 소켓 개방은 hal_Ethernet.c의 Ethernet_Init()에서 하드웨어 정상일 때만 수행함
    // 중복 소켓 개방(socket) 호출 제거 - 하드웨어 미연결 시 무한 루프(Stuck) 방지
    
    xEthCtrl.State = STATE_BOOTING;
    xEthCtrl.LastRecvTimestamp = 0;
    xEthCtrl.TickCount100ms = 0;
    xEthCtrl.TimeoutCount = 0;
    xEthCtrl.RetryCount = 0;
    xEthCtrl.WaitAckTimer = 0;
    xEthCtrl.CbitPeriodSec = 0;
    xEthCtrl.CbitTimer100ms = 0;
    xEthCtrl.IbitInProgress = 0;
    xEthCtrl.Power270VStatus = 0;
    xEthCtrl.WaitAckCode = 0;
    xEthCtrl.TxSize = 0;
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
@function   Ethernet_CalculateChecksum(const uint8_t *pData, uint16_t length)
@brief      마지막 2바이트를 제외한 모든 바이트를 합산하여 하위 2바이트 반환
*/
uint16_t Ethernet_CalculateChecksum(const uint8_t *pData, uint16_t length)
{
    uint32_t sum = 0;
    uint16_t i;
    
    // Checksum 2바이트 필드를 제외하고 합산
    if (length <= 2) return 0;
    
    for (i = 0; i < (length - 2); i++)
    {
        sum += pData[i];
    }
    
    return (uint16_t)(sum & 0xFFFF);
}

/*
@function   Ethernet_SendAck(uint16_t codeInfo, uint16_t ackInfo, uint8_t reqAck)
@brief      ACK 전용 패킷 전송 (직렬화 적용)
*/
static void Ethernet_SendAck(uint16_t codeInfo, uint16_t ackInfo, uint8_t reqAck)
{
    uint16_t offset = 0;
    
    // 1. 헤더 직렬화 (12 Bytes)
    Serialize_Uint32(xEthCtrl.LastRecvTimestamp, xEthCtrl.TxBuffer, &offset); // Timestamp
    Serialize_Uint8(ETH_MY_ID, xEthCtrl.TxBuffer, &offset);                   // Source_ID
    Serialize_Uint8(ETH_FC_ID, xEthCtrl.TxBuffer, &offset);                   // Dest_ID
    Serialize_Uint8(ETH_CODE_ACK, xEthCtrl.TxBuffer, &offset);                // Code
    Serialize_Uint8(reqAck, xEthCtrl.TxBuffer, &offset);                      // Request_ACK
    Serialize_Uint8(0, xEthCtrl.TxBuffer, &offset);                           // Priority
    Serialize_Uint8(1, xEthCtrl.TxBuffer, &offset);                           // Send_Count
    Serialize_Uint16(4, xEthCtrl.TxBuffer, &offset);                          // Data_Length (ACK는 4바이트 페이로드)
    
    // 2. 페이로드 직렬화 (4 Bytes)
    Serialize_Uint16(codeInfo, xEthCtrl.TxBuffer, &offset);
    Serialize_Uint16(ackInfo, xEthCtrl.TxBuffer, &offset);
    
    // 3. 체크섬 계산 및 직렬화 (2 Bytes)
    uint16_t totalLen = offset + 2;
    uint16_t checksum = Ethernet_CalculateChecksum(xEthCtrl.TxBuffer, totalLen);
    Serialize_Uint16(checksum, xEthCtrl.TxBuffer, &offset);
    
    // 4. 전송
    xEthCtrl.TxSize = totalLen;
    sendto_W6x00(SOCK_UDP_COM, xEthCtrl.TxBuffer, xEthCtrl.TxSize, fc_ip, fc_port, fc_addrlen);
}

/*
@function   Ethernet_SendMessage(uint8_t code, uint8_t reqAck, uint8_t *pData, uint16_t dataLen)
@brief      일반 페이로드 패킷 조립 및 전송 (ACK 재전송 보관, 직렬화 적용)
*/
void Ethernet_SendMessage(uint8_t code, uint8_t reqAck, uint8_t *pData, uint16_t dataLen)
{
    uint16_t offset = 0;
    uint16_t totalLen = 12 + dataLen + 2; // Header(12) + Data + Checksum(2)
    uint16_t checksum;
    
    if (totalLen > sizeof(xEthCtrl.TxBuffer)) return; // 버퍼 초과 방지
    
    // 1. 헤더 직렬화
    Serialize_Uint32(0x00000000, xEthCtrl.TxBuffer, &offset); // 자발적 발행 메시지는 0x00
    Serialize_Uint8(ETH_MY_ID, xEthCtrl.TxBuffer, &offset);
    Serialize_Uint8(ETH_FC_ID, xEthCtrl.TxBuffer, &offset);
    Serialize_Uint8(code, xEthCtrl.TxBuffer, &offset);
    Serialize_Uint8(reqAck, xEthCtrl.TxBuffer, &offset);
    Serialize_Uint8(0, xEthCtrl.TxBuffer, &offset);           // Priority
    
    // Send_Count 결정
    uint8_t sendCount = 1;
    if (xEthCtrl.WaitAckCode == code && xEthCtrl.RetryCount > 0) {
        sendCount = (uint8_t)(xEthCtrl.RetryCount);
    }
    Serialize_Uint8(sendCount, xEthCtrl.TxBuffer, &offset);
    
    Serialize_Uint16(dataLen, xEthCtrl.TxBuffer, &offset);
    
    // 2. 페이로드 복사 (pData 배열은 이미 1바이트씩 들어있다고 가정)
    if (dataLen > 0 && pData != NULL) {
        uint16_t i;
        for (i = 0; i < dataLen; i++) {
            Serialize_Uint8(pData[i], xEthCtrl.TxBuffer, &offset);
        }
    }
    
    // 3. 체크섬 연산 및 직렬화
    checksum = Ethernet_CalculateChecksum(xEthCtrl.TxBuffer, totalLen);
    Serialize_Uint16(checksum, xEthCtrl.TxBuffer, &offset);
    
    xEthCtrl.TxSize = totalLen;
    
    // 4. 전송
    sendto_W6x00(SOCK_UDP_COM, xEthCtrl.TxBuffer, xEthCtrl.TxSize, fc_ip, fc_port, fc_addrlen);
    
    // 5. ACK 대기 타이머 세팅
    if (reqAck == ETH_ACK_REQ) {
        xEthCtrl.WaitAckCode = code;
        xEthCtrl.WaitAckTimer = 0;
        if (xEthCtrl.RetryCount == 0) {
            xEthCtrl.RetryCount = 1; // 최초 전송 카운트
        }
    }
}

/*
@function   Ethernet_StateMachine(void)
@brief      메인 제어 루프 등에서 100ms 주기로 호출되는 망 가입 상태 머신
*/
void Ethernet_StateMachine(void)
{
    // 하드웨어(W6100)가 연결되지 않은 경우 상태 머신을 동작시키지 않아 무한 루프(Stuck) 방지
    if (isW6100Connected == 0) {
        return;
    }

    xEthCtrl.TickCount100ms++;
    
    // [1] 재전송 및 타임아웃 감시 로직 (ACK 대기 중일 때)
    if (xEthCtrl.WaitAckCode != 0)
    {
        xEthCtrl.WaitAckTimer++;
        if (xEthCtrl.WaitAckTimer >= ETH_ACK_TIMEOUT)
        {
            xEthCtrl.RetryCount++;
            
            if (xEthCtrl.RetryCount > ETH_MAX_RETRY_COUNT)
            {
                // 4회 실패 (최초 1회 + 재전송 3회) -> 통신 두절 예외 처리
                xEthCtrl.WaitAckCode = 0;
                xEthCtrl.RetryCount = 0;
                
                // Step 5: 통신 두절 발생 시 소켓 리셋 후 망 가입 롤백
                // (socket 닫고 새로 열기)
                socket(SOCK_UDP_COM, Sn_MR_UDP, PORT_UDP_COM, 0x00);
                xEthCtrl.State = STATE_WAIT_BOOT_ACK;
                xEthCtrl.TimeoutCount = 0;
            }
            else
            {
                // 기존 패킷의 Send_Count만 업데이트 후 재전송
                if (xEthCtrl.TxSize >= sizeof(stEthHeader)) {
                    stEthHeader *pHeader = (stEthHeader*)xEthCtrl.TxBuffer;
                    pHeader->Send_Count = (uint8_t)xEthCtrl.RetryCount;
                    
                    // 체크섬 재계산
                    uint16_t cs = Ethernet_CalculateChecksum(xEthCtrl.TxBuffer, xEthCtrl.TxSize);
                    memcpy(&xEthCtrl.TxBuffer[xEthCtrl.TxSize - 2], &cs, 2);
                    
                    sendto_W6x00(SOCK_UDP_COM, xEthCtrl.TxBuffer, xEthCtrl.TxSize, fc_ip, fc_port, fc_addrlen);
                }
                xEthCtrl.WaitAckTimer = 0; // 타이머 리셋
            }
        }
    }
    
    // [2] 상태에 따른 동작 수행
    switch (xEthCtrl.State)
    {
        case STATE_BOOTING:
            // 28V 제어전원 인가 및 UDP Bind 완료 직후 상태
            // 망 가입 절차로 즉시 천이
            xEthCtrl.State = STATE_WAIT_BOOT_ACK;
            break;
            
        case STATE_WAIT_BOOT_ACK:
            // 망 가입을 위해 500ms(5 Ticks) 마다 Boot Done 전송 시도
            if ((xEthCtrl.TickCount100ms % ETH_BOOTDONE_PERIOD) == 0 && xEthCtrl.WaitAckCode == 0)
            {
                // Boot Done은 ACK를 요청함
                Ethernet_SendMessage(ETH_CODE_BOOT_DONE, ETH_ACK_REQ, NULL, 0);
            }
            break;
            
        case STATE_JOINED:
            // 망 가입 완료 (Step 4: Heartbeat)
            // 100ms 주기 상태정보 통신
            if ((xEthCtrl.TickCount100ms % ETH_HEARTBEAT_PERIOD) == 0 && xEthCtrl.WaitAckCode == 0)
            {
                // Heartbeat 페이로드의 첫 번째 바이트에 270V 전원 상태를 싣는다고 가정
                uint8_t statusPayload[1];
                statusPayload[0] = xEthCtrl.Power270VStatus;
                
                Ethernet_SendMessage(ETH_CODE_HEARTBEAT, ETH_ACK_NOT_REQ, statusPayload, 1);
            }
            
            // 통신 두절 감시 (100ms * 50회 = 5초간 수신 없으면 리셋)
            xEthCtrl.TimeoutCount++;
            if (xEthCtrl.TimeoutCount >= ETH_DISCONNECT_LIMIT)
            {
                // 소켓 리셋 및 망 가입 초기화
                socket(SOCK_UDP_COM, Sn_MR_UDP, PORT_UDP_COM, 0x00);
                xEthCtrl.State = STATE_WAIT_BOOT_ACK;
                xEthCtrl.TimeoutCount = 0;
                xEthCtrl.WaitAckCode = 0;
            }
            
            // CBIT 동작 로직 (Step 5)
            if (xEthCtrl.CbitPeriodSec > 0 && xEthCtrl.IbitInProgress == 0)
            {
                xEthCtrl.CbitTimer100ms++;
                if (xEthCtrl.CbitTimer100ms >= (xEthCtrl.CbitPeriodSec * 10))
                {
                    xEthCtrl.CbitTimer100ms = 0;
                    // CBIT 결과 전송 (ACK 미요청)
                    Ethernet_SendMessage(ETH_CODE_CBIT_REP, ETH_ACK_NOT_REQ, NULL, 0);
                }
            }
            break;
    }
}

/*
@function   Ethernet_ParsePacket(uint8_t *pRxBuf, uint16_t length)
@brief      수신 패킷 역직렬화 파싱 및 명령 수행
*/
void Ethernet_ParsePacket(uint8_t *pRxBuf, uint16_t length)
{
    // 최소 길이 (Header 12 + Checksum 2) 검증
    if (length < 14) return;
    
    uint16_t offset = 0;
    
    // 1. 역직렬화 수행
    stEthHeader header;
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
    uint16_t calcChecksum = Ethernet_CalculateChecksum(pRxBuf, length);
    
    // 에러 발생 시 처리 (체크섬 오류 시 NACK)
    if (recvChecksum != calcChecksum)
    {
        if (header.Request_ACK == ETH_ACK_REQ && header.Code != ETH_CODE_ACK)
        {
            // 수신 패킷의 Timestamp는 일단 적용 후 NACK 전송
            xEthCtrl.LastRecvTimestamp = header.Timestamp;
            Ethernet_SendAck(header.Code, ETH_ACK_INFO_CS_ERR, ETH_ACK_NACK);
        }
        return; 
    }
    
    // 패킷 정상 수신 -> 타임아웃 타이머 초기화 (통신 두절 방어)
    xEthCtrl.TimeoutCount = 0;
    xEthCtrl.LastRecvTimestamp = header.Timestamp;
    
    // 정상 ACK 응답 처리
    if (header.Request_ACK == ETH_ACK_REQ && header.Code != ETH_CODE_ACK)
    {
        Ethernet_SendAck(header.Code, ETH_ACK_INFO_OK, ETH_ACK_NORMAL);
    }
    
    // 메시지 Code 분석 및 동작
    switch (header.Code)
    {
        case ETH_CODE_ACK:
            // 내가 보낸 메시지에 대한 응답이 온 경우
            if (length >= 18 && header.Data_Length == 4) // 고정 18바이트 (Header 12 + Data 4 + CS 2)
            {
                uint16_t codeInfo = Deserialize_Uint16(pRxBuf, &offset);
                uint16_t ackInfo = Deserialize_Uint16(pRxBuf, &offset);
                
                // 현재 대기 중인 ACK 코드와 일치하는가?
                if (xEthCtrl.WaitAckCode == codeInfo && ackInfo == ETH_ACK_INFO_OK)
                {
                    // 재전송 중지 및 대기 해제
                    xEthCtrl.WaitAckCode = 0;
                    xEthCtrl.RetryCount = 0;
                    
                    // 만약 Boot Done 전송 중이었는데 ACK가 왔다면 망 가입 완료
                    if (xEthCtrl.State == STATE_WAIT_BOOT_ACK && codeInfo == ETH_CODE_BOOT_DONE)
                    {
                        xEthCtrl.State = STATE_JOINED;
                    }
                }
            }
            break;
            
        case ETH_CODE_PBIT_REQ:
            // PBIT 결과 전송
            Ethernet_SendMessage(ETH_CODE_PBIT_REP, ETH_ACK_NOT_REQ, NULL, 0);
            break;
            
        case ETH_CODE_IBIT_REQ:
            xEthCtrl.IbitInProgress = 1;
            // TODO: 실제 IBIT 검증 함수 연동 필요. (수행 완료 후 결과 전송)
            // 임시로 바로 완료되었다고 가정하고 IBIT_REP 전송
            Ethernet_SendMessage(ETH_CODE_IBIT_REP, ETH_ACK_NOT_REQ, NULL, 0);
            xEthCtrl.IbitInProgress = 0;
            break;
            
        case ETH_CODE_POWER_270V:
            // 270VDC 구동 전원 인가 메시지 수신 (상태정보에 반영)
            xEthCtrl.Power270VStatus = 1;
            break;
            
        case ETH_CODE_CBIT_SET:
            if (header.Data_Length >= 2)
            {
                // 주기 설정값 파싱 (예: 페이로드 첫 2바이트가 N초)
                uint16_t periodSec = Deserialize_Uint16(pRxBuf, &offset);
                xEthCtrl.CbitPeriodSec = periodSec;
                xEthCtrl.CbitTimer100ms = 0;
            }
            break;
            
        case ETH_CODE_HEARTBEAT:
            // 체계에서 Heartbeat 요청이 오는 경우 응답 (현재 상태 전송)
            {
                uint8_t statusPayload[1];
                statusPayload[0] = xEthCtrl.Power270VStatus;
                Ethernet_SendMessage(ETH_CODE_HEARTBEAT, ETH_ACK_NOT_REQ, statusPayload, 1);
            }
            break;
            
        default:
            break;
    }
}
