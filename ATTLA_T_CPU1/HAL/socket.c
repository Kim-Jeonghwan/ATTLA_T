/**********************************************************************
 Nexcom Co., Ltd.
 Filename         : socket.c
 Version          : 00.05
 Description      : WIZnet 이더넷 라이브러리 파일
 Programmer       : Kim Jeonghwan
 Last Updated     : 2026. 06. 16. (미사용 변수 taddr 제거)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 16. - 미사용 변수 taddr 제거
 * 2026. 06. 16. - UDP 단일화 및 불필요한 통신 로직 전면 삭제
 * 2026. 06. 16. - 사용하지 않는 매크로 및 검사(Check) 매크로 전부 삭제
 *               - DAPA 정적시험(Single Return) 원칙 적용
 *               - IPv6 전용 로직 제거 및 W6100 송수신 함수 통합
 * 2026. 06. 15. - 정적시험 통과를 위한 타기종 및 미사용 TCP/IPv6 기능 전면 삭제
 */

#include "socket.h"

#define SOCK_ANY_PORT_NUM  0xC000

static uint16_t sock_any_port = SOCK_ANY_PORT_NUM;
static uint16_t sock_io_mode = 0;
static uint16_t sock_is_sending = 0;

static uint16_t sock_remained_size[_WIZCHIP_SOCK_NUM_] = {0, 0,};
uint8_t  sock_pack_info[_WIZCHIP_SOCK_NUM_] = {0,};

/*
@function    int8_t socket(uint8_t sn, uint8_t protocol, uint16_t port, uint8_t flag)
@brief      새로운 소켓을 생성하고 지정된 프로토콜(TCP, UDP 등), 포트 및 플래그로 초기화합니다.
@param      sn: 열고자 하는 소켓 번호 (0 ~ 7)
@param      protocol: 사용할 프로토콜 (예: Sn_MR_UDP, Sn_MR_TCP)
@param      port: 바인딩할 로컬 포트 번호. 0일 경우 임의의 포트 자동 할당.
@param      flag: 소켓 동작 플래그 (예: SF_IO_NONBLOCK, SF_MULTI_ENABLE)
@return     성공 시 열린 소켓 번호(sn), 실패 시 에러 코드(SOCKERR_)
*/
int8_t socket(uint8_t sn, uint8_t protocol, uint16_t port, uint8_t flag) {
    int8_t ret_val = -1;
    
    if (sn >= _WIZCHIP_SOCK_NUM_) {
        ret_val = SOCKERR_SOCKNUM;
    } else {
        switch (protocol & 0x0F) {
        case Sn_MR_UDP:
            break;
        default:
            ret_val = SOCKERR_SOCKMODE;
            break;
        }

        if (ret_val == -1) { 
            if ((flag & 0x04) != 0) {
                ret_val = SOCKERR_SOCKFLAG;
            } else {
                if (flag != 0) {
                    switch (protocol) {
                    case Sn_MR_UDP:
                        if (flag & SF_IGMP_VER2) {
                            if ((flag & SF_MULTI_ENABLE) == 0) {
                                ret_val = SOCKERR_SOCKFLAG;
                            }
                        }
                        break;
                    default:
                        break;
                    }
                }
                
                if (ret_val == -1) {
                    close(sn);
                    setSn_MR(sn, (protocol | (flag & 0xF0)));
                    if (!port) {
                        port = sock_any_port++;
                        if (sock_any_port == 0xFFF0) {
                            sock_any_port = SOCK_ANY_PORT_NUM;
                        }
                    }
                    setSn_PORTR(sn, port);
                    setSn_CR(sn, Sn_CR_OPEN);
                    while (getSn_CR(sn));
                    
                    sock_io_mode &= ~(1 << sn);
                    sock_io_mode |= ((flag & SF_IO_NONBLOCK) << sn);
                    sock_is_sending &= ~(1 << sn);
                    sock_remained_size[sn] = 0;
                    sock_pack_info[sn] = PACK_COMPLETED;
                    
                    while (getSn_SR(sn) == SOCK_CLOSED);
                    ret_val = (int8_t)sn;
                }
            }
        }
    }
    return ret_val;
}

/*
@function    int8_t close(uint8_t sn)
@brief      열려있는 소켓을 정상적으로 닫고, 해당 소켓의 모든 인터럽트 및 상태를 초기화합니다.
@param      sn: 닫을 소켓 번호
@return     항상 SOCK_OK 반환
*/
int8_t close(uint8_t sn) {
    int8_t ret_val = SOCK_OK;
    if (sn >= _WIZCHIP_SOCK_NUM_) {
        ret_val = SOCKERR_SOCKNUM;
    } else {
        setSn_CR(sn, Sn_CR_CLOSE);
        while (getSn_CR(sn));
        
        setSn_IR(sn, 0xFF);
        sock_io_mode &= ~(1 << sn);
        sock_is_sending &= ~(1 << sn);
        sock_remained_size[sn] = 0;
        sock_pack_info[sn] = PACK_NONE;
        
        while (getSn_SR(sn) != SOCK_CLOSED);
    }
    return ret_val;
}

/*
@function    int32_t sendto_W6x00(uint8_t sn, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t port, uint8_t addrlen)
@brief      UDP 모드로 설정된 소켓을 통해 목적지 IP 및 포트로 데이터를 전송합니다. (UDP 송신)
@param      sn: 데이터를 전송할 소켓 번호
@param      buf: 전송할 데이터가 담긴 버퍼 포인터
@param      len: 전송할 데이터 길이 (바이트)
@param      addr: 목적지 IP 주소 배열 포인터 (IPv4의 경우 4바이트)
@param      port: 목적지 포트 번호
@param      addrlen: 주소 배열의 길이 (IPv4는 4)
@return     성공 시 전송된 데이터 길이, 실패 시 에러 코드(SOCKERR_)
*/
int32_t sendto_W6x00(uint8_t sn, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t port, uint8_t addrlen) {
    int32_t ret_val = 0;
    uint8_t tmp = 0;
    uint16_t freesize = 0;
    uint32_t taddr;

    if (sn >= _WIZCHIP_SOCK_NUM_) {
        ret_val = SOCKERR_SOCKNUM;
    } else if (len == 0) {
        ret_val = SOCKERR_DATALEN;
    } else {
        tmp = getSn_MR(sn);
        switch (tmp & 0x0F) {
        case Sn_MR_UDP:
            break;
        default:
            ret_val = SOCKERR_SOCKMODE;
            break;
        }

        if (ret_val == 0) {
            if (addrlen == 4) {
                setSn_DIPR(sn, addr);
            } else {
                ret_val = SOCKERR_IPINVALID;
            }
        }

        if (ret_val == 0) {
            if (port) {
                setSn_DPORTR(sn, port);
            } else {
                ret_val = SOCKERR_PORTZERO;
            }
        }

        if (ret_val == 0) {
            taddr = ((uint32_t)addr[0]) & 0x000000FF;
            taddr = (taddr << 8) + ((uint32_t)addr[1] & 0x000000FF);
            taddr = (taddr << 8) + ((uint32_t)addr[2] & 0x000000FF);
            taddr = (taddr << 8) + ((uint32_t)addr[3] & 0x000000FF);

            if (taddr == 0) {
                ret_val = SOCKERR_IPINVALID;
            } else if (port == 0) {
                ret_val = SOCKERR_PORTZERO;
            }
        }

        if (ret_val == 0) {
            uint8_t sr = getSn_SR(sn);
            if (sr != SOCK_UDP) {
                ret_val = SOCKERR_SOCKSTATUS;
            }
        }

        if (ret_val == 0) {
            setSn_DIPR(sn, addr);
            setSn_DPORT(sn, port);

            freesize = getSn_TxMAX(sn);
            if (len > freesize) {
                len = freesize;
            }

            while (1) {
                freesize = getSn_TX_FSR(sn);
                if (getSn_SR(sn) == SOCK_CLOSED) {
                    ret_val = SOCKERR_SOCKCLOSED;
                    break;
                }
                if ((sock_io_mode & (1 << sn)) && (len > freesize)) {
                    ret_val = SOCK_BUSY;
                    break;
                }
                if (len <= freesize) {
                    break;
                }
            }

            if (ret_val == 0) {
                wiz_send_data(sn, buf, len);
                setSn_CR(sn, Sn_CR_SEND);
                while (getSn_CR(sn));
                
                while (1) {
                    tmp = getSn_IR(sn);
                    if (tmp & Sn_IR_SENDOK) {
                        setSn_IR(sn, Sn_IR_SENDOK);
                        ret_val = (int32_t)len;
                        break;
                    } else if (tmp & Sn_IR_TIMEOUT) {
                        setSn_IR(sn, Sn_IR_TIMEOUT);
                        ret_val = SOCKERR_TIMEOUT;
                        break;
                    }
                }
            }
        }
    }
    return ret_val;
}

/*
@function    int32_t recvfrom_W6x00(uint8_t sn, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t *port, uint8_t *addrlen)
@brief      UDP 소켓을 통해 수신된 데이터를 버퍼로 읽어오고, 송신자의 IP와 포트 정보를 획득합니다. (UDP 수신)
@param      sn: 수신할 소켓 번호
@param      buf: 데이터를 저장할 버퍼 포인터
@param      len: 수신할 최대 데이터 길이
@param      addr: 데이터를 보낸 상대방(송신자)의 IP 주소가 저장될 버퍼 포인터
@param      port: 상대방의 포트 번호가 저장될 포인터
@param      addrlen: 수신된 주소의 길이 (IPv4는 4)
@return     성공 시 수신된 데이터 길이, 실패 시 에러 코드(SOCKERR_)
*/
int32_t recvfrom_W6x00(uint8_t sn, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t *port, uint8_t *addrlen) {
    int32_t ret_val = 0;
    uint8_t mr;
    uint8_t head[8];
    uint16_t pack_len = 0;

    if (sn >= _WIZCHIP_SOCK_NUM_) {
        ret_val = SOCKERR_SOCKNUM;
    } else if (len == 0) {
        ret_val = SOCKERR_DATALEN;
    } else {
        mr = getSn_MR(sn);
        switch (mr & 0x0F) {
        case Sn_MR_UDP:
            break;
        default:
            ret_val = SOCKERR_SOCKMODE;
            break;
        }

        if (ret_val == 0) {
            if (sock_remained_size[sn] == 0) {
                while (1) {
                    pack_len = getSn_RX_RSR(sn);
                    if (getSn_SR(sn) == SOCK_CLOSED) {
                        ret_val = SOCKERR_SOCKCLOSED;
                        break;
                    }
                    if ((sock_io_mode & (1 << sn)) && (pack_len == 0)) {
                        ret_val = SOCK_BUSY;
                        break;
                    }
                    if (pack_len != 0) {
                        break;
                    }
                }
            }
        }

        if (ret_val == 0) {
            switch (mr & 0x07) {
            case Sn_MR_UDP:
                if (sock_remained_size[sn] == 0) {
                    wiz_recv_data(sn, head, 8);
                    setSn_CR(sn, Sn_CR_RECV);
                    while (getSn_CR(sn));
                    
                    addr[0] = head[0];
                    addr[1] = head[1];
                    addr[2] = head[2];
                    addr[3] = head[3];
                    *port = head[4];
                    *port = (*port << 8) + head[5];
                    sock_remained_size[sn] = head[6];
                    sock_remained_size[sn] = (sock_remained_size[sn] << 8) + head[7];
                    sock_pack_info[sn] = PACK_FIRST;
                }
                pack_len = (len < sock_remained_size[sn]) ? len : sock_remained_size[sn];
                wiz_recv_data(sn, buf, pack_len);
                break;
            default:
                wiz_recv_ignore(sn, pack_len);
                sock_remained_size[sn] = pack_len;
                break;
            }

            if (ret_val == 0) {
                setSn_CR(sn, Sn_CR_RECV);
                while (getSn_CR(sn));
                
                sock_remained_size[sn] -= pack_len;
                if (sock_remained_size[sn] != 0) {
                    sock_pack_info[sn] |= PACK_REMAINED;
                } else {
                    sock_pack_info[sn] = PACK_COMPLETED;
                }
                ret_val = (int32_t)pack_len;
            }
        }
    }
    return ret_val;
}

/*
@function    int8_t ctlsocket(uint8_t sn, ctlsock_type cstype, void* arg)
@brief      소켓의 네트워크 속성 제어 및 상태 조회를 수행하는 다목적 제어 함수입니다.
@param      sn: 제어할 소켓 번호
@param      cstype: 수행할 명령 타입 (예: CS_SET_IOMODE, CS_GET_MAXTXBUF 등)
@param      arg: 명령에 필요한 인수 포인터
@return     성공 시 SOCK_OK, 실패 시 에러 코드(SOCKERR_)
*/
int8_t ctlsocket(uint8_t sn, ctlsock_type cstype, void* arg) {
    int8_t ret_val = SOCK_OK;
    uint8_t tmp = 0;

    if (sn >= _WIZCHIP_SOCK_NUM_) {
        ret_val = SOCKERR_SOCKNUM;
    } else {
        if (arg != NULL) tmp = *((uint8_t*)arg);
        
        switch (cstype) {
        case CS_SET_IOMODE:
            if (tmp == SOCK_IO_NONBLOCK) {
                sock_io_mode |= (1 << sn);
            } else if (tmp == SOCK_IO_BLOCK) {
                sock_io_mode &= ~(1 << sn);
            } else {
                ret_val = SOCKERR_ARG;
            }
            break;
        case CS_GET_IOMODE:
            if (arg != NULL) *((uint8_t*)arg) = (uint8_t)((sock_io_mode >> sn) & 0x0001);
            break;
        case CS_GET_MAXTXBUF:
            if (arg != NULL) *((uint16_t*)arg) = getSn_TxMAX(sn);
            break;
        case CS_GET_MAXRXBUF:
            if (arg != NULL) *((uint16_t*)arg) = getSn_RxMAX(sn);
            break;
        case CS_CLR_INTERRUPT:
            if (tmp > SIK_ALL) {
                ret_val = SOCKERR_ARG;
            } else {
                setSn_IR(sn, tmp);
            }
            break;
        case CS_GET_INTERRUPT:
            if (arg != NULL) *((uint8_t*)arg) = getSn_IR(sn);
            break;
        case CS_SET_INTMASK:
            if (tmp > SIK_ALL) {
                ret_val = SOCKERR_ARG;
            } else {
                setSn_IMR(sn, tmp);
            }
            break;
        case CS_GET_INTMASK:
            if (arg != NULL) *((uint8_t*)arg) = getSn_IMR(sn);
            break;
        default:
            ret_val = SOCKERR_ARG;
            break;
        }
    }
    return ret_val;
}

/*
@function    int8_t setsockopt(uint8_t sn, sockopt_type sotype, void* arg)
@brief      지정한 소켓에 대해 세부 옵션(TTL, TOS, 목적지 IP/포트 등)을 설정합니다.
@param      sn: 설정할 소켓 번호
@param      sotype: 설정할 옵션 타입 (예: SO_TTL, SO_DESTIP 등)
@param      arg: 설정할 값이 담긴 포인터
@return     성공 시 SOCK_OK, 실패 시 에러 코드(SOCKERR_)
*/
int8_t setsockopt(uint8_t sn, sockopt_type sotype, void* arg) {
    int8_t ret_val = SOCK_OK;

    if (sn >= _WIZCHIP_SOCK_NUM_) {
        ret_val = SOCKERR_SOCKNUM;
    } else {
        if (arg != NULL) {
            switch (sotype) {
            case SO_TTL:
                setSn_TTL(sn, *(uint8_t*)arg);
                break;
            case SO_TOS:
                setSn_TOS(sn, *(uint8_t*)arg);
                break;
            
            case SO_DESTIP:
                setSn_DIPR(sn, (uint8_t*)arg);
                break;
            case SO_DESTPORT:
                setSn_DPORTR(sn, *(uint16_t*)arg);
                break;
            default:
                ret_val = SOCKERR_ARG;
                break;
            }
        } else {
            ret_val = SOCKERR_ARG;
        }
    }
    return ret_val;
}

/*
@function    int8_t getsockopt(uint8_t sn, sockopt_type sotype, void* arg)
@brief      지정한 소켓의 세부 옵션 상태나 설정된 값을 읽어옵니다.
@param      sn: 조회할 소켓 번호
@param      sotype: 조회할 옵션 타입
@param      arg: 읽어온 값을 저장할 포인터
@return     성공 시 SOCK_OK, 실패 시 에러 코드(SOCKERR_)
*/
int8_t getsockopt(uint8_t sn, sockopt_type sotype, void* arg) {
    int8_t ret_val = SOCK_OK;

    if (sn >= _WIZCHIP_SOCK_NUM_) {
        ret_val = SOCKERR_SOCKNUM;
    } else {
        if (arg != NULL) {
            switch (sotype) {
            case SO_FLAG:
                *(uint8_t*)arg = getSn_MR(sn) & 0xF0;
                break;
            case SO_TTL:
                *(uint8_t*) arg = getSn_TTL(sn);
                break;
            case SO_TOS:
                *(uint8_t*) arg = getSn_TOS(sn);
                break;
            
            case SO_DESTIP:
                getSn_DIPR(sn, (uint8_t*)arg);
                break;
            case SO_DESTPORT:
                *(uint16_t*) arg = getSn_DPORTR(sn);
                break;
            case SO_SENDBUF:
                *(uint16_t*) arg = getSn_TX_FSR(sn);
                break;
            case SO_RECVBUF:
                *(uint16_t*) arg = getSn_RX_RSR(sn);
                break;
            case SO_STATUS:
                *(uint8_t*) arg = getSn_SR(sn);
                break;
            case SO_REMAINSIZE:
                *(uint16_t*)arg = sock_remained_size[sn];
                break;
            case SO_PACKINFO:
                *(uint8_t*)arg = sock_pack_info[sn];
                break;
            default:
                ret_val = SOCKERR_SOCKOPT;
                break;
            }
        } else {
            ret_val = SOCKERR_ARG;
        }
    }
    return ret_val;
}
