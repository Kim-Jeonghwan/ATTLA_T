/**********************************************************************
 Nexcom Co., Ltd.
 Filename         : socket.c
 Version          : 00.01
 Description      : WIZnet 이더넷 라이브러리 파일
 Programmer       : Kim Jeonghwan
 Last Updated     : 2026. 06. 15. (정적시험용 코드 다이어트: 미사용 기능 삭제)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 15. - 정적시험 통과를 위한 타기종 및 미사용 TCP/IPv6 기능 전면 삭제
 */
//*****************************************************************************
#pragma diag_suppress 69 // integer conversion resulted in a change of sign
//
//! \file socket.c
//! \brief SOCKET APIs Implements file.
//! \details SOCKET APIs like as Berkeley Socket APIs.
//! \version 1.0.3
//! \date 2013/10/21
//! \author MidnightCow
//! \copyright
//!
//! Copyright (c)  2013, WIZnet Co., LTD.
//! All rights reserved.
//!
//! Redistribution and use in source and binary forms, with or without
//! modification, are permitted provided that the following conditions
//! are met:
//!
//!     * Redistributions of source code must retain the above copyright
//! notice, this list of conditions and the following disclaimer.
//!     * Redistributions in binary form must reproduce the above copyright
//! notice, this list of conditions and the following disclaimer in the
//! documentation and/or other materials provided with the distribution.
//!     * Neither the name of the <ORGANIZATION> nor the names of its
//! contributors may be used to endorse or promote products derived
//! from this software without specific prior written permission.
//!
//! THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//! AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//! IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//! ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
//! LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//! CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//! SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//! INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//! CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//! ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
//! THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************
#include "socket.h"

//#define SOCK_ANY_PORT_NUM  0xC000;
#define SOCK_ANY_PORT_NUM  0xC000

static uint16_t sock_any_port = SOCK_ANY_PORT_NUM;
static uint16_t sock_io_mode = 0;
static uint16_t sock_is_sending = 0;

static uint16_t sock_remained_size[_WIZCHIP_SOCK_NUM_] = {0, 0,};

//static uint8_t  sock_pack_info[_WIZCHIP_SOCK_NUM_] = {0,};
uint8_t  sock_pack_info[_WIZCHIP_SOCK_NUM_] = {0,};
//

#define CHECK_SOCKNUM()   \
   do{                    \
      if(sn >= _WIZCHIP_SOCK_NUM_) return SOCKERR_SOCKNUM;   \
   }while(0);             \

#define CHECK_SOCKMODE(mode)  \
   do{                     \
      if((getSn_MR(sn) & 0x0F) != mode) return SOCKERR_SOCKMODE;  \
   }while(0);              \

#define CHECK_TCPMODE()                                           \
   do{                                                            \
      if((getSn_MR(sn) & 0x03) != 0x01) return SOCKERR_SOCKMODE;  \
   }while(0);

#define CHECK_SOCKINIT()   \
   do{                     \
      if((getSn_SR(sn) != SOCK_INIT)) return SOCKERR_SOCKINIT; \
   }while(0);              \

#define CHECK_SOCKDATA()   \
   do{                     \
      if(len == 0) return SOCKERR_DATALEN;   \
   }while(0);              \
//teddy 240122
#define CHECK_TCPMODE()                                           \
   do{                                                            \
      if((getSn_MR(sn) & 0x03) != 0x01) return SOCKERR_SOCKMODE;  \
   }while(0);

#define CHECK_UDPMODE()                                           \
   do{                                                            \
      if((getSn_MR(sn) & 0x03) != 0x02) return SOCKERR_SOCKMODE;  \
   }while(0);

#define CHECK_IPMODE()                                            \
   do{                                                            \
      if((getSn_MR(sn) & 0x07) != 0x03) return SOCKERR_SOCKMODE;  \
   }while(0);

#define CHECK_DGRAMMODE()                                         \
   do{                                                            \
      if(getSn_MR(sn) == Sn_MR_CLOSED) return SOCKERR_SOCKMODE;   \
      if((getSn_MR(sn) & 0x03) == 0x01) return SOCKERR_SOCKMODE;  \
   }while(0);

#define CHECK_IPZERO(addr, addrlen)                                  \
   do{                                                               \
      uint16_t ipzero= 0;                                            \
      uint8_t i;                                                     \
      for(i=0; i<addrlen; i++)  ipzero += (uint16_t)addr[i];         \
      if (ipzero == 0) return SOCKERR_IPINVALID;                     \
   }while(0);

#define IPV6_AVAILABLE

#if 1

#define Sn_MR_TCP4           (Sn_MR_TCP)   ///< Refer to @ref Sn_MR_TCP.
#define Sn_MR_UDP4           (Sn_MR_UDP)   ///< Refer to @ref Sn_MR_UDP
#define Sn_MR_IPRAW4         (Sn_MR_IPRAW)   ///< Refer to @ref Sn_MR_IPRAW.   
#define Sn_MR_TCP6           (0x09)
#define Sn_MR_UDP6           (0x0A) //0x1010
#define Sn_MR_IPRAW6         (0x0B) //0x1011
#define Sn_MR_TCPD           (0x0D)
#define Sn_MR_UDPD           (0x0E)

#endif

#if 0 // By lihan  
static uint8_t addrlenTEST = -1 ;

void setAddrlen_W6x00(uint8_t num) {
    addrlenTEST = num;
}

uint8_t  checkAddrlen_W6x00() {
    //if (addrlenTEST < 0 )
    if ((addrlenTEST != 4)  && (addrlenTEST != 16)) {
        perror("Error: addrlen is not initialized");
    } else {
        printf("addrlenTEST %d \r\n", addrlenTEST) ;
    }
    return addrlenTEST;
}

inline void inline_setAddrlen_W6x00(uint8_t num) {
    setAddrlen_W6x00(num);
}

inline uint8_t inline_CheckAddrlen_W6x00(void) {
    return  checkAddrlen_W6x00();
}
#endif

int8_t socket(uint8_t sn, uint8_t protocol, uint16_t port, uint8_t flag) {

    uint8_t taddr[16];
    uint16_t local_port = 0;
    CHECK_SOCKNUM();
    switch (protocol & 0x0F) {
    case Sn_MR_TCP : {
        /*
            uint8_t taddr[4];
            getSIPR(taddr);
        */
        uint32_t taddr;
        getSIPR((uint8_t*)&taddr);
        if (taddr == 0) {
            return SOCKERR_SOCKINIT;
        }
        break;
    }
    case Sn_MR_UDP :
    case Sn_MR_UDP6 :
    case Sn_MR_UDPD :
    case Sn_MR_MACRAW :
    case Sn_MR_IPRAW4 :
    case Sn_MR_IPRAW6 :
        break;
    default :
        return SOCKERR_SOCKMODE;
    }
    //if((flag & 0x06) != 0) return SOCKERR_SOCKFLAG;
    if ((flag & 0x04) != 0) {
        return SOCKERR_SOCKFLAG;
    }

    if (flag != 0) {
        switch (protocol) {

        case Sn_MR_TCP:
            if ((flag & (SF_TCP_NODELAY | SF_IO_NONBLOCK)) == 0) {
                return SOCKERR_SOCKFLAG;
            }

            break;
        case Sn_MR_UDP:
            if (flag & SF_IGMP_VER2) {
                if ((flag & SF_MULTI_ENABLE) == 0) {
                    return SOCKERR_SOCKFLAG;
                }
            }
            break;

        default:
            break;
        }
    }
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
    //
    sock_io_mode |= ((flag & SF_IO_NONBLOCK) << sn);
    sock_is_sending &= ~(1 << sn);
    sock_remained_size[sn] = 0;
    //sock_pack_info[sn] = 0;
    sock_pack_info[sn] = PACK_COMPLETED;//PACK_COMPLETED //TODO::need verify:LINAN 20250421
    //
    while (getSn_SR(sn) == SOCK_CLOSED);
    return (int8_t)sn;
}

int8_t close(uint8_t sn) {
    CHECK_SOCKNUM();
    setSn_CR(sn, Sn_CR_CLOSE);
    /* wait to process the command... */
    while (getSn_CR(sn));
    /* clear all interrupt of SOCKETn. */
    setSn_IR(sn, 0xFF);
    sock_io_mode &= ~(1 << sn);
    //
    sock_is_sending &= ~(1 << sn);
    sock_remained_size[sn] = 0;
    sock_pack_info[sn] = PACK_NONE;
    while (getSn_SR(sn) != SOCK_CLOSED);
    return SOCK_OK;
}

//int8_t connect (uint8_t sn, uint8_t * addr, uint16_t port )

#if 1

#else //for speed optimization, by lihan

#endif

int32_t sendto_W6x00(uint8_t sn, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t port, uint8_t addrlen) {
    // printf("sendto_W6x00\r\n" ) ;
    //static int32_t sendto_IO_6(uint8_t sn, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t port)
    return sendto_IO_6(sn,  buf,  len,   addr,  port, addrlen);
}

static int32_t sendto_IO_6(uint8_t sn, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t port, uint8_t addrlen) {
    uint8_t tmp = 0;
    uint8_t tcmd = Sn_CR_SEND;
    uint16_t freesize = 0;
    uint32_t taddr;

    /*
        The below codes can be omitted for optmization of speed
    */
    CHECK_SOCKNUM();
    //CHECK_DGRAMMODE();
    /************/
    switch (getSn_MR(sn) & 0x0F) {
    case Sn_MR_UDP:
    case Sn_MR_MACRAW:
    //         break;
    //   #if ( _WIZCHIP_ < 5200 )
    case Sn_MR_IPRAW:
    case Sn_MR_IPRAW6:
        break;
    //   #endif
    default:
        return SOCKERR_SOCKMODE;
    }
    tmp = getSn_MR(sn);
    if (tmp != Sn_MR_MACRAW) {
        if (addrlen == 16) {    // addrlen=16, Sn_MR_UDP6(1010), Sn_MR_UDPD(1110)), IPRAW6(1011)
                return SOCKERR_SOCKMODE;
        } else if (addrlen == 4) { // addrlen=4, Sn_MR_UDP4(0010), Sn_MR_UDPD(1110), IPRAW4(0011)
            if (tmp == Sn_MR_UDP6 || tmp == Sn_MR_IPRAW6) {
                return SOCKERR_SOCKMODE;
            }
            setSn_DIPR(sn, addr);
            tcmd = Sn_CR_SEND;
        } else {
            return SOCKERR_IPINVALID;
        }
    }
    if ((tmp & 0x03) == 0x02) { // Sn_MR_UPD4(0010), Sn_MR_UDP6(1010), Sn_MR_UDPD(1110)
        if (port) {
            setSn_DPORTR(sn, port);
        } else {
            return SOCKERR_PORTZERO;
        }
    }
    CHECK_SOCKDATA();
    //if(*((uint32_t*)addr) == 0) return SOCKERR_IPINVALID;
    //{
    //uint32_t taddr;
    taddr = ((uint32_t)addr[0]) & 0x000000FF;
    taddr = (taddr << 8) + ((uint32_t)addr[1] & 0x000000FF);
    taddr = (taddr << 8) + ((uint32_t)addr[2] & 0x000000FF);
    taddr = (taddr << 8) + ((uint32_t)addr[3] & 0x000000FF);
    //}
    //
    //if(*((uint32_t*)addr) == 0) return SOCKERR_IPINVALID;
    if ((taddr == 0) && ((getSn_MR(sn)&Sn_MR_MACRAW) != Sn_MR_MACRAW)) {
        return SOCKERR_IPINVALID;
    }
    if ((port  == 0) && ((getSn_MR(sn)&Sn_MR_MACRAW) != Sn_MR_MACRAW)) {
        return SOCKERR_PORTZERO;
    }
    tmp = getSn_SR(sn);
    //#if ( _WIZCHIP_ < 5200 )
    if ((tmp != SOCK_MACRAW) && (tmp != SOCK_UDP) && (tmp != SOCK_IPRAW)) {
        return SOCKERR_SOCKSTATUS;
    }
    //#else
    //   if(tmp != SOCK_MACRAW && tmp != SOCK_UDP) return SOCKERR_SOCKSTATUS;
    //#endif

    setSn_DIPR(sn, addr);
    setSn_DPORT(sn, port);

    freesize = getSn_TxMAX(sn);
    if (len > freesize) {
        len = freesize;    // check size not to exceed MAX size.
    }

    while (1) {
        freesize = getSn_TX_FSR(sn);
        if (getSn_SR(sn) == SOCK_CLOSED) {
            return SOCKERR_SOCKCLOSED;
        }
        if ((sock_io_mode & (1 << sn)) && (len > freesize)) {
            return SOCK_BUSY;
        }
        if (len <= freesize) {
            break;
        }
    };
    wiz_send_data(sn, buf, len);

    //
    setSn_CR(sn, Sn_CR_SEND);
    /* wait to process the command... */
    while (getSn_CR(sn));
    while (1) {
        tmp = getSn_IR(sn);
        if (tmp & Sn_IR_SENDOK) {
            setSn_IR(sn, Sn_IR_SENDOK);
            break;
        }
        //M:20131104
        //else if(tmp & Sn_IR_TIMEOUT) return SOCKERR_TIMEOUT;
        else if (tmp & Sn_IR_TIMEOUT) {
            setSn_IR(sn, Sn_IR_TIMEOUT);
            //len = (uint16_t)SOCKERR_TIMEOUT;
            //break;
            return SOCKERR_TIMEOUT;
        }
        ////////////
    }
    //return len;
    return (int32_t)len;
}

int32_t recvfrom_W6x00(uint8_t sn, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t *port, uint8_t *addrlen) {
    // printf("recvfrom_W6x00\r\n" ) ;
    //int32_t recvfrom_IO_6(uint8_t sn, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t *port)
    return recvfrom_IO_6(sn,  buf,  len,   addr,  port, addrlen);
}
static int32_t recvfrom_IO_6(uint8_t sn, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t *port, uint8_t *addrlen) { //TODO : WILL BE IMPROVED
    uint8_t  mr;
    //
    uint8_t  head[8];
    uint16_t pack_len = 0;

    /*
        The below codes can be omitted for optmization of speed
    */
    CHECK_SOCKNUM();
    //CHECK_DGRAMMODE();
    //CHECK_SOCKDATA();
    /************/
    //CHECK_SOCKMODE(Sn_MR_UDP);

    switch ((mr = getSn_MR(sn)) & 0x0F) {
    case Sn_MR_UDP:
    case Sn_MR_IPRAW:
    case Sn_MR_IPRAW6:
    case Sn_MR_MACRAW:
        break;
    default:
        return SOCKERR_SOCKMODE;
    }
    CHECK_SOCKDATA();
    if (sock_remained_size[sn] == 0) {
        while (1) {
            pack_len = getSn_RX_RSR(sn);
            if (getSn_SR(sn) == SOCK_CLOSED) {
                return SOCKERR_SOCKCLOSED;
            }
            if ((sock_io_mode & (1 << sn)) && (pack_len == 0)) {
                return SOCK_BUSY;
            }
            if (pack_len != 0) {
                break;
            }
        };
    }
    //D20150601 : Move it to bottom
    // sock_pack_info[sn] = PACK_COMPLETED;
    switch (mr & 0x07) {
    case Sn_MR_UDP4 :
    case Sn_MR_UDP6:
    case Sn_MR_UDPD:
        if (sock_remained_size[sn] == 0) {
            wiz_recv_data(sn, head, 8);
            setSn_CR(sn, Sn_CR_RECV);
            while (getSn_CR(sn));
            // read peer's IP address, port number & packet length
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
        if (len < sock_remained_size[sn]) {
            pack_len = len;
        } else {
            pack_len = sock_remained_size[sn];
        }
        len = pack_len;
        //
        // Need to packet length check (default 1472)
        //
        wiz_recv_data(sn, buf, pack_len); // data copy.
        break;
    case Sn_MR_MACRAW :
        if (sock_remained_size[sn] == 0) {
            wiz_recv_data(sn, head, 2);
            setSn_CR(sn, Sn_CR_RECV);
            while (getSn_CR(sn));
            // read peer's IP address, port number & packet length
            sock_remained_size[sn] = head[0];
            sock_remained_size[sn] = (sock_remained_size[sn] << 8) + head[1] - 2;
            if (sock_remained_size[sn] > 1514) {
                close(sn);
                return SOCKFATAL_PACKLEN;
            }
            sock_pack_info[sn] = PACK_FIRST;
        }
        if (len < sock_remained_size[sn]) {
            pack_len = len;
        } else {
            pack_len = sock_remained_size[sn];
        }
        wiz_recv_data(sn, buf, pack_len);
        break;
    //#if ( _WIZCHIP_ < 5200 )
    case Sn_MR_IPRAW6:
    case Sn_MR_IPRAW4 :
        if (sock_remained_size[sn] == 0) {
            wiz_recv_data(sn, head, 6);
            setSn_CR(sn, Sn_CR_RECV);
            while (getSn_CR(sn));
            addr[0] = head[0];
            addr[1] = head[1];
            addr[2] = head[2];
            addr[3] = head[3];
            sock_remained_size[sn] = head[4];
            //sock_remaiend_size[sn] = (sock_remained_size[sn] << 8) + head[5];
            sock_remained_size[sn] = (sock_remained_size[sn] << 8) + head[5];
            sock_pack_info[sn] = PACK_FIRST;
            //
            // Need to packet length check
            //
            if (len < sock_remained_size[sn]) {
                pack_len = len;
            } else {
                pack_len = sock_remained_size[sn];
            }
            wiz_recv_data(sn, buf, pack_len); // data copy.
        }
        break;
    default:
        wiz_recv_ignore(sn, pack_len); // data copy.
        sock_remained_size[sn] = pack_len;
        break;
    }
    setSn_CR(sn, Sn_CR_RECV);
    /* wait to process the command... */
    while (getSn_CR(sn)) ;
    sock_remained_size[sn] -= pack_len;
    //if(sock_remained_size[sn] != 0) sock_pack_info[sn] |= 0x01;
    if (sock_remained_size[sn] != 0) {
        sock_pack_info[sn] |= PACK_REMAINED;
    } else {
        sock_pack_info[sn] = PACK_COMPLETED;
    }
    //
    //return pack_len;
    return (int32_t)pack_len;
}

int8_t  ctlsocket(uint8_t sn, ctlsock_type cstype, void* arg) {
    uint8_t tmp = 0;
    CHECK_SOCKNUM();
    tmp = *((uint8_t*)arg);
    switch (cstype) {
    case CS_SET_IOMODE:
        if (tmp == SOCK_IO_NONBLOCK) {
            sock_io_mode |= (1 << sn);
        } else if (tmp == SOCK_IO_BLOCK) {
            sock_io_mode &= ~(1 << sn);
        } else {
            return SOCKERR_ARG;
        }
        break;
    case CS_GET_IOMODE:
        //*((uint8_t*)arg) = (sock_io_mode >> sn) & 0x0001;
        *((uint8_t*)arg) = (uint8_t)((sock_io_mode >> sn) & 0x0001);
        //
        break;
    case CS_GET_MAXTXBUF:
        *((uint16_t*)arg) = getSn_TxMAX(sn);
        break;
    case CS_GET_MAXRXBUF:
        *((uint16_t*)arg) = getSn_RxMAX(sn);
        break;
    case CS_CLR_INTERRUPT:
        if (tmp > SIK_ALL) {
            return SOCKERR_ARG;
        }
        setSn_IR(sn, tmp);
        break;
    case CS_GET_INTERRUPT:
        *((uint8_t*)arg) = getSn_IR(sn);
        break;
    case CS_SET_INTMASK:
        if (tmp > SIK_ALL) {
            return SOCKERR_ARG;
        }
        setSn_IMR(sn, tmp);
        break;
    case CS_GET_INTMASK:
        *((uint8_t*)arg) = getSn_IMR(sn);
        break;
    default:
        return SOCKERR_ARG;
    }
    return SOCK_OK;
}

int8_t  setsockopt(uint8_t sn, sockopt_type sotype, void* arg) {
    //uint8_t tmp;
    CHECK_SOCKNUM();
    switch (sotype) {
    case SO_TTL:
        setSn_TTL(sn, *(uint8_t*)arg);
        break;
    case SO_TOS:
        setSn_TOS(sn, *(uint8_t*)arg);
        break;
    case SO_MSS:
        setSn_MSSR(sn, *(uint16_t*)arg);
        break;
    case SO_DESTIP:
            setSn_DIPR(sn, (uint8_t*)arg);
        break;
    case SO_DESTPORT:
        setSn_DPORTR(sn, *(uint16_t*)arg);
        break;
    case SO_KEEPALIVESEND:
        CHECK_TCPMODE();
        if (getSn_KPALVTR(sn) != 0) {
            return SOCKERR_SOCKOPT;
        }
        setSn_CR(sn, Sn_CR_SEND_KEEP);
        while (getSn_CR(sn) != 0) {
            //if ((tmp = getSn_IR(sn)) & Sn_IR_TIMEOUT)
            if (getSn_IR(sn) & Sn_IR_TIMEOUT) {
                setSn_IR(sn, Sn_IR_TIMEOUT);
                return SOCKERR_TIMEOUT;
            }
        }
        break;
    case SO_KEEPALIVEAUTO:
        CHECK_TCPMODE();
        setSn_KPALVTR(sn, *(uint8_t*)arg);
        break;
    default:
        return SOCKERR_ARG;
    }
    return SOCK_OK;
}

int8_t getsockopt(uint8_t sn, sockopt_type sotype, void* arg) {
    CHECK_SOCKNUM();
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
    case SO_MSS:
        *(uint16_t*) arg = getSn_MSSR(sn);
        break;
    case SO_DESTIP:
        getSn_DIPR(sn, (uint8_t*)arg);
        break;
    case SO_DESTPORT:
        *(uint16_t*) arg = getSn_DPORTR(sn);
        break;
    case SO_KEEPALIVEAUTO:
        CHECK_TCPMODE();
        *(uint16_t*) arg = getSn_KPALVTR(sn);
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
        if (getSn_MR(sn) & Sn_MR_TCP) {
            *(uint16_t*)arg = getSn_RX_RSR(sn);
        } else {
            *(uint16_t*)arg = sock_remained_size[sn];
        }
        break;
    case SO_PACKINFO  :
        //CHECK_SOCKMODE(Sn_MR_TCP);
        if ((getSn_MR(sn) == Sn_MR_TCP)) {
            return SOCKERR_SOCKMODE;
        }
        *(uint8_t*)arg = sock_pack_info[sn];
        break;

    default:
        return SOCKERR_SOCKOPT;
    }
    return SOCK_OK;
}

