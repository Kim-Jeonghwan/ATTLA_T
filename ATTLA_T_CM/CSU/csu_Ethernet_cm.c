/**********************************************************************
   Nexcom Co., Ltd.
   Filename         : csu_Ethernet_cm.c
   Version          : 00.07
   Description      : CM 코어 체계 이더넷(Raw UDP) 연동통제안 및 프로토콜 구현
   Programmer       : Kim Jeonghwan
   Last Updated     : 2026. 06. 24. (파일명 리팩토링)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 24. - 파일명 리팩토링 (_cm 분리)
 * 2026. 06. 23. - 체계 연동통제안(ICD) 상태머신 및 메시지 처리 구현
 */

#include "csu_Ethernet_cm.h"

/* 글로벌 상태 구조체 인스턴스 */
stEthControl xEthCtrl;

/* TX 패킷 디스크립터 풀 (단일 변수 사용 시 큐 꼬임 방지) */
static Ethernet_Pkt_Desc s_xTxPktDesc[ETH_TX_NUM_PKT_DESC];
static uint8_t s_ucTxPktDescIdx = 0U;

/* 공유 데이터 전역 변수 */
stEthAppState xEthApp = {
    .txData = {0.0f, 0U, 0U, 0U},
    .rxData = {0.0f, 0U, 0U, 0U},
    .realPcMac = {ETH_PC_MAC0, ETH_PC_MAC1, ETH_PC_MAC2, ETH_PC_MAC3, ETH_PC_MAC4, ETH_PC_MAC5},
    .lastRxSrcPort = 0U
};

/* 이더넷 활동(Tx/Rx) LED 표시용 타이머 (1ms 주기 감소) */
uint16_t ethActivityTimer = 0U;

/* ---------------------------------------------------------------
 * static 함수 선언
 * --------------------------------------------------------------- */
static uint16_t calcUdpMsgChecksum(const uint8_t *pBuf, uint16_t length);
static uint16_t calcIPChecksum(const uint8_t *pIPHdr);
static void     buildEthernetHeader(uint8_t *pFrame);
static void     buildIPHeader(uint8_t *pFrame, uint16_t udpPayloadLen);
static void     buildUDPHeader(uint8_t *pFrame, uint16_t payloadLen);
static bool     sendEthernetFrame(uint8_t *pFrame, uint16_t frameSize);

/* ---------------------------------------------------------------
 * IP 헤더 체크섬 계산 (RFC791)
 * --------------------------------------------------------------- */
static uint16_t calcIPChecksum(const uint8_t *pIPHdr)
{
    uint32_t uiSum   = 0U;
    uint16_t uiWord  = 0U;
    uint8_t  i       = 0U;
    uint16_t uiRet   = 0U;

    if (pIPHdr != NULL)
    {
        /* 16비트 단위로 합산 (20B = 10 words) */
        for (i = 0U; i < IP_HDR_SIZE; i += 2U)
        {
            uiWord = ((uint16_t)pIPHdr[i] << 8U) | (uint16_t)pIPHdr[i + 1U];
            uiSum += (uint32_t)uiWord;
        }

        /* 캐리 접기 */
        while ((uiSum >> 16U) != 0U)
        {
            uiSum = (uiSum & 0x0000FFFFU) + (uiSum >> 16U);
        }

        uiRet = (uint16_t)(~uiSum);
    }

    return uiRet;
}

/* ---------------------------------------------------------------
 * UDP 메시지 Payload 체크섬 계산 (규격서: 최하위 2바이트)
 * --------------------------------------------------------------- */
static uint16_t calcUdpMsgChecksum(const uint8_t *pBuf, uint16_t length)
{
    uint32_t uiSum  = 0U;
    uint16_t i      = 0U;
    uint16_t uiRet  = 0U;

    if (pBuf != NULL)
    {
        for (i = 0U; i < length; i++)
        {
            uiSum += (uint32_t)pBuf[i];
        }
        uiRet = (uint16_t)(uiSum & 0x0000FFFFU);
    }

    return uiRet;
}

/* ---------------------------------------------------------------
 * 이더넷 헤더 조립
 * --------------------------------------------------------------- */
static void buildEthernetHeader(uint8_t *pFrame)
{
    if (pFrame != NULL)
    {
        /* Destination MAC: 동적으로 캡처된 PC MAC */
        pFrame[0U] = xEthApp.realPcMac[0];
        pFrame[1U] = xEthApp.realPcMac[1];
        pFrame[2U] = xEthApp.realPcMac[2];
        pFrame[3U] = xEthApp.realPcMac[3];
        pFrame[4U] = xEthApp.realPcMac[4];
        pFrame[5U] = xEthApp.realPcMac[5];

        /* 출발지 MAC: DSP MAC A8:63:F2:00:38:88 */
        pFrame[6U]  = ETH_DSP_MAC0;
        pFrame[7U]  = ETH_DSP_MAC1;
        pFrame[8U]  = ETH_DSP_MAC2;
        pFrame[9U]  = ETH_DSP_MAC3;
        pFrame[10U] = ETH_DSP_MAC4;
        pFrame[11U] = ETH_DSP_MAC5;

        /* EtherType: IPv4 (0x0800) Big Endian */
        pFrame[12U] = 0x08U;
        pFrame[13U] = 0x00U;
    }
}

/* ---------------------------------------------------------------
 * IP 헤더 조립
 * --------------------------------------------------------------- */
static void buildIPHeader(uint8_t *pFrame, uint16_t udpPayloadLen)
{
    if (pFrame != NULL)
    {
        uint8_t  *pIP       = pFrame + ETH_HDR_SIZE;
        uint16_t  uiTotalLen = (uint16_t)IP_HDR_SIZE + (uint16_t)UDP_HDR_SIZE + udpPayloadLen;
        uint16_t  uiChksum   = 0U;

        /* Version=4, IHL=5 */
        pIP[0U]  = IP_HDR_VER_IHL;
        /* DSCP/ECN */
        pIP[1U]  = IP_HDR_DSCP;
        /* Total Length (Big Endian) */
        pIP[2U]  = (uint8_t)(uiTotalLen >> 8U);
        pIP[3U]  = (uint8_t)(uiTotalLen & 0x00FFU);
        /* Identification: 0 */
        pIP[4U]  = 0x00U;
        pIP[5U]  = 0x00U;
        /* Flags + Fragment Offset: Don't Fragment */
        pIP[6U]  = 0x40U;
        pIP[7U]  = 0x00U;
        /* TTL */
        pIP[8U]  = IP_TTL;
        /* Protocol: UDP */
        pIP[9U]  = IP_PROTO_UDP;
        /* Header Checksum: 먼저 0으로 클리어 후 계산 */
        pIP[10U] = 0x00U;
        pIP[11U] = 0x00U;
        /* Source IP */
        pIP[12U] = ETH_DSP_IP0;
        pIP[13U] = ETH_DSP_IP1;
        pIP[14U] = ETH_DSP_IP2;
        pIP[15U] = ETH_DSP_IP3;
        /* Destination IP */
        pIP[16U] = ETH_PC_IP0;
        pIP[17U] = ETH_PC_IP1;
        pIP[18U] = ETH_PC_IP2;
        pIP[19U] = ETH_PC_IP3;

        /* IP 헤더 체크섬 계산 후 삽입 (Big Endian) */
        uiChksum = calcIPChecksum(pIP);
        pIP[10U] = (uint8_t)(uiChksum >> 8U);
        pIP[11U] = (uint8_t)(uiChksum & 0x00FFU);
    }
}

/* ---------------------------------------------------------------
 * UDP 헤더 조립
 * --------------------------------------------------------------- */
static void buildUDPHeader(uint8_t *pFrame, uint16_t payloadLen)
{
    if (pFrame != NULL)
    {
        uint8_t  *pUDP     = pFrame + UDP_HDR_OFFSET;
        uint16_t  uiUdpLen = (uint16_t)UDP_HDR_SIZE + payloadLen;

        /* Source Port: DSP RX Port (5001, Big Endian) */
        pUDP[0U] = (uint8_t)(ETH_DSP_RX_PORT >> 8U);
        pUDP[1U] = (uint8_t)(ETH_DSP_RX_PORT & 0x00FFU);
        
        /* Destination Port: PC RX Port */
        uint16_t pcDestPort = (xEthApp.lastRxSrcPort != 0U) ? xEthApp.lastRxSrcPort : ETH_PC_RX_PORT;
        pUDP[2U] = (uint8_t)(pcDestPort >> 8U);
        pUDP[3U] = (uint8_t)(pcDestPort & 0x00FFU);
        
        /* Length (Big Endian) */
        pUDP[4U] = (uint8_t)(uiUdpLen >> 8U);
        pUDP[5U] = (uint8_t)(uiUdpLen & 0x00FFU);
        /* UDP Checksum: 0 (비활성) */
        pUDP[6U] = 0x00U;
        pUDP[7U] = 0x00U;
    }
}

/* ---------------------------------------------------------------
 * 이더넷 드라이버 패킷 전송
 * --------------------------------------------------------------- */
static bool sendEthernetFrame(uint8_t *pFrame, uint16_t frameSize)
{
    bool bRet = false;

    if ((xEthDriver.hEMAC != (Ethernet_Handle)0U) && (pFrame != NULL))
    {
        Ethernet_Pkt_Desc *pTxDesc = &s_xTxPktDesc[s_ucTxPktDescIdx];

        pTxDesc->dataBuffer     = pFrame;
        pTxDesc->dataOffset     = 0U;
        pTxDesc->validLength    = (uint32_t)frameSize;
        pTxDesc->bufferLength   = (uint32_t)frameSize;
        pTxDesc->pktLength      = (uint32_t)frameSize;
        pTxDesc->pktChannel     = ETHERNET_DMA_CHANNEL_NUM_0;
        pTxDesc->numPktFrags    = 1U;
        pTxDesc->flags          = ETHERNET_PKT_FLAG_SOP | ETHERNET_PKT_FLAG_EOP;
        pTxDesc->nextPacketDesc = NULL;

        uint32_t retCode = Ethernet_sendPacket(xEthDriver.hEMAC, pTxDesc);
        
        if (retCode == 0U)
        {
            s_ucTxPktDescIdx = (s_ucTxPktDescIdx + 1U) % ETH_TX_NUM_PKT_DESC;
            bRet = true;
        }
    }

    return bRet;
}

/* ---------------------------------------------------------------
 * 체계 연동통제안 UDP 패킷 송신 (직렬화 및 체크섬 포함)
 * --------------------------------------------------------------- */
/*
@function   buildAndSendUdpPacket
@brief      주어진 메시지 Code와 Payload를 체계 스펙에 맞추어 UDP 패킷으로 조립하여 송신합니다.
@param      rxTimestamp : 화포컴퓨터의 최신 Timestamp (Ack 회신 시 필수)
@param      msgCode     : 메시지 명령어 Code
@param      reqAck      : ACK 필요 여부 플래그
@param      pData       : Payload 데이터 버퍼 포인터 (NULL 허용)
@param      dataLen     : Payload 크기
*/
void buildAndSendUdpPacket(uint32_t rxTimestamp, uint8_t msgCode, uint8_t reqAck, const uint8_t *pData, uint16_t dataLen)
{
    uint8_t  *pPayload = xEthDriver.txBuf + PAYLOAD_OFFSET;
    uint16_t  offset = 0U;
    uint16_t  totalPayloadLen = ETH_MSG_HEADER_SIZE + dataLen + ETH_CHECKSUM_SIZE;
    uint16_t  i;

    /* ---- 1. MSG Header 조립 (12B) ---- */
    /* Timestamp (4B, Little Endian) */
    pPayload[offset++] = (uint8_t)(rxTimestamp & 0x000000FFU);
    pPayload[offset++] = (uint8_t)((rxTimestamp >>  8U) & 0x000000FFU);
    pPayload[offset++] = (uint8_t)((rxTimestamp >> 16U) & 0x000000FFU);
    pPayload[offset++] = (uint8_t)((rxTimestamp >> 24U) & 0x000000FFU);

    pPayload[offset++] = ETH_MY_ID;    /* Source ID */
    pPayload[offset++] = ETH_FC_ID;    /* Dest ID */
    pPayload[offset++] = msgCode;      /* Code */
    pPayload[offset++] = reqAck;       /* Request ACK */
    pPayload[offset++] = ETH_PRIORITY_NORMAL; /* Priority */

    /* Send Count */
    uint8_t sendCount = 1U;
    if (xEthCtrl.WaitAckCode == msgCode && xEthCtrl.RetryCount > 0U)
    {
        sendCount = (uint8_t)(xEthCtrl.RetryCount);
    }
    pPayload[offset++] = sendCount;

    /* Data Length (2B, Little Endian) */
    pPayload[offset++] = (uint8_t)(dataLen & 0x00FFU);
    pPayload[offset++] = (uint8_t)(dataLen >> 8U);

    /* ---- 2. Payload(Data) 조립 ---- */
    if (dataLen > 0U && pData != NULL)
    {
        for (i = 0U; i < dataLen; i++)
        {
            pPayload[offset++] = pData[i];
        }
    }
    }
    else if (msgCode == ETH_CODE_CBIT_REP)
    {
        /* CBIT 임시 처리 대체됨 (payload 조립 시 pData가 NULL이 아님) */
        for (i = 0U; i < dataLen; i++)
        {
            pPayload[offset++] = pData[i];
        }
    }
    else
    {
        /* 방어 코드 */
    }

    /* ---- 3. Checksum 계산 및 직렬화 ---- */
    uint16_t checksum = calcUdpMsgChecksum(pPayload, offset);
    pPayload[offset++] = (uint8_t)(checksum & 0x00FFU);
    pPayload[offset++] = (uint8_t)(checksum >> 8U);

    /* ---- 4. 이더넷/IP/UDP 헤더 조립 및 송신 ---- */
    buildEthernetHeader(xEthDriver.txBuf);
    buildIPHeader(xEthDriver.txBuf, totalPayloadLen);
    buildUDPHeader(xEthDriver.txBuf, totalPayloadLen);

    /* Tx 활동 LED 점등 */
    ETH_LED_ON();
    ethActivityTimer = 20U;

    uint16_t totalFrameSize = ETH_HDR_SIZE + IP_HDR_SIZE + UDP_HDR_SIZE + totalPayloadLen;
    (void)sendEthernetFrame(xEthDriver.txBuf, totalFrameSize);

    /* ---- 5. ACK 대기 타이머 세팅 ---- */
    if (reqAck == ETH_ACK_REQ)
    {
        xEthCtrl.WaitAckCode = msgCode;
        xEthCtrl.WaitAckTimer = 0U;
        if (xEthCtrl.RetryCount == 0U)
        {
            xEthCtrl.RetryCount = 1U;
            /* 백업 버퍼 저장 */
            xEthCtrl.TxSize = totalFrameSize;
            (void)memcpy(xEthCtrl.TxBuffer, xEthDriver.txBuf, totalFrameSize);
        }
    }
}

/* ---------------------------------------------------------------
 * 체계 수신 패킷 파싱 및 명령 해독
 * --------------------------------------------------------------- */
void processReceivedEthernetPacket(uint8_t *pPacket, uint16_t length)
{
    if ((pPacket != NULL) && (length >= (uint16_t)MIN_RX_FRAME_SIZE))
    {
        uint16_t ethType = ((uint16_t)pPacket[ETH_HDR_TYPE_OFFSET] << 8U) | (uint16_t)pPacket[ETH_HDR_TYPE_OFFSET + 1U];
        uint16_t i;

        /* ---- [1] ARP Request에 대한 실시간 응답 (Auto-Learning 보장) ---- */
        if (ethType == 0x0806U)
        {
            /* ARP Request(Opcode 1)인지 확인 */
            if ((pPacket[20U] == 0x00U) && (pPacket[21U] == 0x01U))
            {
                /* Target IP가 DSP IP인지 확인 */
                if ((pPacket[38U] == ETH_DSP_IP0) && (pPacket[39U] == ETH_DSP_IP1) &&
                    (pPacket[40U] == ETH_DSP_IP2) && (pPacket[41U] == ETH_DSP_IP3))
                {
                    ETH_LED_ON();
                    ethActivityTimer = 20U;

                    static uint8_t arpReply[60];
                    (void)memset(arpReply, 0U, sizeof(arpReply));

                    /* Ethernet Header (Dst MAC = Sender MAC) */
                    for (i = 0U; i < 6U; i++)
                    {
                        arpReply[i] = pPacket[22U + i];
                    }
                    /* Src MAC = DSP MAC */
                    arpReply[6U]  = ETH_DSP_MAC0; arpReply[7U]  = ETH_DSP_MAC1; arpReply[8U]  = ETH_DSP_MAC2;
                    arpReply[9U]  = ETH_DSP_MAC3; arpReply[10U] = ETH_DSP_MAC4; arpReply[11U] = ETH_DSP_MAC5;
                    arpReply[12U] = 0x08U; arpReply[13U] = 0x06U;

                    /* ARP Payload */
                    arpReply[14U] = 0x00U; arpReply[15U] = 0x01U; /* HTYPE: Ethernet */
                    arpReply[16U] = 0x08U; arpReply[17U] = 0x00U; /* PTYPE: IPv4 */
                    arpReply[18U] = 0x06U; arpReply[19U] = 0x04U; /* HLEN: 6, PLEN: 4 */
                    arpReply[20U] = 0x00U; arpReply[21U] = 0x02U; /* Opcode: Reply(2) */

                    /* Sender MAC = DSP MAC */
                    for (i = 0U; i < 6U; i++)
                    {
                        arpReply[22U + i] = arpReply[6U + i];
                    }
                    /* Sender IP = DSP IP */
                    arpReply[28U] = ETH_DSP_IP0; arpReply[29U] = ETH_DSP_IP1; arpReply[30U] = ETH_DSP_IP2; arpReply[31U] = ETH_DSP_IP3;

                    /* Target MAC = PC MAC */
                    for (i = 0U; i < 6U; i++)
                    {
                        arpReply[32U + i] = pPacket[22U + i];
                    }
                    /* Target IP = PC IP */
                    for (i = 0U; i < 4U; i++)
                    {
                        arpReply[38U + i] = pPacket[28U + i];
                    }

                    (void)sendEthernetFrame(arpReply, 60U);
                }
            }
        }
        /* ---- [2] IPv4 / UDP 데이터 수신 처리 ---- */
        else if (ethType == 0x0800U)
        {
            if (pPacket[IP_HDR_OFFSET + 9U] == IP_PROTO_UDP)
            {
                uint16_t uiDstPort = ((uint16_t)pPacket[UDP_HDR_OFFSET + 2U] << 8U) |
                                     (uint16_t)pPacket[UDP_HDR_OFFSET + 3U];

                if (uiDstPort == ETH_DSP_RX_PORT)
                {
                    ETH_LED_ON();
                    ethActivityTimer = 20U;

                    xEthApp.lastRxSrcPort = ((uint16_t)pPacket[UDP_HDR_OFFSET] << 8U) |
                                             (uint16_t)pPacket[UDP_HDR_OFFSET + 1U];

                    /* PC의 실제 MAC 획득 */
                    for (i = 0U; i < 6U; i++)
                    {
                        xEthApp.realPcMac[i] = pPacket[6U + i];
                    }

                    uint8_t *pPayload = pPacket + PAYLOAD_OFFSET;

                    /* Dest ID가 나(ATTLA_T) 혹은 PC로부터 온 것인지 검사 */
                    if (pPayload[5U] == ETH_MY_ID || pPayload[5U] == ETH_FC_ID)
                    {
                        uint32_t rxTimestamp = ((uint32_t)pPayload[3U] << 24U) |
                                               ((uint32_t)pPayload[2U] << 16U) |
                                               ((uint32_t)pPayload[1U] <<  8U) |
                                                (uint32_t)pPayload[0U];

                        uint8_t  recvCode   = pPayload[6U];
                        uint8_t  requestAck = pPayload[7U];
                        uint16_t dataLength = ((uint16_t)pPayload[11U] << 8U) | (uint16_t)pPayload[10U];

                        /* 체크섬 검증 */
                        uint16_t calcChk = calcUdpMsgChecksum(pPayload, (uint16_t)(ETH_MSG_HEADER_SIZE + dataLength));
                        uint16_t recvChk = ((uint16_t)pPayload[ETH_MSG_HEADER_SIZE + dataLength + 1U] << 8U) |
                                            (uint16_t)pPayload[ETH_MSG_HEADER_SIZE + dataLength];

                        if (calcChk != recvChk)
                        {
                            if (requestAck == ETH_ACK_REQ && recvCode != ETH_CODE_ACK)
                            {
                                xEthCtrl.LastRecvTimestamp = rxTimestamp;
                                sendAckResponse(ETH_ACK_NACK, ETH_ACK_INFO_CS_ERR, rxTimestamp, recvCode);
                            }
                            return; 
                        }

                        /* 정상 패킷 수신 -> 타임아웃 초기화 */
                        xEthCtrl.TimeoutCount = 0U;
                        xEthCtrl.LastRecvTimestamp = rxTimestamp;

                        /* ACK 응답 처리 */
                        if (requestAck == ETH_ACK_REQ && recvCode != ETH_CODE_ACK)
                        {
                            sendAckResponse(ETH_ACK_NORMAL, ETH_ACK_INFO_OK, rxTimestamp, recvCode);
                        }

                        /* 메시지 해석 */
                        switch (recvCode)
                        {
                            case ETH_CODE_ACK:
                                if (dataLength == 4U)
                                {
                                    uint16_t codeInfo = ((uint16_t)pPayload[13U] << 8U) | (uint16_t)pPayload[12U];
                                    uint16_t ackInfo  = ((uint16_t)pPayload[15U] << 8U) | (uint16_t)pPayload[14U];

                                    if (xEthCtrl.WaitAckCode == codeInfo && ackInfo == ETH_ACK_INFO_OK)
                                    {
                                        /* 재전송 해제 */
                                        xEthCtrl.WaitAckCode = 0U;
                                        xEthCtrl.RetryCount = 0U;

                                        /* Boot Done ACK 면 망가입 완료 */
                                        if (xEthCtrl.State == STATE_WAIT_BOOT_ACK && codeInfo == ETH_CODE_BOOT_DONE)
                                        {
                                            xEthCtrl.State = STATE_JOINED;
                                        }
                                        /* IBIT_DONE ACK 수신 시 CBIT 재개 */
                                        if (codeInfo == ETH_CODE_IBIT_DONE)
                                        {
                                            xEthCtrl.IbitInProgress = 0U;
                                        }
                                    }
                                }
                                break;
                            case ETH_CODE_STATUS_REQ:
                                /* 통신 상태 확인 요청 수신 -> 100ms 통신 두절 방어용 */
                                pxDataCmToCpu1->seqCount++;
                                pxDataCmToCpu1->Payload.RxData.reserved1 = 0U;
                                pxDataCmToCpu1->seqCount++;
                                break;


                            case ETH_CODE_PBIT_REQ:
                                {
                                    uint32_t bitResult = pxDataCpu1ToCm->Payload.TxData.bitInformAll;
                                    uint8_t payload[4];
                                    payload[0] = (uint8_t)(bitResult & 0xFFU);
                                    payload[1] = (uint8_t)((bitResult >> 8U) & 0xFFU);
                                    payload[2] = (uint8_t)((bitResult >> 16U) & 0xFFU);
                                    payload[3] = (uint8_t)((bitResult >> 24U) & 0xFFU);
                                    buildAndSendUdpPacket(rxTimestamp, ETH_CODE_PBIT_REP, ETH_ACK_NOT_REQ, payload, 4U);
                                }
                                break;

                            case ETH_CODE_IBIT_REQ:
                                if (dataLength >= 2U)
                                {
                                    uint16_t durationSec = ((uint16_t)pPayload[13U] << 8U) | (uint16_t)pPayload[12U];
                                    xEthCtrl.IbitDuration = durationSec;
                                    xEthCtrl.IbitInProgress = 1U;
                                    xEthCtrl.IbitTimer = durationSec * 10U; /* 100ms 주기 변환 */
                                    
                                    /* CPU1 측에 에러 클리어 요청 (IPC) */
                                    pxDataCmToCpu1->seqCount++;
                                    pxDataCmToCpu1->Payload.RxData.ibitClearReq = 1U;
                                    pxDataCmToCpu1->seqCount++;
                                }
                                break;
                                
                            case ETH_CODE_IBIT_RES_REQ:
                                {
                                    uint32_t bitResult = pxDataCpu1ToCm->Payload.TxData.bitInformAll;
                                    uint8_t payload[4];
                                    payload[0] = (uint8_t)(bitResult & 0xFFU);
                                    payload[1] = (uint8_t)((bitResult >> 8U) & 0xFFU);
                                    payload[2] = (uint8_t)((bitResult >> 16U) & 0xFFU);
                                    payload[3] = (uint8_t)((bitResult >> 24U) & 0xFFU);
                                    buildAndSendUdpPacket(rxTimestamp, ETH_CODE_IBIT_REP, ETH_ACK_NOT_REQ, payload, 4U);
                                }
                                break;

                            case ETH_CODE_CBIT_SET:
                                if (dataLength >= 2U)
                                {
                                    uint16_t periodSec = ((uint16_t)pPayload[13U] << 8U) | (uint16_t)pPayload[12U];
                                    xEthCtrl.CbitPeriodSec = periodSec;
                                    xEthCtrl.CbitTimer100ms = 0U;
                                }
                                break;

                            case ETH_CODE_CBIT_STOP:
                                xEthCtrl.CbitTxFlag = 0U;
                                break;
                                
                            case ETH_CODE_CBIT_REQ:
                                xEthCtrl.CbitTxFlag = 1U;
                                break;

                            default:
                                break;
                        }
                    }
                }
            }
        }
        else
        {
            /* 방어 코드 */
        }
    }
}

/* ---------------------------------------------------------------
 * ACK 전용 패킷 송신
 * --------------------------------------------------------------- */
void sendAckResponse(uint8_t ackResult, uint16_t ackInfo, uint32_t timestamp, uint8_t targetCode)
{
    static uint8_t s_ucAckBuf[TX_ACK_FRAME_SIZE];
    uint8_t        *pPayload = s_ucAckBuf + PAYLOAD_OFFSET;
    uint16_t        offset = 0U;
    uint16_t        totalPayloadLen = ETH_MSG_HEADER_SIZE + ETH_ACK_DATA_SIZE + ETH_CHECKSUM_SIZE;

    /* MSG Header (12B) */
    pPayload[offset++] = (uint8_t)(timestamp & 0x000000FFU);
    pPayload[offset++] = (uint8_t)((timestamp >>  8U) & 0x000000FFU);
    pPayload[offset++] = (uint8_t)((timestamp >> 16U) & 0x000000FFU);
    pPayload[offset++] = (uint8_t)((timestamp >> 24U) & 0x000000FFU);

    pPayload[offset++] = ETH_MY_ID;
    pPayload[offset++] = ETH_FC_ID;
    pPayload[offset++] = ETH_CODE_ACK;
    pPayload[offset++] = ackResult;      /* 0x10=ACK / 0x11=NACK */
    pPayload[offset++] = 0U;
    pPayload[offset++] = 1U;             /* Send Count */

    pPayload[offset++] = (uint8_t)(ETH_ACK_DATA_SIZE & 0x00FFU);
    pPayload[offset++] = (uint8_t)(ETH_ACK_DATA_SIZE >> 8U);

    /* Data (4B) */
    pPayload[offset++] = targetCode;
    pPayload[offset++] = 0U;
    pPayload[offset++] = (uint8_t)(ackInfo & 0x00FFU);
    pPayload[offset++] = (uint8_t)(ackInfo >> 8U);

    /* Checksum (2B) */
    uint16_t checksum = calcUdpMsgChecksum(pPayload, offset);
    pPayload[offset++] = (uint8_t)(checksum & 0x00FFU);
    pPayload[offset++] = (uint8_t)(checksum >> 8U);

    buildEthernetHeader(s_ucAckBuf);
    buildIPHeader(s_ucAckBuf, totalPayloadLen);
    buildUDPHeader(s_ucAckBuf, totalPayloadLen);

    (void)sendEthernetFrame(s_ucAckBuf, TX_ACK_FRAME_SIZE);
}

/* ---------------------------------------------------------------
 * 100ms 주기 연동통제안 상태 머신 구동 (CM 코어 구동)
 * --------------------------------------------------------------- */
void Ethernet_StateMachine(void)
{
    xEthCtrl.TickCount100ms++;

    /* [1] 재전송 및 타임아웃 감시 로직 */
    if (xEthCtrl.WaitAckCode != 0U)
    {
        xEthCtrl.WaitAckTimer++;
        if (xEthCtrl.WaitAckTimer >= ETH_ACK_TIMEOUT)
        {
            xEthCtrl.RetryCount++;
            
            if (xEthCtrl.RetryCount > ETH_MAX_RETRY_COUNT)
            {
                /* 4회 전송 실패 -> 망 이탈 및 롤백 */
                xEthCtrl.WaitAckCode = 0U;
                xEthCtrl.RetryCount = 0U;
                xEthCtrl.State = STATE_WAIT_BOOT_ACK;
                xEthCtrl.TimeoutCount = 0U;
            }
            else
            {
                /* 기존 백업 버퍼 재송신 (Send_Count만 업데이트) */
                if (xEthCtrl.TxSize >= PAYLOAD_OFFSET + 12U)
                {
                    uint8_t *pPayload = xEthCtrl.TxBuffer + PAYLOAD_OFFSET;
                    pPayload[9U] = (uint8_t)xEthCtrl.RetryCount;
                    
                    /* 체크섬 재계산 및 갱신 */
                    uint16_t chkLen = xEthCtrl.TxSize - PAYLOAD_OFFSET - 2U;
                    uint16_t cs = calcUdpMsgChecksum(pPayload, chkLen);
                    xEthCtrl.TxBuffer[xEthCtrl.TxSize - 2U] = (uint8_t)(cs & 0x00FFU);
                    xEthCtrl.TxBuffer[xEthCtrl.TxSize - 1U] = (uint8_t)(cs >> 8U);

                    (void)sendEthernetFrame(xEthCtrl.TxBuffer, xEthCtrl.TxSize);
                }
                xEthCtrl.WaitAckTimer = 0U;
            }
        }
    }

    /* [2] 상태 머신 구동 */
    switch (xEthCtrl.State)
    {
        case STATE_BOOTING:
            xEthCtrl.State = STATE_WAIT_BOOT_ACK;
            break;

        case STATE_WAIT_BOOT_ACK:
            /* 500ms(5 ticks) 마다 Boot Done 전송 */
            if (((xEthCtrl.TickCount100ms % ETH_BOOTDONE_PERIOD) == 0U) && (xEthCtrl.WaitAckCode == 0U))
            {
                buildAndSendUdpPacket(0U, ETH_CODE_BOOT_DONE, ETH_ACK_REQ, NULL, 0U);
            }
            break;

        case STATE_JOINED:

            /* 통신 두절 감시 (5초 미수신 시 롤백) */
            xEthCtrl.TimeoutCount++;
            if (xEthCtrl.TimeoutCount >= ETH_DISCONNECT_LIMIT)
            {
                xEthCtrl.State = STATE_WAIT_BOOT_ACK;
                xEthCtrl.TimeoutCount = 0U;
                xEthCtrl.WaitAckCode = 0U;
                break;
            }

            /* CBIT 주기 전송 */
            if (xEthCtrl.CbitTxFlag == 1U && xEthCtrl.CbitPeriodSec > 0U && xEthCtrl.IbitInProgress == 0U)
            {
                xEthCtrl.CbitTimer100ms++;
                if (xEthCtrl.CbitTimer100ms >= (xEthCtrl.CbitPeriodSec * 10U))
                {
                    xEthCtrl.CbitTimer100ms = 0U;
                    uint32_t bitResult = pxDataCpu1ToCm->Payload.TxData.bitInformAll;
                    uint8_t payload[4];
                    payload[0] = (uint8_t)(bitResult & 0xFFU);
                    payload[1] = (uint8_t)((bitResult >> 8U) & 0xFFU);
                    payload[2] = (uint8_t)((bitResult >> 16U) & 0xFFU);
                    payload[3] = (uint8_t)((bitResult >> 24U) & 0xFFU);
                    buildAndSendUdpPacket(0U, ETH_CODE_CBIT_REP, ETH_ACK_NOT_REQ, payload, 4U);
                }
            }
            
            /* IBIT 지연 처리 (수행 시뮬레이션) */
            if (xEthCtrl.IbitInProgress == 1U && xEthCtrl.IbitTimer > 0U)
            {
                xEthCtrl.IbitTimer--;
                if (xEthCtrl.IbitTimer == 0U)
                {
                    xEthCtrl.IbitInProgress = 2U; // 대기 상태 (결과 요청 올 때까지 대기)
                    buildAndSendUdpPacket(0U, ETH_CODE_IBIT_DONE, ETH_ACK_REQ, NULL, 0U);
                    
                    /* IBIT Clear Req 해제 */
                    pxDataCmToCpu1->seqCount++;
                    pxDataCmToCpu1->Payload.RxData.ibitClearReq = 0U;
                    pxDataCmToCpu1->seqCount++;
                }
            }
            break;

        default:
            break;
    }
}
