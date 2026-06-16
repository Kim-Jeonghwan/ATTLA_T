/**********************************************************************
 Nexcom Co., Ltd.
 Filename         : csu_Ethernet.c
 Version          : 00.00
 Description      : 이더넷(W6100) 연동통제안 프로토콜 및 상태 머신 구현
 Programmer       : Kim Jeonghwan
 Last Updated     : 2026. 06. 16. (이더넷 통신 상태 머신 및 파싱 로직 구현)
**********************************************************************/

/*
 * Modification History
 * 2026. 06. 16. - 체계 연동 통제안 상세 로직(Heartbeat 전송, 270V 구동전원 파싱, 소켓 오픈) 반영
 * 2026. 06. 16. - 초기 구현 (체크섬, 파싱, 상태머신 1~5단계, CBIT/IBIT 통합)
 */

#include "csu_Ethernet.h"
#include "hal_Ethernet.h" // 소켓 제어 함수 및 매크로 사용

/* 글로벌 상태 구조체 인스턴스 */
stEthControl xEthCtrl;

/* 화포통제컴퓨터 고정 IP 및 포트 (추정치) */
static uint8_t fc_ip[4] = {192, 168, 200, 1};
static uint16_t fc_port = PORT_UDP_COM;
static uint8_t fc_addrlen = 4;

/*
@function   csu_Ethernet_Init(void)
@brief      이더넷 상태머신 초기화
*/
void csu_Ethernet_Init(void)
{
    // 초기 소켓 개방 (UDP 모드, 포트 5001)
    socket(SOCK_UDP_COM, Sn_MR_UDP, PORT_UDP_COM, 0x00);
    
    xEthCtrl.State = ETH_STATE_INIT;
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

/*
@function   csu_Ethernet_CalculateChecksum(const uint8_t *pData, uint16_t length)
@brief      마지막 2바이트를 제외한 모든 바이트를 합산하여 하위 2바이트 반환
*/
uint16_t csu_Ethernet_CalculateChecksum(const uint8_t *pData, uint16_t length)
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
@function   csu_Ethernet_SendAck(uint16_t codeInfo, uint16_t ackInfo, uint8_t reqAck)
@brief      ACK 전용 패킷 전송
*/
static void csu_Ethernet_SendAck(uint16_t codeInfo, uint16_t ackInfo, uint8_t reqAck)
{
    stEthAckMessage ackMsg;
    
    ackMsg.Header.Timestamp = xEthCtrl.LastRecvTimestamp; // 수신된 Timestamp 그대로 리턴
    ackMsg.Header.Source_ID = ETH_MY_ID;
    ackMsg.Header.Dest_ID = ETH_FC_ID;
    ackMsg.Header.Code = ETH_CODE_ACK;
    ackMsg.Header.Request_ACK = reqAck;
    ackMsg.Header.Priority = 0;
    ackMsg.Header.Send_Count = 1;
    ackMsg.Header.Data_Length = 4;
    
    ackMsg.Code_Info = codeInfo;
    ackMsg.Ack_Info = ackInfo;
    
    ackMsg.Checksum = csu_Ethernet_CalculateChecksum((uint8_t*)&ackMsg, sizeof(stEthAckMessage));
    
    sendto_W6x00(SOCK_UDP_COM, (uint8_t*)&ackMsg, sizeof(stEthAckMessage), fc_ip, fc_port, fc_addrlen);
}

/*
@function   csu_Ethernet_SendMessage(uint8_t code, uint8_t reqAck, uint8_t *pData, uint16_t dataLen)
@brief      일반 페이로드 패킷 조립 및 전송 (ACK 재전송 보관)
*/
void csu_Ethernet_SendMessage(uint8_t code, uint8_t reqAck, uint8_t *pData, uint16_t dataLen)
{
    stEthHeader header;
    uint16_t totalLen = sizeof(stEthHeader) + dataLen + 2; // Header + Data + Checksum
    uint16_t checksum;
    
    if (totalLen > sizeof(xEthCtrl.TxBuffer)) return; // 버퍼 초과 방지
    
    header.Timestamp = 0x00000000; // 자발적 발행 메시지는 0x00
    header.Source_ID = ETH_MY_ID;
    header.Dest_ID = ETH_FC_ID;
    header.Code = code;
    header.Request_ACK = reqAck;
    header.Priority = 0;
    
    // Send_Count 결정 (신규 송신이면 1, 재전송이면 RetryCount 반영)
    if (xEthCtrl.WaitAckCode == code && xEthCtrl.RetryCount > 0) {
        header.Send_Count = (uint8_t)(xEthCtrl.RetryCount);
    } else {
        header.Send_Count = 1;
    }
    
    header.Data_Length = dataLen;
    
    // Tx 버퍼에 직렬화
    uint16_t offset = 0;
    memcpy(&xEthCtrl.TxBuffer[offset], &header, sizeof(stEthHeader));
    offset += sizeof(stEthHeader);
    
    if (dataLen > 0 && pData != NULL) {
        memcpy(&xEthCtrl.TxBuffer[offset], pData, dataLen);
        offset += dataLen;
    }
    
    // 체크섬 계산 및 추가
    checksum = csu_Ethernet_CalculateChecksum(xEthCtrl.TxBuffer, totalLen);
    memcpy(&xEthCtrl.TxBuffer[offset], &checksum, 2);
    offset += 2;
    
    xEthCtrl.TxSize = totalLen;
    
    // 물리적 전송
    sendto_W6x00(SOCK_UDP_COM, xEthCtrl.TxBuffer, xEthCtrl.TxSize, fc_ip, fc_port, fc_addrlen);
    
    // ACK 요청 메시지인 경우 재전송 타이머 세팅
    if (reqAck == ETH_ACK_REQ) {
        xEthCtrl.WaitAckCode = code;
        xEthCtrl.WaitAckTimer = 0;
        if (xEthCtrl.RetryCount == 0) {
            xEthCtrl.RetryCount = 1; // 최초 전송 카운트
        }
    }
}

/*
@function   csu_Ethernet_StateMachine(void)
@brief      메인 제어 루프 등에서 100ms 주기로 호출되는 망 가입 상태 머신
*/
void csu_Ethernet_StateMachine(void)
{
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
                xEthCtrl.State = ETH_STATE_INIT;
                xEthCtrl.TimeoutCount = 0;
            }
            else
            {
                // 기존 패킷의 Send_Count만 업데이트 후 재전송
                if (xEthCtrl.TxSize >= sizeof(stEthHeader)) {
                    stEthHeader *pHeader = (stEthHeader*)xEthCtrl.TxBuffer;
                    pHeader->Send_Count = (uint8_t)xEthCtrl.RetryCount;
                    
                    // 체크섬 재계산
                    uint16_t cs = csu_Ethernet_CalculateChecksum(xEthCtrl.TxBuffer, xEthCtrl.TxSize);
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
        case ETH_STATE_INIT:
            // 소켓 개방 후 500ms(5 Ticks) 마다 Boot Done 전송 시도
            if ((xEthCtrl.TickCount100ms % ETH_BOOTDONE_PERIOD) == 0 && xEthCtrl.WaitAckCode == 0)
            {
                // Boot Done은 ACK를 요청함
                csu_Ethernet_SendMessage(ETH_CODE_BOOT_DONE, ETH_ACK_REQ, NULL, 0);
                xEthCtrl.State = ETH_STATE_BOOT_DONE;
            }
            break;
            
        case ETH_STATE_BOOT_DONE:
            // 망 가입 중 재전송 로직이 1번에서 처리되므로 ACK 오기를 기다림
            break;
            
        case ETH_STATE_LINKED:
            // 망 가입 완료 (Step 4: Heartbeat)
            // 100ms 주기 상태정보 통신
            if ((xEthCtrl.TickCount100ms % ETH_HEARTBEAT_PERIOD) == 0 && xEthCtrl.WaitAckCode == 0)
            {
                // Heartbeat 페이로드의 첫 번째 바이트에 270V 전원 상태를 싣는다고 가정
                uint8_t statusPayload[1];
                statusPayload[0] = xEthCtrl.Power270VStatus;
                
                csu_Ethernet_SendMessage(ETH_CODE_HEARTBEAT, ETH_ACK_NOT_REQ, statusPayload, 1);
            }
            
            // 통신 두절 감시 (100ms * 50회 = 5초간 수신 없으면 리셋)
            xEthCtrl.TimeoutCount++;
            if (xEthCtrl.TimeoutCount >= ETH_DISCONNECT_LIMIT)
            {
                // 소켓 리셋 및 망 가입 초기화
                socket(SOCK_UDP_COM, Sn_MR_UDP, PORT_UDP_COM, 0x00);
                xEthCtrl.State = ETH_STATE_INIT;
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
                    csu_Ethernet_SendMessage(ETH_CODE_CBIT_REP, ETH_ACK_NOT_REQ, NULL, 0);
                }
            }
            break;
    }
}

/*
@function   csu_Ethernet_ParsePacket(uint8_t *pRxBuf, uint16_t length)
@brief      수신 패킷 파싱 및 명령 수행
*/
void csu_Ethernet_ParsePacket(uint8_t *pRxBuf, uint16_t length)
{
    // 최소 길이 (Header 12 + Checksum 2) 검증
    if (length < 14) return;
    
    // 체크섬 검증
    uint16_t recvChecksum;
    memcpy(&recvChecksum, &pRxBuf[length - 2], 2);
    
    uint16_t calcChecksum = csu_Ethernet_CalculateChecksum(pRxBuf, length);
    
    stEthHeader *pHeader = (stEthHeader *)pRxBuf;
    
    // 에러 발생 시 처리 (체크섬 오류 시 NACK)
    if (recvChecksum != calcChecksum)
    {
        if (pHeader->Request_ACK == ETH_ACK_REQ && pHeader->Code != ETH_CODE_ACK)
        {
            // 수신 패킷의 Timestamp는 일단 적용 후 NACK 전송
            xEthCtrl.LastRecvTimestamp = pHeader->Timestamp;
            csu_Ethernet_SendAck(pHeader->Code, ETH_ACK_INFO_CS_ERR, ETH_ACK_NACK);
        }
        return; 
    }
    
    // 패킷 정상 수신 -> 타임아웃 타이머 초기화 (통신 두절 방어)
    xEthCtrl.TimeoutCount = 0;
    xEthCtrl.LastRecvTimestamp = pHeader->Timestamp;
    
    // 정상 ACK 응답 처리
    if (pHeader->Request_ACK == ETH_ACK_REQ && pHeader->Code != ETH_CODE_ACK)
    {
        csu_Ethernet_SendAck(pHeader->Code, ETH_ACK_INFO_OK, ETH_ACK_NORMAL);
    }
    
    // 메시지 Code 분석 및 동작
    switch (pHeader->Code)
    {
        case ETH_CODE_ACK:
            // 내가 보낸 메시지에 대한 응답이 온 경우
            if (length >= 18) // 고정 18바이트 (Header 12 + Data 4 + CS 2)
            {
                stEthAckMessage *pAck = (stEthAckMessage *)pRxBuf;
                // 현재 대기 중인 ACK 코드와 일치하는가?
                if (xEthCtrl.WaitAckCode == pAck->Code_Info && pAck->Ack_Info == ETH_ACK_INFO_OK)
                {
                    // 재전송 중지 및 대기 해제
                    xEthCtrl.WaitAckCode = 0;
                    xEthCtrl.RetryCount = 0;
                    
                    // 만약 Boot Done 전송 중이었는데 ACK가 왔다면 망 가입 완료
                    if (xEthCtrl.State == ETH_STATE_BOOT_DONE && pAck->Code_Info == ETH_CODE_BOOT_DONE)
                    {
                        xEthCtrl.State = ETH_STATE_LINKED;
                    }
                }
            }
            break;
            
        case ETH_CODE_PBIT_REQ:
            // PBIT 결과 전송
            csu_Ethernet_SendMessage(ETH_CODE_PBIT_REP, ETH_ACK_NOT_REQ, NULL, 0);
            break;
            
        case ETH_CODE_IBIT_REQ:
            xEthCtrl.IbitInProgress = 1;
            // TODO: 실제 IBIT 검증 함수 연동 필요. (수행 완료 후 결과 전송)
            // 임시로 바로 완료되었다고 가정하고 IBIT_REP 전송
            csu_Ethernet_SendMessage(ETH_CODE_IBIT_REP, ETH_ACK_NOT_REQ, NULL, 0);
            xEthCtrl.IbitInProgress = 0;
            break;
            
        case ETH_CODE_POWER_270V:
            // 270VDC 구동 전원 인가 메시지 수신 (상태정보에 반영)
            xEthCtrl.Power270VStatus = 1;
            break;
            
        case ETH_CODE_CBIT_SET:
            if (pHeader->Data_Length >= 2)
            {
                // 주기 설정값 파싱 (예: 페이로드 첫 2바이트가 N초)
                uint16_t periodSec = 0;
                memcpy(&periodSec, &pRxBuf[sizeof(stEthHeader)], 2);
                xEthCtrl.CbitPeriodSec = periodSec;
                xEthCtrl.CbitTimer100ms = 0;
            }
            break;
            
        case ETH_CODE_HEARTBEAT:
            // 체계에서 Heartbeat 요청이 오는 경우 응답 (현재 상태 전송)
            {
                uint8_t statusPayload[1];
                statusPayload[0] = xEthCtrl.Power270VStatus;
                csu_Ethernet_SendMessage(ETH_CODE_HEARTBEAT, ETH_ACK_NOT_REQ, statusPayload, 1);
            }
            break;
            
        default:
            break;
    }
}
