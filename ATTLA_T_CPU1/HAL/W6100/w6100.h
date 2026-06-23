/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : w6100.h
    Version          : 00.05
    Description      : WIZnet 이더넷 라이브러리 파일
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 16. (중복된 #endif 제거하여 컴파일 오류 수정)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 16. - 중복된 #endif 제거하여 컴파일 오류 수정
 * 2026. 06. 16. - #define 매크로 관련 영문 주석을 한국어로 번역
 * 2026. 06. 16. - 주석 없이 중복 선언되어 있던 함수(getSn_TX_FSR, getSn_RX_RSR)
 * 삭제 2026. 06. 16. - 영문 주석 한국어 번역 및 주요 I/O API 함수에 CSU/HAL
 * 주석 표준 포맷(@function 등) 적용 2026. 06. 15. - 정적시험 통과를 위한 타기종
 * 및 미사용 TCP/IPv6 기능 전면 삭제
 */

#ifndef _W6100_H_
#define _W6100_H_

#include "wizchip_conf.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/// @cond DOXY_APPLY_CODE
/// @endcond

#define _W6100_SPI_READ_                                                       \
  (0x00 << 2) ///< 제어(Control) 단계에서 SPI 인터페이스 읽기 동작
#define _W6100_SPI_WRITE_                                                      \
  (0x01 << 2) ///< 제어(Control) 단계에서 SPI 인터페이스 쓰기 동작

#define WIZCHIP_CREG_BLOCK (0x00 << 3)           ///< 공통 레지스터 블록
#define WIZCHIP_SREG_BLOCK(N) ((1 + 4 * N) << 3) ///< SOCKETn 레지스터 블록
#define WIZCHIP_TXBUF_BLOCK(N)                                                 \
  ((2 + 4 * N) << 3) ///< SOCKETn 송신(Tx) 버퍼 주소 블록
#define WIZCHIP_RXBUF_BLOCK(N)                                                 \
  ((3 + 4 * N) << 3) ///< SOCKETn 수신(Rx) 버퍼 주소 블록

#define WIZCHIP_OFFSET_INC(ADDR, N) (ADDR + (N << 8)) ///< 오프셋 주소 증가

#if (_WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_BUS_INDIR_)
#define IDM_AR0                                                                \
  ((_WIZCHIP_IO_BASE_ + 0x0000)) ///< 간접(Indirect) 상위 주소 레지스터
#define IDM_AR1                                                                \
  ((_WIZCHIP_IO_BASE_ + 0x0001)) ///< 간접(Indirect) 하위 주소 레지스터
#define IDM_BSR ((_WIZCHIP_IO_BASE_ + 0x0002)) ///< 블록 선택(Select) 레지스터
#define IDM_DR                                                                 \
  ((_WIZCHIP_IO_BASE_ + 0x0003)) ///< 간접(Indirect) 데이터 레지스터
#define _W6100_IO_BASE_ _WIZCHIP_IO_BASE_
#elif (_WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_SPI_)
#define _W6100_IO_BASE_ 0x00000000
#endif

//----------------------------- W6100 Common Registers IOMAP
//-----------------------------

/**
    @brief 칩 식별(Identification) 레지스터 주소 [RO] [0x6100]
    @sa getCIDR()
*/
#define _CIDR_ (_W6100_IO_BASE_ + (0x0000 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief 칩 버전 레지스터 주소 [RO] [0x4661]
    @sa getVER()
*/
#define _VER_ (_W6100_IO_BASE_ + (0x0002 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief 시스템 상태(Status) 레지스터 주소 [RO] [0xEU]
    @details @ref _SYSR_은 CHIP, NET, PHY 잠금 및 호스트 인터페이스와 같은
   정보를 보여줍니다. <table> <tr> <td>7   </td> <td>6   </td> <td>5   </td>
   <td>4 ~ 2   </td> <td>1  </td> <td>0  </td> </tr> <tr> <td>CHPL</td>
   <td>NETL</td> <td>PHYL</td> <td>Reserved</td> <td>IND</td> <td>SPI</td> </tr>
    </table>
    - @ref SYSR_CHPL
    - @ref SYSR_NETL
    - @ref SYSR_PHYL
    - @ref SYSR_IND  : HOST use Parallel BUS Interface(Indirect Bus Mode)
    - @ref SYSR_SPI  : HOST use SPI Interface

    @sa _CHPLCKR_, _NETLCKR_, _PHYLCKR_,
    @sa getSYSR(), setCHPLCKR(), getCHPLCKR(), CHIPLOCK(), CHIPUNLOCK(),
   setNETLCKR(), getNETLCKR(), NETLOCK(), NETUNLOCK() \n setPHYLCKR(),
   getPHYLCKR(), PHYLOCK(), PHYUNLOCK()
*/
#define _SYSR_ (_W6100_IO_BASE_ + (0x2000 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief 시스템 설정(Config) 레지스터 0 주소 [WO][0x80]
    @details @ref _SYCR0_은 _WIZCHIP_을 소프트하게 리셋합니다.
    <table>
      <tr> <td>7  </td> <td>6 ~ 0   </td> </tr>
      <tr> <td>RST</td> <td>Reserved</td> </tr>
    </table>
     - @ref SYCR0_RST : Software Reset.

    @note It can be accessed only when @ref SYSR_CHPL = 1.
    @sa _CHPLCKR_, _SYSR_, SYSR_CHPL
    @sa setSYCR0(), setCHPLCKR(), getCHPLCKR(), CHIPLOCK(), CHIPUNLOCK(),
   getSYSR()
*/
#define _SYCR0_ (_W6100_IO_BASE_ + (0x2004 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief 시스템 설정(Config) 레지스터 1 주소 [R=W][0x80]
    @details @ref _SYCR1_은 전역 인터럽트 활성화를 제어하고 시스템 클럭을
   선택합니다. <table> <tr> <td>7  </td> <td>6 ~ 1   </td> <td>0     </td> </tr>
      <tr> <td>IEN</td> <td>Reserved</td> <td>CLKSEL</td> </tr>
    </table>
     - @ref SYCR1_IEN
     - @ref SYCR1_CLKSEL

    @note SYCR1_CLKSEL bit can be accessed only when @ref SYSR_CHPL = 1.
    @sa _CHPLCKR_, _SYSR_, SYSR_CHPL
    @sa getSYCR1(), setSYCR1(), setCHPLCKR(), getCHPLCKR(), CHIPLOCK(),
   CHIPUNLOCK(), getSYSR()
*/
#define _SYCR1_ (WIZCHIP_OFFSET_INC(_SYCR0_, 1))

/**
    @brief 티커(Ticker) 카운터 레지스터 주소 [RO][0x0000]
    @details @ref _TCNTR_은 _WIZCHIP_ 리셋 후 100us마다 1씩 증가합니다.
    @sa _TCNTRCLR_
    @sa getTCNTR(), setTCNTRCLR()
*/
#define _TCNTR_ (_W6100_IO_BASE_ + (0x2016 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief 티커(Ticker) 카운터 클리어 레지스터 주소 [RO][0x00]
    @details @ref _TCNTRCLR_은 @ref _TCNTR_을 초기화(clear)합니다.
    @sa setTCNTRCLR(), getTCNTR()
*/
#define _TCNTRCLR_ (_W6100_IO_BASE_ + (0x2020 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief 인터럽트 레지스터 주소 [RO][0x00]
    @details @ref _IR_은 인터럽트 상태를 나타냅니다.
            If @ref _IR_ is not equal to x00 INTn PIN is asserted to low until
   it is x00.

    <table>
      <tr> <td>7  </td> <td>6 ~ 5   </td> <td>4   </td> <td>3       </td> <td>2
   </td> <td>1   </td> <td>0    </td> </tr> <tr> <td>WOL</td> <td>Reserved</td>
   <td>UNR6</td> <td>Reserved</td> <td>IPCONF</td> <td>UNR4</td> <td>PTERM</td>
   </tr>
    </table>
    - @ref IR_WOL    : Wake On LAN
    - @ref IR_UNR6   : Destination Port Unreachable for IPv6
    - @ref IR_IPCONF : @ref _SIPR_ is Conflict
    - @ref IR_UNR4   : Destination Port Unreachable for IPv4
    - @ref IR_PTERM  : PPPoE Terminated

    @sa _IMR_, _IRCLR_, SYCR1_IEN, _CHIPLCKR_, _SYSR_, SYSR_CHPL
    @sa getIR(), setIRCLR(), getSYCR1(), setSYCR1(), getCHPLCKR(), setCHPLCKR(),
   CHIPLOCK(), CHIPUNLOCK(), getSYSR()
*/
#define _IR_ (_W6100_IO_BASE_ + (0x2100 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief SOCKET 인터럽트 레지스터 주소 [RO][0x00]
    @details @ref _SIR_은 소켓 인터럽트 발생 여부를 나타냅니다.\n
            Each bit of @ref _SIR_ be still until @ref _Sn_IR_ is cleared by
   @ref _Sn_IRCLR_
    @sa _SIMR_, _Sn_IR_, _Sn_IRCLR_, _Sn_IMR_, SYCR1_IEN , _CHIPLCKR_, _SYSR_,
   SYSR_CHPL
    @sa getSIR(), getSn_IR(), setSn_IRCLR(), getSIMR(), setSIMR(), getSn_IMR(),
   setSn_IMR(), getSYCR1(), setSYCR1(), \n getSYCR1(), setSYCR1(), getCHPLCKR(),
   setCHPLCKR(), CHIPLOCK(), CHIPUNLOCK(), getSYSR()
*/
#define _SIR_ (_W6100_IO_BASE_ + (0x2101 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief SOCKET-less 인터럽트 레지스터 주소 [RO][0x00]
    @details @ref _SLIR_은 @ref _SLCR_의 완료 또는 시간 초과를 나타냅니다.
    <table>
      <tr> <td>7   </td> <td>6   </td> <td>5    </td> <td>4   </td> <td>3 </td>
   <td>2 </td> <td>1 </td> <td>0 </td> </tr> <tr> <td>TOUT</td> <td>ARP4</td>
   <td>PING4</td> <td>ARP6</td> <td>PING6</td> <td>NS</td> <td>RS</td>
   <td>RA</td> </tr>
    </table>
    - @ref SLIR_TOUT  : The timeout occurrence after @ref _SLCR_ is performed
    - @ref SLIR_ARP4  : The completion of @ref SLCR_ARP4
    - @ref SLIR_PING4 : The completion of @ref SLCR_PING4
    - @ref SLIR_ARP6  : The completion f @ref SLCR_ARP6
    - @ref SLIR_PING6 : The completion of @ref SLCR_PING6
    - @ref SLIR_NS    : The completion of @ref SLCR_NS
    - @ref SLIR_RS    : The completion of @ref SLIR_RS
    - @ref SLIR_RA    : The reception from Router Advertisement

    @sa _SLIRCLR_, _SLIMR_, SYCR1_IEN, _CHPLCKR_, _SYSR_, SYSR_CHPL
    @sa getSLIR(), setSLIRCLR(),  getSLIR(), getSLIMR(), setSLIMR(), \n
       getSYCR1(), setSYCR1(), getCHPLCKR(), setCHPLCKR(), CHIPLOCK(),
   CHIPUNLOCK(), getSYSR()
*/
#define _SLIR_ (_W6100_IO_BASE_ + (0x2102 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief 인터럽트 마스크 레지스터 주소 [R=W][0x00]
    @details @ref _IMR_은 @ref _IR_의 인터럽트를 마스킹하는데 사용됩니다.\n
            When a bit of @ref _IMR_ and the corresponding bit of @ref _IR_ is
   set, an interrupt will be issued.
    @sa _IR_, SYCR1_IEN, _CHPLCKR_, _SYSR_, SYSR_CHPL
    @sa getIMR(), setIMR(),  getIR(), setIRCLR(), \n
       getSYCR1(), setSYCR1(), getCHPLCKR(), setCHPLCKR(), CHIPLOCK(),
   CHIPUNLOCK(), getSYSR()
*/
#define _IMR_ (_W6100_IO_BASE_ + (0x2104 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief @ref _IR_ 클리어 레지스터 주소 [WO][0x00]
    @details @ref _IRCLR_은 @ref _IR_을 클리어합니다.
    @sa _IR_, _IMR_, SYCR1_IEN, _CHPLCKR_, _SYSR_, SYSR_CHPL
    @sa setIRCLR(), getIR(), getIMR(), getSYCR1(), setSYCR1(), getCHPLCKR(),
   setCHPLCKR(), CHIPLOCK(), CHIPUNLOCK(), getSYSR()
*/
#define _IRCLR_ (_W6100_IO_BASE_ + (0x2108 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief SOCKET 인터럽트 마스크 레지스터 주소 [R=W]][0x00]
    @details @ref _SIMR_은 @ref _SIR_의 인터럽트를 마스킹하는 데 사용됩니다.\n
            When a bit of @ref _SIMR_ and the corresponding bit of @ref _SIR_ is
   set, an interrupt will be issued.\n when @ref _Sn_IR_ is not 0,  The N-th bit
   of @ref _SIR_ is set. Otherwise, this bit is automatically clear.\n
    @sa _SIR_, _Sn_IR_, _Sn_IRCLR_, _Sn_IMR_, SYCR1_IEN, _CHPLCKR_, _SYSR_,
   SYSR_CHPL
    @sa getSIMR(), setSIMR(), getSIR(), setSn_IRCLR(), getSn_IMR(), setSn_IMR(),
   \n getSYCR1(), setSYCR1(), getCHPLCKR(), setCHPLCKR(), CHIPLOCK(),
   CHIPUNLOCK(), getSYSR()
*/
#define _SIMR_ (_W6100_IO_BASE_ + (0x2114 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief SOCKET-less 인터럽트 마스크 레지스터 주소 [R=W][0x00]
    @details @ref _SLIMR_은 @ref _SLIR_의 인터럽트를 마스킹하는데 사용됩니다\n
            When a bit of @ref _SLIMR_ and the corresponding bit of @ref _SLIR_
   is set, an interrupt will be issued.
    @sa _SLIR_, _SLIRCLR_, SYCR1_IEN, _CHPLCKR_, _SYSR_, SYSR_CHPL
    @sa getSLIMR(), setSLIMR(), getSLIR(), setSLIRCLR(), \n
       getSYCR1(), setSYCR1(), getCHPLCKR(), setCHPLCKR(), CHIPLOCK(),
   CHIPUNLOCK(), getSYSR()
*/
#define _SLIMR_ (_W6100_IO_BASE_ + (0x2124 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief SOCKET-less 인터럽트 클리어 레지스터 주소 [WO][0x00]
    @details @ref _SLIRCLR_은 @ref _SLIR_을 초기화(clear)합니다
    @sa _SLIR_, _SLIRCLR_, _SLIMR_, SYCR1_IEN, _CHPLCKR_, _SYSR_, SYSR_CHPL
    @sa getSLIR(), setSLIRCLR(), getSLIMR(), setSLIMR(), \n
       getSYCR1(), setSYCR1(), getCHPLCKR(), setCHPLCKR(), CHIPLOCK(),
   CHIPUNLOCK(), getSYSR()
*/
#define _SLIRCLR_ (_W6100_IO_BASE_ + (0x2128 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief SOCKET-less 기본 소스 IPv6 주소 레지스터 주소 [R=W][0x00]
    @details @ref _SLPSR_은 @ref _SLCR_로 패킷을 전송하기 위한 소스 IPv6 주소를
   선택합니다.
     - @ref PSR_AUTO
     - @ref PSR_LLA
     - @ref PSR_GUA
    @sa _SLCR_, _Sn_PSR_
    @sa getSLPSR(), setSLPSR(), getSLCR(), setSLCR(), getSn_PSR(), setSn_PSR()
*/
#define _SLPSR_ (_W6100_IO_BASE_ + (0x212C << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief SOCKET-less 명령어(Command) 레지스터 주소 [RW,AC][0x00]
    @details @ref _SLCR_은 SOCKET 없이 ARP, PING, ICMPv6와 같은 메시지를 요청할
   수 있습니다. <table> <tr> <td>7       </td> <td>6   </td> <td>5    </td>
   <td>4   </td> <td>3    </td> <td>2 </td> <td>1 </td> <td>0  </td> </tr> <tr>
   <td>Reserved</td> <td>ARP4</td> <td>PING4</td> <td>ARP6</td> <td>PING6</td>
   <td>NS</td> <td>RS</td> <td>UNA</td> </tr>
    </table>
     - @ref SLCR_ARP4
     - @ref SLCR_PING4
     - @ref SLCR_ARP6
     - @ref SLCR_PING6
     - @ref SLCR_NS
     - @ref SLCR_RS
     - @ref SLCR_UNA

    @sa _SLIR_, _SLIMR_, _SLDIPR_, _SLDIP4R_, _SLDIP6R_, _SLDHAR_, SYCR1_IEN,
   _CHPLCKR_, _SYSR_, SYSR_CHPL
    @sa getSLCR(), setSLCR(), getSLIR(), setSLIRCLR(), getSLIMR(), setSLIMR(),
   getSLDIPR(),setSLDIPR(), getSLDIP4R(),setSLDIP4R(), getSLDIP6R(),
   setSLDIP6R(), getSLDHAR(), getSYCR1(), setSYCR1(), getCHPLCKR(),
   setCHPLCKR(), CHIPLOCK(), CHIPUNLOCK(), getSYSR()
*/
#define _SLCR_ (_W6100_IO_BASE_ + (0x2130 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief PHY 상태 레지스터 주소 [RO][0x00]
    @details @ref _PHYSR_은 PHY의 작동 모드, 링크 상태 등을 표시합니다.
     - @ref PHYSR_CAB  : @ref PHYSR_CAB_OFF, @ref PHYSR_CAB_ON
     - @ref PHYSR_MODE : @ref PHYSR_MODE_AUTO, @ref PHYSR_MODE_100F, @ref
   PHYSR_MODE_100H, @ref PHYSR_MODE_10F, @ref PHYSR_MODE_10H
     - @ref PHYSR_DPX  : @ref PHYSR_DPX_FULL, @ref PHYSR_DPX_HALF
     - @ref PHYSR_SPD  : @ref PHYSR_SPD_100M, @ref PHYSR_SPD_10M
     - @ref PHYSR_LNK  : @ref PHYSR_LNK_UP, @ref PHYSR_LNK_DOWN

    @sa _PHYCR0_, _PHYLCKR_, _SYSR_, SYSR_PHYL
    @sa getPHYSR(), setPHYCR0(), setPHYLCKR(), getPHYLCKR(), PHYLOCK(),
   PHYUNLOCK(), getSYSR()
*/
#define _PHYSR_ (_W6100_IO_BASE_ + (0x3000 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief PHY 내부 레지스터 주소 지정 레지스터 주소(R/W)
    @details @ref _PHYRAR_은 이더넷 PHY 내부 레지스터의 주소를 지정합니다.
     - @ref PHYRAR_BMCR
     - @ref PHYRAR_BMSR
    @sa _PHYACR_, _PHYDOR_, _PHYDIR_, _PHYDIVR_
    @sa getPHYACR(), setPHYACR(), getPHYDOR(), setPHYDIR(), getPHYDIVR(),
   setPHYDIVR()
*/
#define _PHYRAR_ (_W6100_IO_BASE_ + (0x3008 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief PHY 데이터 입력(Data Input) 레지스터 주소 [R=W][0x00]
    @details @ref _PHYDIR_은 PHY 내 레지스터에 기록할 값을 지정합니다.
    @sa _PHYRAR_, _PHYACR_, _PHYDOR_, _PHYDIVR_
    @sa setPHYDIR(), getPHYRAR(), setPHYRAR(), getPHYACR(), setPHYACR(),
   getPHYDOR(), setPHYDIR(), getPHYDIVR(), setPHYDIVR()
*/
#define _PHYDIR_ (_W6100_IO_BASE_ + (0x300C << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief PHY 데이터 출력(Data Output) 레지스터 주소 [WO][0x00]
    @details @ref _PHYDOR_은 PHY 내 레지스터에서 값을 읽습니다.
    @sa _PHYRAR_, _PHYACR_, _PHYDIR_, _PHYDIVR_
    @sa getPHYDOR(), getPHYRAR(), setPHYRAR(), getPHYACR(), setPHYACR(),
   setPHYDIR(), getPHYDIVR(), setPHYDIVR()
*/
#define _PHYDOR_ (_W6100_IO_BASE_ + (0x3010 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief PHY 접근(Access) 레지스터 주소 [RW,AC][0x00]
    @details @ref _PHYACR_는 이더넷 PHY 내 레지스터의 값을 쓰거나(읽거나)
   합니다.
     - @ref PHYACR_READ
     - @ref PHYACR_WRITE
    @sa _PHYRAR_, _PHYDOR_, _PHYDIR_, _PHYDIVR_
    @sa getPHYACR(), setPHYACR(), getPHYDOR(), getPHYRAR(), setPHYRAR(),
   setPHYDIR(), getPHYDIVR(), setPHYDIVR()
*/
#define _PHYACR_ (_W6100_IO_BASE_ + (0x3014 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief PHY의 MDC 클럭 분주(Division) 레지스터 주소 [R=W][0x01]
    @details @ref _PHYDIVR_은 이더넷 PHY의 MDC 클럭을 위해 시스템 클럭을
   분주합니다'
     - @ref PHYDIVR_32
     - @ref PHYDIVR_64
     - @ref PHYDIVR_128
    @sa _PHYRAR_, _PHYACR_, _PHYDOR_, _PHYDIR_, _PHYDIVR_
    @sa getPHYDIVR(), setPHYDIVR(), getPHYRAR(), setPHYRAR(), getPHYACR(),
   setPHYACR(), getPHYDOR(), setPHYDIR()
*/
#define _PHYDIVR_ (_W6100_IO_BASE_ + (0x3018 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief PHY 제어(Control) 레지스터 주소 [WO][0x00]
    @details @ref _PHYCR0_은 PHY의 작동 모드를 제어합니다.
            The result will be checked by @ref _PHYSR_ after PHY HW reset by
   @ref PHYCR1_RST.
     - @ref PHYCR0_AUTO
     - @ref PHYCR0_100F
     - @ref PHYCR0_100H
     - @ref PHYCR0_10F
     - @ref PHYCR0_10H

    @note It can be only accessed when @ref SYSR_PHYL is unlock.
    @sa _SYSR_, _PHYCR1_
    @sa setPHYCR0(), getSYSR(), getPHYCR1(), setPHYCR1()
*/
#define _PHYCR0_ (_W6100_IO_BASE_ + (0x301C << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief PHY 제어(Control) 레지스터 주소 [R=W][0x40]
    @details @ref _PHYCR1_은 HW 리셋, 전원 차단 등 이더넷 PHY 기능을 제어합니다.
    <table>
      <tr> <td>7       </td> <td>6       </td> <td>5   </td> <td>4      </td>
   <td>3 </td> <td>2 ~ 1   </td> <td>0  </td> </tr> <tr> <td>Reserved</td>
   <td>Always 1</td> <td>PWDN</td> <td>Reseved</td> <td>TE</td>
   <td>Reserved</td> <td>RST</td> </tr>
    </table>
     - @ref PHYCR1_PWDN
     - @ref PHYCR1_TE
     - @ref PHYCR1_RST

    @note It can be only accessed when @ref SYSR_PHYL is unlock.
    @sa _SYSR_, _PHYCR0_
    @sa getPHYCR1(), setPHYCR1(), setPHYCR0(), getSYSR()
*/
#define _PHYCR1_ WIZCHIP_OFFSET_INC(_PHYCR0_, 1)

/**
    @brief 네트워크 IPv4 모드 레지스터 주소 [R=W][0x00]
    @details @ref _NET4MR_는 Unreachable 메시지, TCP 리셋 및 핑 응답과 같은
   전송을 차단할 수 있습니다.\n It can ARP request before ping relpy.

    <table>
      <tr> <td>7 ~ 4   </td> <td>3   </td> <td>2   </td> <td>1   </td> <td>0
   </td> </tr> <tr> <td>Reserved</td> <td>UNRB</td> <td>PARP</td> <td>RSTB</td>
   <td>PB</td> </tr>
    </table>
     - @ref NETxMR_UNRB
     - @ref NETxMR_PARP
     - @ref NETxMR_RSTB
     - @ref NETxMR_PB
    @sa _NET6MR_
    @sa getNET4MR(), setNET4MR(), getNET6MR(), setNET6MR()
*/
#define _NET4MR_ (_W6100_IO_BASE_ + (0x4000 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief 네트워크 IPv6 모드 레지스터 주소 [R=W][0x00]
    @details @ref _NET6MR_는 Unreachable 메시지, TCP 리셋 및 핑 응답과 같은
   전송을 차단할 수 있습니다.\n It can ARP request before ping reply.

    <table>
      <tr> <td>7 ~ 4   </td> <td>3   </td> <td>2   </td> <td>1   </td> <td>0
   </td> </tr> <tr> <td>Reserved</td> <td>UNRB</td> <td>PARP</td> <td>RSTB</td>
   <td>PB</td> </tr>
    </table>
     - @ref NETxMR_UNRB
     - @ref NETxMR_PARP
     - @ref NETxMR_RSTB
     - @ref NETxMR_PB
    @sa _NET4MR_
    @sa getNET6MR(), setNET6MR(), getNET4MR(), setNET4MR()
*/
#define _NET6MR_ (_W6100_IO_BASE_ + (0x4004 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief 네트워크 모드 레지스터 주소 [R=W][0x00]
    @details @ref _NETMR_는 WOL(Wake On Lan) 모드를 설정합니다.\n
            It also can block a packet such as \n
            IPv6 PING request from an all-node broadcasting, \n
            IPv6 PING request from a solicited mulitcasting address,\n
            IPv4 packets, \n
            and IPv6 packets.

    <table>
      <tr> <td>7 ~ 6   </td> <td>5  </td> <td>4  </td> <td>3       </td> <td>2
   </td> <td>1   </td> <td>0  </td> </tr> <tr> <td>Reserved</td> <td>ANB</td>
   <td>M6B</td> <td>Always 0</td> <td>WOL</td> <td>IP6B</td> <td>IP4B</td> </tr>
    </table>
    - @ref NETMR_ANB
    - @ref NETMR_M6B
    - @ref NETMR_WOL
    - @ref NETMR_IP6B
    - @ref NETMR_IP4B
    @sa getNETMR(), setNETMR()

*/
#define _NETMR_ (_W6100_IO_BASE_ + (0x4008 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief 네트워크 모드 레지스터 2 주소 [R=W][0x00]
    @details @ref _NETMR2_는 PPPoE 모드를 설정합니다.\n
            It also can select the destination hardware address to either
   Ethernet frame MAC or target MAC in the ARP-reply message <table> <tr> <td>7
   </td> <td>6 ~ 1</td> <td>0    </td> </tr> <tr> <td>DHAS</td> <td>6 ~ 1</td>
   <td>PPPoE</td> </tr>
    </table>
     - @ref NETMR2_DHAS : @ref NETMR2_DHAS_ARP, @ref NETMR2_DHAS_ETH
     - @ref NETMR2_PPPoE
    @sa getNETMR2(), setNETMR2()
*/
#define _NETMR2_ (_W6100_IO_BASE_ + (0x4009 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief PPP LCP 요청 타이머(Timer) 레지스터 주소 [R=W][0x28]
    @details @ref _PTMR_은 LCP 에코 요청 전송 주기를 설정합니다.\n
            The unit of time is 25ms.
    @sa _PMNR_, _PHAR_, _PSIDR_, _PMRUR_, NETMR2_PPPoE
    @sa getPTMR(), setPTMR(), getPMNR(), setPMNR(), getPHAR(), setPHAR(),
   getPSIDR(), setPSIDR(), getPMRUR(), setPMRUR(), getNETMR2(), setNETMR2()
*/
#define _PTMR_ (_W6100_IO_BASE_ + (0x4100 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief PPP LCP 매직 넘버(Magic Number) 레지스터 주소 [R=W][0x00]
    @details @ref _PMNR_은 LCP 협상에 사용될 4바이트 매직 넘버를 설정합니다.
    @sa _PTMR_, _PHAR_, _PSIDR_, _PMRUR_, NETMR2_PPPoE
    @sa getPMNR(), setPMNR(), getPTMR(), setPTMR(), getPHAR(), setPHAR(),
   getPSIDR(), setPSIDR(), getPMRUR(), setPMRUR(), getNETMR2(), setNETMR2()
*/
#define _PMNR_ (_W6100_IO_BASE_ + (0x4104 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief PPPoE 하드웨어 주소 레지스터 주소 [R=W][0x00]
    @details @ref _PHAR_은 PPPoE 접속 과정에서 획득한 PPPoE 서버 하드웨어 주소를
   설정합니다.
    @sa _PTMR_, _PMNR_, _PSIDR_, _PMRUR_, NETMR2_PPPoE
    @sa getPHAR(), setPHAR(), getPTMR(), setPTMR(), getPMNR(), setPMNR(),
   getPSIDR(), setPSIDR(), getPMRUR(), setPMRUR(), getNETMR2(), setNETMR2()
*/
#define _PHAR_ (_W6100_IO_BASE_ + (0x4108 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief PPP 세션 ID 레지스터 주소 [R=W][0X0000]
    @details @ref _PSIDR_은 PPPoE 접속 과정에서 얻은 PPPoE 서버 세션 ID를
   설정합니다.
    @sa _PTMR_, _PMNR_, _PHAR_, _PMRUR_, NETMR2_PPPoE
    @sa getPSIDR(), setPSIDR(), getPTMR(), setPTMR(), getPMNR(), setPMNR(),
   getPHAR(), setPHAR(), getPMRUR(), setPMRUR(), getNETMR2(), setNETMR2()
*/
#define _PSIDR_ (_W6100_IO_BASE_ + (0x4110 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief PPP 최대 수신 단위(MRU) 레지스터 주소 [R=W][0xFFFF]
    @details @ref _PMRUR_은 PPPoE의 최대 수신 단위(MRU)를 설정합니다.
    @sa _PTMR_, _PMNR_, _PHAR_, _PSIDR_, NETMR2_PPPoE
    @sa  getPMRUR(), setPMRUR(), getPTMR(), setPTMR(), getPMNR(), setPMNR(),
   getPHAR(), setPHAR(), getPSIDR(), setPSIDR(), getNETMR2(), setNETMR2()
*/
#define _PMRUR_ (_W6100_IO_BASE_ + (0x4114 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief 소스 하드웨어 주소 레지스터 주소 [R=W][00:00:00:00:00:00]
    @details @ref _SHAR_은 소스 하드웨어 주소를 설정합니다.
    @note It can be accessed only when @ref SYSR_NETL is unlock.
    @sa SYSR_NETL, _NETLCKR_
    @sa getSHAR(), setSHAR(), getSYSR(), setNETLCKR(), getNETLCKR(), NETLOCK(),
   NETUNLOCK()
*/
#define _SHAR_ (_W6100_IO_BASE_ + (0x4120 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief IPv4 게이트웨이(Gateway) 주소 레지스터 주소 [R=W][0.0.0.0]
    @details @ref _GAR_은 기본 게이트웨이 IPv4 주소를 설정합니다.
    @note It can be accessed only when @ref SYSR_NETL is unlock.
    @sa SYSR_NETL, _NETLCKR_, _GA6R_
    @sa getGAR(), setGAR(), getSYSR(), setNETLCKR(), getNETLCKR(), NETLOCK(),
   NETUNLOCK(), getGA6R(), setGA6R()
*/
#define _GAR_ (_W6100_IO_BASE_ + (0x4130 << 8) + WIZCHIP_CREG_BLOCK)
#define _GA4R_ (_GAR_) ///< Refer to @ref _GAR_
/**
    @brief IPv4 서브넷 마스크(Subnet Mask) 레지스터 주소 [R=W][0.0.0.0]
    @details @ref _SUBR_은 IPv4의 기본 서브넷 마스크 주소를 설정합니다.
    @note It can be accessed only when @ref SYSR_NETL is unlock.
    @sa SYSR_NETL, _NETLCKR_, _SUB6R_
    @sa getSUBR(), setSUBR(), getSYSR(), setNETLCKR(), getNETLCKR(), NETLOCK(),
   NETUNLOCK(), getSUB6R(), setSUB6R()
*/
#define _SUBR_ (_W6100_IO_BASE_ + (0x4134 << 8) + WIZCHIP_CREG_BLOCK)
#define _SUB4R_ (_SUBR_) ///< Refer to @ref _SUBR_

/**
    @brief IPv4 소스(Source) IP 레지스터 주소 [R=W][0.0.0.0]
    @details @ref _SIPR_은 소스 IPv4 주소를 설정합니다.
    @note It can be accessed only when @ref SYSR_NETL is unlock.
    @sa SYSR_NETL, _NETLCKR_, _LLAR_, _GUAR_
    @sa getSIPR(), setSIPR(), getSYSR(), setNETLCKR(), getNETLCKR(), NETLOCK(),
   NETUNLOCK(), getLLAR(), setLLAR(), getGUAR(),setGUAR()
*/
#define _SIPR_ (_W6100_IO_BASE_ + (0x4138 << 8) + WIZCHIP_CREG_BLOCK)
#define _SIP4R_ (_SIPR_) ///< Refer to @ref _SIPR_.

/**
    @brief IPv6 LLA(Link Local Address) 레지스터 주소 [R=W][::]
    @details @ref _LLAR_은 IPv6의 LLA 주소를 설정합니다.
    @note It can be accessed only when @ref SYSR_NETL is unlock.
    @sa SYSR_NETL, _NETLCKR_, _GUAR_, _SIPR_
    @sa getLLAR(), setLLAR(), getSYSR(), setNETLCKR(), getNETLCKR(), NETLOCK(),
   NETUNLOCK(), getGUAR(),setGUAR(), getSIPR(), setSIPR()
*/
#define _LLAR_ (_W6100_IO_BASE_ + (0x4140 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief IPv6 GUA(Global Unicast Address) 레지스터 주소 [R=W][::]
    @details @ref _GUAR_은 IPv6의 GUA 주소를 설정합니다.
    @note It can be accessed only when @ref SYSR_NETL is unlock.
    @sa SYSR_NETL, _NETLCKR_, _LLAR_, _SIPR_
    @sa getGUAR(), setGUAR(), getSYSR(), setNETLCKR(), getNETLCKR(), NETLOCK(),
   NETUNLOCK(), getLLAR(),setLLAR(), getSIPR(), setSIPR()
*/
#define _GUAR_ (_W6100_IO_BASE_ + (0x4150 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief IPv6 서브넷 마스크(Subnet Mask) 레지스터 주소 [R=W][]
    @details @ref _SUB6R_은 IPv6의 기본 서브넷 마스크 주소를 설정합니다.
    @note It can be accessed only when @ref SYSR_NETL is unlock.
    @sa SYSR_NETL, _NETLCKR_, _SUBR_
    @sa getSUB6R(), setSUB6R(), getSYSR(), setNETLCKR(), getNETLCKR(),
   NETLOCK(), NETUNLOCK(), getSUBR(), setSUBR()
*/
#define _SUB6R_ (_W6100_IO_BASE_ + (0x4160 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief IPv6 게이트웨이(Gateway) 주소 레지스터 주소 [R/W][::]
    @details @ref _GA6R_은 기본 게이트웨이 IPv6 주소를 설정합니다.
    @sa _GAR_
    @sa getGA6R(), setGA6R(), getGAR(), setGAR()
*/
#define _GA6R_ (_W6100_IO_BASE_ + (0x4170 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief SOCKET-less 피어 IPv6 주소 레지스터 주소 [R=W][::]
    @details @ref _SLDIP6R_은 @ref _SLCR_의 목적지 IP 주소를 설정합니다.
    @sa _SLDIP6R_, _SLCR_, _SLIR_, _SLIRCLR_, _SLIMR_, _SLDHAR_, _SLDIPR_,
   _SLDIP4R_
    @sa getSLDIP6R(), setSLDIP6R(), getSLCR(), setSLCR(), getSLIR(),
   setSLIRCLR(), getSLIMR(), setSLIMR(), getSLDHAR(), getSLDIPR(), setSLDIPR(),
   getSLDIP4R(), setSLDIP4R()
*/
#define _SLDIP6R_ (_W6100_IO_BASE_ + (0x4180 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief SOCKET-less 피어 IPv4 주소 레지스터 주소 [R=W][0.0.0.0]
    @details @ref _SLDIPR_(= @ref _SLDIP4R_)은 @ref _SLCR_의 목적지 IPv4 주소를
   설정합니다.
    @sa _SLDIP4R_, _SLCR_, _SLIR_, _SLIRCLR_, _SLIMR_, _SLDHAR_, _SLDIP6R_
    @sa getSLDIPR(), setSLDIPR(), getSLDIP4R(), setSLDIP4R(), getSLCR(),
   setSLCR(), getSLIR(), setSLIRCLR(), getSLIMR(), setSLIMR(), getSLDHAR(),
       getSLDIP6R(), setSLDIP6R()
*/
#define _SLDIPR_ (_W6100_IO_BASE_ + (0x418C << 8) + WIZCHIP_CREG_BLOCK)
#define _SLDIP4R_ (_SLDIPR_) ///< Refer to @ref _SLDIPR_.

/**
    @brief SOCKET-less 피어 하드웨어 주소 레지스터 주소 [RO][00:00:00:00:00:00]
    @details @ref _SLDHAR_은 @ref SLCR_ARP4, SLCR_ARP6, SLCR_PING4, SLCR_PING6에
   의해 획득된 목적지 하드웨어 주소를 가져옵니다.
    @sa _SLDIP4R_, _SLDIP6R_, _SLCR_, _SLIR_, _SLIRCLR_, _SLIMR_
    @sa getSLDHAR(), getSLDIP4R(), setSLDIP4R(), getSLDIP6R(), setSLDIP6R(),
   getSLCR(), setSLCR(), \n getSLIR(), setSLIRCLR(), getSLIMR(), setSLIMR()
*/
#define _SLDHAR_ (_W6100_IO_BASE_ + (0x4190 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief SOCKET-less 핑 ID 레지스터 주소 [R=W][0x00]
    @details @ref _PINGIDR_은 @ref SLCR_PING4 또는 @ref SLCR_PING6으로 보낼 핑
   요청 ID를 설정합니다.
    @sa _SLCR_, _PINGSEQR_, _SLDIPR_, _SLDIP4R_, _SLDIP6R_, _SLDHAR_, _SLIR_,
   _SLIRCLR_, _SLIMR_
    @sa getPINGIDR(), setPINGIDR(), getSLCR(), setSLCR(), getPINGSEQR(),
   setPINGSEQR(), getSLDIPR(), setSLDIPR(), getSLDIP4R(), setSLDIP4R(),
   getSLDIP6R(), setSLDIP6R(), getSLDHAR(), getSLIR(), setSLIRCLR(), getSLIMR(),
   setSLIMR()
*/
#define _PINGIDR_ (_W6100_IO_BASE_ + (0x4198 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief SOCKET-less 핑 시퀀스 번호 레지스터 주소 [R=W][0x0000]
    @details @ref _PINGSEQR_은 @ref SLCR_PING4 또는 @ref SLCR_PING6으로 전송될
   핑 요청 시퀀스 번호를 설정합니다.
    @sa _SLCR_, _PINGIDR_, _SLDIPR_, _SLDIP4R_, _SLDIP6R_, _SLDHAR_, _SLIR_,
   _SLIRCLR_, _SLIMR_
    @sa getPINGSEQR(), setPINGSEQR(), getSLCR(), setSLCR(), getPINGIDR(),
   setPINGIDR(), getSLDIPR(), setSLDIPR(), getSLDIP4R(), setSLDIP4R(),
       getSLDIP6R(), setSLDIP6R(), getSLDHAR(), getSLIR(), setSLIRCLR(),
   getSLIMR(), setSLIMR()
*/
#define _PINGSEQR_ (_W6100_IO_BASE_ + (0x419C << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief IPv4 Unreachable 주소 레지스터 주소 [RO][0.0.0.0]
    @details @ref _UIPR_은 Unreachable ICMPv4 메시지를 수신할 때 설정됩니다.
    @sa _UPORTR_, _UIP6R_, _UPORT6R_
    @sa getUIPR(), setUIPR(), getUPORTR(), setUPORTR(), getUIPR6(), setUIPR6(),
   getUPORT6R(), setUPORT6R()
*/
#define _UIPR_ (_W6100_IO_BASE_ + (0x41A0 << 8) + WIZCHIP_CREG_BLOCK)
#define _UIP4R_ (_UIPR_) ///< Refer to @ref _UPORTR_

/**
    @brief IPv4 Unreachable 포트 번호 레지스터 주소 [RO][0x0000]
    @details @ref _UPORTR_은 Unreachable ICMPv4 메시지를 수신할 때 설정됩니다.
    @sa _UIPR_, _UIP6R_, _UPORT6R_
    @sa getUPORTR(), setUPORTR(), getUIPR(), setUIPR(), getUIPR6(), setUIPR6(),
   getUPORT6R(), setUPORT6R()
*/
#define _UPORTR_ (_W6100_IO_BASE_ + (0x41A4 << 8) + WIZCHIP_CREG_BLOCK)
#define _UPORT4R_ (_UPORTR_) ///< Refer to @ref _UPORTR_
/**
    @brief IPv6 Unreachable IP 주소 레지스터 주소 [RO][::]
    @details @ref _UIP6R_은 Unreachable ICMPv6 메시지를 수신할 때 설정됩니다.
    @sa _UIPR_, _UPORTR_, _UIP6R_, _UPORT6R_
    @sa getUIPR6(), setUIPR6(), getUIPR(), setUIPR(), getUPORTR(), setUPORTR(),
   getUPORT6R(), setUPORT6R()
*/
#define _UIP6R_ (_W6100_IO_BASE_ + (0x41B0 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief IPv6 Unreachable 포트 번호 레지스터 주소 [RO][0x0000]
    @details @ref _UIP6R_은 Unreachable ICMPv6 메시지를 수신할 때 설정됩니다.
    @sa _UIPR_, _UPORTR_, _UIP6R_, _UPORT6R_
    @sa getUIPR6(), setUIPR6(), getUIPR(), setUIPR(), getUPORTR(), setUPORTR(),
   getUPORT6R(), setUPORT6R()
*/
#define _UPORT6R_ (_W6100_IO_BASE_ + (0x41C0 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief 인터럽트 보류 시간(Pending Time) 레지스터 주소 [R=W][0x0000]
    @details @ref _INTPTMR_은 @ref _WIZCHIP_의 INTn 핀에서 발생한 다음
   인터럽트를 보류합니다.\n It is decreased 1 every 4 SYS_CLK. \n If it is zero
   and some interrupt is still remained, the INTn pin is issued.
    @sa _IR_, _IRCLR_, _IMR_, _SIR_, _Sn_IRCLR_, _SIMR_, _SLIR_, _SLIRCLR_,
   _SLIMR_, SYCR_IEN
    @sa getINTPTMR(), setINTPTMR(), getIR(), setIRCLR(), getIMR(), setIMR(),
   getSIR(), setSn_IRCLR(), getSIMR(), setSIMR(), \n getSLIR(), setSLIRCLR(),
   getSLIMR(), setSLIMR(), getSYCR1(), setSYCR1()
*/
#define _INTPTMR_ (_W6100_IO_BASE_ + (0x41C5 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief RA 프리픽스(Prefix) 길이 레지스터 주소 [RO][0x00]
    @details @ref _PLR_은 라우터로부터 RA 패킷을 수신할 때 설정됩니다.
    @sa SLIR_RA, _SLIRCLR_, _PFR_, _VLTR_, _PLTR_, _PAR_
    @sa getPLR(), getSLIR(), setSLIRCLR(), getPFR(), getVLTR(), getPLTR(),
   getPAR()
*/
#define _PLR_ (_W6100_IO_BASE_ + (0x41D0 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief RA 프리픽스(Prefix) 플래그 레지스터 주소 [RO][0x00]
    @details @ref _PFR_은 라우터로부터 RA 패킷을 수신할 때 설정됩니다.
    @sa SLIR_RA, _SLIRCLR_, _PLR_, _VLTR_, _PLTR_, _PAR_
    @sa getPFR(), getSLIR(), setSLIRCLR(), getPLR(), getVLTR(), getPLTR(),
   getPAR()
*/
#define _PFR_ (_W6100_IO_BASE_ + (0x41D4 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief RA 유효(Valid) 수명 레지스터 주소 [RO][0x00000000]
    @details @ref _VLTR_은 라우터로부터 RA 패킷을 수신할 때 설정됩니다.
    @sa SLIR_RA, _SLIRCLR_, _PLR_, _PFR_, _PLTR_, _PAR_
    @sa getVLTR(), getSLIR(), setSLIRCLR(), getPLR(), getPFR(),  getPLTR(),
   getPAR()
*/
#define _VLTR_ (_W6100_IO_BASE_ + (0x41D8 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief RA 권장(Preferred) 수명 레지스터 주소 [RO][0x00000000]
    @details @ref _PLTR_은 라우터로부터 RA 패킷을 수신할 때 설정됩니다.
    @sa SLIR_RA, _SLIRCLR_, _PLR_, _PFR_, _PLTR_, _PAR_
    @sa getPLTR(), getSLIR(), setSLIRCLR(), getPLR(), getPFR(), getVLTR(),
   getPAR()
*/
#define _PLTR_ (_W6100_IO_BASE_ + (0x41DC << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief RA 프리픽스(Prefix) 주소 레지스터 주소[RO][::]
    @details @ref _PAR_은 라우터로부터 RA 패킷을 수신할 때 설정됩니다.
    @sa SLIR_RA, _SLIRCLR_, _PLR_, _PFR_, _VLTR_, _PLTR_, _PAR_
    @sa getPAR(), getPLTR(), getSLIR(), setSLIRCLR(), getPLR(), getPFR(),
   getVLTR()
*/
#define _PAR_ (_W6100_IO_BASE_ + (0x41E0 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief ICMPv6 차단(Block) 레지스터 주소 [R=W][0x00]
    @details @ref _ICMP6BLKR_는 PING, MLD, RA, NS 및 NA와 같은 ICMPv6 메시지를
   차단할 수 있습니다.\n In this blocked case, @ref Sn_MR_IPRAW6 SOCKET can
   receive it. <table> <tr> <td>7 ~ 5</td> <td>4    </td> <td>3  </td> <td>2
   </td> <td>1 </td> <td>0 </td> </tr> <tr> <td>7 ~ 5</td> <td>PING6</td>
   <td>MLD</td> <td>RA</td> <td>NA</td> <td>NS</td> </tr>
    </table>
     - @ref ICMP6BLKR_PING6 : The same as @ref NETxMR_PB
     - @ref ICMP6BLKR_MLD
     - @ref ICMP6BLKR_RA
     - @ref ICMP6BLKR_NA
     - @ref ICMP6BLKR_NS

    @note The blocked message can be accepted by SOCKETn opened with @ref
   Sn_MR_IPRAW6.
    @sa NETxMR_PB
    @sa getICMP6BLKR(), setICMP6BLKR(), getNET6MR(), setNET6MR()
*/
#define _ICMP6BLKR_ (_W6100_IO_BASE_ + (0x41F0 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief 칩 설정 잠금(Lock) 레지스터 주소 [WO][0x00]
    @details @ref _CHPLCKR_는 @ref _SYCR0_ 및 @ref _SYCR1_에 대한 접근을
   잠그거나 잠금 해제할 수 있습니다.\n The lock state can be checked from @ref
   SYSR_CHPL.
    @sa _SYCR0_, _SYCR1_, _SYSR_, SYSR_CHPL
    @sa getCHPLCKR(), setCHPLCKR(), CHIPLOCK(), CHIPUNLOCK(), getSYSR()
*/
#define _CHPLCKR_ (_W6100_IO_BASE_ + (0x41F4 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief 네트워크 설정 잠금 레지스터 주소 [WO][0x00]
    @details @ref _NETLCKR_는 @ref _SIPR_, @ref _LLAR_ 등과 같은 네트워크 정보
   레지스터에 접근하기 위한 잠금 또는 잠금 해제를 설정할 수 있습니다.\n The lock
   state can be checked from @ SYSR_NETL.
    @sa _SHAR_, _SIPR_, _SUBR_, _GAR_, _LLAR_, _GUAR_, _SUB6R_, _SYSR_,
   SYSR_NETL
    @sa getNETLCKR(), setNETLCKR(), NETLOCK(), NETUNLOCK(), getSHAR(),
   setSHAR(), getSIPR(), getSIPR(), getSUBR(), setSUBR(), \n getGAR(), setGAR(),
   getLLAR(), setLLAR(), getGUAR(), setGUAR(), getSUB6R(), setSUB6R(), getSYSR()
*/
#define _NETLCKR_ (_W6100_IO_BASE_ + (0x41F5 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief PHY 설정 잠금(Lock) 레지스터 주소 [WO][0x00]
    @details @ref _PHYLCKR_는 @ref _PHYCR0_ 및 @ref _PHYCR1_에 대한 접근을
   잠그거나 해제할 수 있습니다.\n The lock state can be checked from @ref
   SYSR_PHYL.
    @sa _PHYCR0_, _PHYCR1_, _SYSR_, SYSR_PHYL.
    @sa getPHYLCKR(), setPHYLCKR(), PHYLOCK(), PHYUNLOCK(), setPHYCR0(),
   getPHYCR1(), setPHYCR1(), getSYSR()
*/
#define _PHYLCKR_ (_W6100_IO_BASE_ + (0x41F6 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief 재전송 시간 레지스터 주소 [R=W][0x07D0]
    @details @ref _RTR_은 @ref _Sn_RTR_의 기본 시간 초과(timeout) 값을
   설정합니다.\n When @ref _Sn_RTR_ is 0, @ref _Sn_RTR_ is reset to @ref _RTR_
   after @ref Sn_CR_OPEN.
    @sa _Sn_RTR_, _RCR_, _Sn_RCR_, _Sn_CR_, Sn_CR_OPEN, _Sn_IR_, _Sn_IRCLR_,
   Sn_IR_TIMEOUT
    @sa getRTR(), setRTR(), getSn_RTR(), setSn_RTR(), getRCR(), setRCR(),
   getSn_RCR(), setSn_RCR(),  \n getSn_CR(), getSn_CR(), getSn_IR(),
   setSn_IRCLR()
*/
#define _RTR_ (_W6100_IO_BASE_ + (0x4200 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief 재전송 카운터 레지스터 주소 [R=W][0x08]
    @details @ref _RCR_은 @ref _Sn_RCR_의 기본 재전송 횟수를 설정합니다.\n
            When @ref _Sn_RCR_ is 0, @ref _Sn_RCR_ is initialized as @ref
   _Sn_RTR_ after @ref Sn_CR_OPEN.
    @sa _Sn_RCR_, _RTR_, _Sn_RTR_, _Sn_CR_, Sn_CR_OPEN, _Sn_IR_, _Sn_IRCLR_,
   Sn_IR_TIMEOUT
    @sa getRCR(), setRCR(), getSn_RCR(), setSn_RCR(), getRTR(), setRTR(),
   getSn_RTR(), setSn_RTR(), \n getSn_CR(), getSn_CR(), getSn_IR(),
   setSn_IRCLR()
*/
#define _RCR_ (_W6100_IO_BASE_ + (0x4204 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief SOCKET-less 재전송 시간 레지스터 주소 [R=W][0x07D0]
    @details @ref _SLRTR_은 @ref _SLCR_에 의해 재전송될 패킷의 시간 초과 값을
   설정합니다.
    @sa _SLRCR_, _SLIR_, _SLIRCLR_, SLIR_TOUT
    @sa getSLRTR(), setSLRTR(), getSLRCR(), setSLRCR(), getSLIR(), setSLIRCLR()
*/
#define _SLRTR_ (_W6100_IO_BASE_ + (0x4208 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief SOCKET-less 재전송 카운트 레지스터 주소 [R=W][0x00]
    @details @ref _SLRCR_은 @ref _SLCR_에 의해 재전송될 패킷의 재시도 횟수를
   설정합니다.
    @sa _SLRTR_, _SLIR_, _SLIRCLR_, SLIR_TOUT
    @sa getSLRCR(), setSLRCR(), getSLRTR(), setSLRTR(), setSLIRCLR(), getSLIR(),
   setSLIRCLR(),
*/
#define _SLRCR_ (_W6100_IO_BASE_ + (0x420C << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief SOCKET-less 홉 제한(Hop Limit) 레지스터 주소 [R=W][0x80]
    @details @ref _SLHOPR_은 @ref _SLCR_에 의해 전송될 패킷의 홉 제한 값을
   설정합니다.
    @sa _SLCR_
    @sa getSLHOPR(), setSLHOPR(), getSLCR(), setSLCR()
*/
#define _SLHOPR_ (_W6100_IO_BASE_ + (0x420F << 8) + WIZCHIP_CREG_BLOCK)

//----------------------------- W6100 Socket Registers
//-----------------------------

/**
    @brief 소켓(Socket) 모드 레지스터 주소 [R=W][0x00]
    @details @ref _Sn_MR_은 @ref Sn_CR_OPEN을 수행하기 전에 SOCKETn의 옵션 또는
   프로토콜 유형을 설정합니다.\n\n Each bit of @ref _Sn_MR_ is defined as the
   following. <table> <tr> <td>7       </td> <td>6        </td> <td>5 </td>
   <td>4        </td> <td>3 ~ 0 </td> </tr> <tr> <td>MULTI/MF</td>
   <td>BRDB/FPSH</td> <td>ND/MC/SMB/MMB</td> <td>UNIB/MMB6</td> <td>P[3:0]</td>
   </tr>
    </table>
     - @ref Sn_MR_MULTI : Support UDP 멀티캐스팅
     - @ref Sn_MR_MF    : Support MAC 필터 Enable
     - @ref Sn_MR_BRDB  : Broadcast Block
     - @ref Sn_MR_FPSH  : Force PSH flag
     - @ref Sn_MR_ND    : No Delay ACK flag
     - @ref Sn_MR_MC    : IGMP ver2, ver1
     - @ref Sn_MR_SMB   : Solicited Multicast Block
     - @ref Sn_MR_MMB   : IPv4 Multicast block
     - @ref Sn_MR_UNIB  : 유니캐스트(Unicast) 차단
     - @ref Sn_MR_MMB6  : IPv6 UDP Multicast Block
     - <b>P[3:0]</b>
    <table>
      <tr> <td> P[3:0] </td> <td> Protocol Mode  </td> </tr>
      <tr> <td> 0000   </td> <td> SOCKET Closed  </td> </tr>
      <tr> <td> 0001   </td> <td> TCP4           </td> </tr>
      <tr> <td> 0010   </td> <td> UDP4           </td> </tr>
      <tr> <td> 0011   </td> <td> IPRAW4         </td> </tr>
      <tr> <td> 0100   </tr> <td> MACRAW         </td> </tr>
      <tr> <td> 1001   </td> <td> TCP6           </td> </tr>
      <tr> <td> 1010   </td> <td> UDP6           </td> </tr>
      <tr> <td> 1100   </td> <td> IPRAW6         </td> </tr>
      <tr> <td> 1101   </td> <td> TCP Dual(TCPD) </td> </tr>
      <tr> <td> 1110   </td> <td> UDP Dual (UDPD)</td> </tr>
    </table>
     - @ref Sn_MR_CLOSE                      : SOCKET Closed
     - @ref Sn_MR_TCP4(= @ref Sn_MR_TCP)     : TCP4 mode
     - @ref Sn_MR_UDP4(= @ref Sn_MR_UDP)     : UDP4 mode
     - @ref Sn_MR_IPRAW4(= @ref Sn_MR_IPRAW) : IPRAW4 mode
     - @ref Sn_MR_MACRAW                     : MACRAW 모드
     - @ref Sn_MR_TCP6                       : TCP6 mode
     - @ref Sn_MR_UDP6                       : UDP6 mode
     - @ref Sn_MR_IPRAW6                     : IPRAW6 mode
     - @ref Sn_MR_TCPD                       : TCP Dual (TCPD) mode
     - @ref Sn_MR_UDPD                       : UDP Dual (UDPD) mode

    @note MACRAW 모드 should be only used in Socket 0.
    @sa _Sn_CR_, Sn_CR_OPEN, _Sn_SR_, _Sn_MR2_
    @sa getSn_MR(), setSn_MR(), getSn_CR(), setSn_CR(), getSn_SR(), getSn_MR2(),
   setSn_MR2()
*/
#define _Sn_MR_(N) (_W6100_IO_BASE_ + (0x0000 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKET n 기본 소스 IPv6 주소 레지스터 주소 [R=W][0x00]
    @details @ref _Sn_PSR_은 @ref _Sn_CR_로 패킷을 전송하기 위한 소스 IPv6
   주소를 선택합니다. This function is same as @ref _SLPSR_.
     - @ref PSR_AUTO
     - @ref PSR_LLA
     - @ref PSR_GUA
    @sa _Sn_CR_, _Sn_PSR_, _SLPSR_
    @sa getSn_PSR(), setSn_PSR(), getSLCR(), setSLCR(), getSLPSR(), setSLPSR(),
*/
#define _Sn_PSR_(N) (_W6100_IO_BASE_ + (0x0004 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief 소켓(Socket) 명령어(Command) 레지스터 주소 [RW,AC][0x00]
    @details @ref _Sn_CR_은 SOCKET n에 대한 명령(OPEN, CLOSE, CONNECT, LISTEN,
   SEND, RECEIVE 등)을 설정하는 데 사용됩니다.\n It is automatically cleared to
   0x00 after the command is recognized by @ref _WIZCHIP_.\n Even though @ref
   _Sn_CR_ is cleared to 0x00, the command is still being processed.\n To check
   whether the command is completed or not, please check the @ref _Sn_IR_ or
   @ref _Sn_SR_.
     - @ref Sn_CR_OPEN            : Initialize or open socket.
     - @ref Sn_CR_LISTEN       : Wait connection request on TCP4/TCP6/TCPD
   mode(<b>Server mode</b>)
     - @ref Sn_CR_CONNECT      : Send connection request on TCP4/TCPD
   mode(<b>Client mode</b>)
     - @ref Sn_CR_CONNECT6  : Send connection request on TCP6/TCPD
   mode(<b>Client mode</b>):nohl
     - @ref Sn_CR_DISCON       : Send closing request on TCP/TCP6/TCPD mode.
     - @ref Sn_CR_CLOSE        : Close socket.
     - @ref Sn_CR_SEND            : Update TX buffer pointer and send data in
   IPv4 socket.
     - @ref Sn_CR_SEND6        : Update TX buffer pointer and send data in IPv6
   socket.
     - @ref Sn_CR_SEND_KEEP : Keep Alive 메시지 전송.
     - @ref Sn_CR_RECV            : Update RX buffer pointer and receive data.

    @note These commands should be exclusive executed.\n That is, the other
   command can not executed when one command is not cleared yet.
    @sa _Sn_IR_, _Sn_IRCLR_, Sn_IMR_, _SIR_, _Sn_SR_
    @sa getSn_CR(), setSn_CR(), getSn_IR(), setSn_IRCLR(), getSn_IMR(),
   setSn_IMR(), getSIR(), getSn_SR()
*/
#define _Sn_CR_(N) (_W6100_IO_BASE_ + (0x0010 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn 인터럽트 레지스터 주소 [RO][0x00]
    @details @ref _Sn_IR_은 연결, 종료, 데이터 수신, 시간 초과와 같은 SOCKETn
   인터럽트 상태를 가져옵니다.\n If SOCKETn interrupt occurs and the n-th bit of
   @ref _SIMR_ is set, then @ref SIR_INT(n) is set.\n In order to clear the @ref
   _Sn_IR_ bit, Set the corresponding bit of _Sn_IRCLR_ to 1.\n If all @ref
   _Sn_IR_ bits are cleared, the @ref SIR_INT(n) is automatically cleared.
    <table>
       <tr> <td>7 ~ 5   </td> <td>4     </td> <td>3      </td> <td>2   </td>
   <td>1     </td> <td>0  </td> </tr> <tr> <td>Reserved</td> <td>SENDOK</td>
   <td>TIMEOUT</td> <td>RECV</td> <td>DISCON</td> <td>CON</td> </tr>
    </table>
     - @ref Sn_IR_SENDOK
     - @ref Sn_IR_TIMEOUT
     - @ref Sn_IR_RECV
     - @ref Sn_IR_DISCON
     - @ref Sn_IR_CON

    @sa _Sn_IRCLR_, _Sn_IMR_, _SIR_, _SIMR_
    @sa getSn_IR(), setSn_IRCLR(), getSn_IMR(), setSn_IMR(), getSIR(),
   getSIMR(), setSIMR()
*/
#define _Sn_IR_(N) (_W6100_IO_BASE_ + (0x0020 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn 인터럽트 마스크 레지스터 주소 [R=W][0xFF]
    @details @ref _Sn_IMR_은 @ref _Sn_IR_의 인터럽트를 마스킹하는데 사용됩니다.
    @sa _Sn_IR_, _Sn_IRCR_, _SIR_, _SIMR_
    @sa getSn_IMR(), setSn_IMR(), getSn_IR(), setSn_IRCLR(), getSIR(),
   getSIMR(), setSIMR()
*/
#define _Sn_IMR_(N) (_W6100_IO_BASE_ + (0x0024 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn 인터럽트 클리어 레지스터 주소 [WO][0x00]
    @details @ref _Sn_IRCLR_은 @ref _Sn_IR_을 초기화(clear)합니다
    @sa _Sn_IR_, _SIR_, _SIMR_
    @sa setSn_IRCLR(), getSn_IR(), getSIR(), getSIMR(), setSIMR()
*/
#define _Sn_IRCLR_(N) (_W6100_IO_BASE_ + (0x0028 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn 상태 레지스터 주소 [RO][0x00]
    @details @ref _Sn_SR_은 SOCKETn의 상태를 나타냅니다.\n
            The status of SOCKETn can be changed by @ref _Sn_CR_, some TCP
   packets such as SYN, FIN, RST packet, or @ref Sn_IR_TIMEOUT.
    - Normal status
      - @ref SOCK_CLOSED      : Closed
      - @ref SOCK_INIT        : Initiate state
      - @ref SOCK_LISTEN      : Listen state
      - @ref SOCK_ESTABLISHED : Success to connect
      - @ref SOCK_CLOSE_WAIT  : Closing state
      - @ref SOCK_UDP         : UDP socket
      - @ref SOCK_IPRAW       : IPRAW socket
      - @ref SOCK_IPRAW6      : IPv6 IPRAW socket
      - @ref SOCK_MACRAW      : MAC raw mode socket
    - Temporary status during changing the status of SOCKETn .
      - @ref SOCK_SYNSENT     : This indicates SOCKETn  sent the connect-request
   packet (SYN packet) to a peer.
      - @ref SOCK_SYNRECV     : It indicates SOCKETn  successfully received the
   connect-request packet (SYN packet) from a peer.
      - @ref SOCK_FIN_WAIT    : Connection state
      - @ref SOCK_TIME_WAIT   : Closing state
      - @ref SOCK_LAST_ACK    : Closing state

    @sa _Sn_CR_, _Sn_IR_, _Sn_IRCLR_, Sn_IR_TIMEOUT
    @sa getSn_SR(), getSn_CR(), setSn_CR(), getSn_IR(), setSn_IRCLR()

    <table width=0 >
       <tr> <td>@image html SocketStatus.png "<SOCKETn Status Transition>"</td>
   </tr>
    </table>

*/
#define _Sn_SR_(N) (_W6100_IO_BASE_ + (0x0030 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn 확장 상태 레지스터 주소 [RO][0x00]
    @details @ref _Sn_ESR_은 연결된 클라이언트의 IP 버전, IPv6 주소 유형(LLA
   또는 GUA)과 같은 IP 주소 정보를 나타냅니다, \n and TCP operation mode such as
   <b>TCP SERVER</b> and <b>TCP CLIENT</b> <table> <tr> <td> 7 ~ 3   </td> <td>2
   </td> <td>1    </td> <td>0   </td> </tr> <tr> <td> Reserved</td>
   <td>TCPM</td> <td>TCPOP</td> <td>IP6T</td> </tr>
    </table>
    - @ref Sn_ESR_TCPM  : @ref Sn_ESR_TCPM_IPV4, @ref Sn_ESR_TCPM_IPV6
    - @ref Sn_ESR_TCPOP : @ref Sn_ESR_TCPOP_SVR, @ref Sn_ESR_TCPOP_CLT
    - @ref Sn_ESR_IP6T  : @ref Sn_ESR_IP6T_LLA,  @ref Sn_ESR_IP6T_GUA

    @note It is valid only on TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6,
   and @ref Sn_MR_TCPD.
    @sa _Sn_MR_, _Sn_PSR_
    @sa getSn_ESR(), getSn_MR(), setSn_MR(), getSn_PSR(), setSn_PSR()
*/
#define _Sn_ESR_(N) (_W6100_IO_BASE_ + (0x0031 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn IP 프로토콜 번호(PN) 레지스터 주소 [R/W][0x0000]
    @details \ref _Sn_PNR_는 IP 계층의 IPv4/IPv6 헤더에서 프로토콜 번호/다음
   헤더 필드를 설정합니다.
    @note It is valid only in IPRAW mode such as @ref Sn_MR_IPRAW4 and @ref
   Sn_MR_IPRAW6.
    @note It is set before @ref Sn_CR_OPEN is performed.
    @sa _Sn_NHR_, _Sn_MR_, Sn_CR_OPEN
    @sa getSn_PNR(), setSn_PNR(), getSn_NHR(), setSn_NHR(), getSn_MR(),
   setSn_MR(), getSn_CR(), setSn_CR()
*/
#define _Sn_PNR_(N) (_W6100_IO_BASE_ + (0x0100 << 8) + WIZCHIP_SREG_BLOCK(N))
#define _Sn_NHR_(N) (_Sn_PNR_(N)) ///< Refer to @ref _Sn_PNR_.

/**
    @brief SOCKETn IPv4 TOS(Type Of Service) 레지스터 주소 [R=W][0x00]
    @details @ref _Sn_TOSR_은 IPv4 헤더 내 TOS(Type Of Service) 필드를
   설정합니다.
    @sa getSn_TOSR(), setSn_TOSR()
*/
#define _Sn_TOSR_(N) (_W6100_IO_BASE_ + (0x0104 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn IP TTL(Time To Live) 레지스터 주소 [R=W][0x80]
    @details @ref _Sn_TTLR_은 IP 계층의 IPv4/IPv6 헤더 내 TTL(Time To
   Live)/HOP(Hop Limit) 필드를 설정합니다.
    @sa _Sn_HOPR_
    @sa getSn_TTLR(), setSn_TTLR(), getSn_HOPR(), setSn_HOPR()
*/
#define _Sn_TTLR_(N) (_W6100_IO_BASE_ + (0x0108 << 8) + WIZCHIP_SREG_BLOCK(N))
#define _Sn_HOPR_(N) (_Sn_TTLR_(N)) ///< Refer to @ref _Sn_TTLR_.

/**
    @brief SOCKETn 단편화(Fragment) 레지스터 주소 [R=W][0x4000]
    @details @ref _Sn_FRGR_는 IPv4 헤더 내 단편화 플래그 및 오프셋을 설정합니다.
    @note @ref _WIZCHIP_ can not support IP fragment & re-assembly.\n So It is
   not recommended to set @ref _Sn_FRGR_ to any other value.
    @sa getSn_FRGR(), setSn_FRGR()
*/
#define _Sn_FRGR_(N) (_W6100_IO_BASE_ + (0x010C << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn 최대 세그먼트 크기(MSS) 레지스터 주소 [RW][0x0000]
    @details @ref _Sn_MSSR_은 SOCKETn의 MTU(Maximum Transfer Unit)를 설정하거나
   가져옵니다. \n The MTU of each protocol is as following. <table> <tr> <td>
   @ref _Sn_MR_[3:0] </td> <td>@ref NETMR2_PPPoE = 0 </td> <td>@ref NETMR2_PPPoE
   = '1'</td> </tr> <tr> <td> @ref Sn_MR_TCP4   </td> <td> 1 ~ 1460 </td> <td> 1
   ~ 1452              </td> </tr> <tr> <td> @ref Sn_MR_TCP6   </td> <td> 1 ~
   1440             </td> <td> 1 ~ 1432              </td> </tr> <tr> <td> @ref
   Sn_MR_UDP4   </td> <td> 1 ~ 1472             </td> <td> 1 ~ 1464 </td> </tr>
      <tr> <td> @ref Sn_MR_UDP6   </td> <td> 1 ~ 1452             </td> <td> 1 ~
   1444              </td> </tr> <tr> <td> @ref Sn_MR_IPRAW4 </td> <td> 1 ~ 1480
   </td> <td> 1 ~ 1472              </td> </tr> <tr> <td> @ref Sn_MR_IPRAW6
   </td> <td> 1 ~ 1460             </td> <td> 1 ~ 1452              </td> </tr>
      <tr> <td> @ref Sn_MR_MACRAW </td> <td colspan = "2">      1 ~ 1514 </td>
   </tr>
    </table>

    @note It is not set exceeding the MTU for each protocol of SOCKETn even if
   _Sn_MSSR_ is set over the MTU.
    @sa _Sn_MR_, NETMR2_PPPoE
    @sa getSn_MSSR(), setSn_MSSR(), getSn_MR(), setSn_MR(), getNETMR2(),
   setNETMR2()
*/
#define _Sn_MSSR_(N) (_W6100_IO_BASE_ + (0x0110 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn 소스 포트 레지스터 주소 [R=W][0x0000]
    @details @ref _Sn_PORTR_은 SOCKETn의 소스 포트 번호를 설정합니다.
    @note It is valid in TCP(@ref Sn_MR_TCP4, @ref Sn_MR_TCP6, @ref Sn_MR_TCPD)
   and UDP(@ref Sn_MR_UDP4, @ref Sn_MR_UDP6, @ref Sn_MR_UDPD) mode.
    @note It should be set before @ref Sn_CR_OPEN is performed.
    @sa _Sn_MR_, Sn_CR_OPEN
    @sa getSn_PORTR(), getSn_PORTR(), getSn_MR(), setSn_MR(), getSn_CR(),
   setSn_CR()
*/
#define _Sn_PORTR_(N) (_W6100_IO_BASE_ + (0x0114 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn 목적지 하드웨어 주소 레지스터 주소 [RW][00:00:00:00:00:00]
    @details @ref _Sn_DHAR_는 SOCKETn의 목적지 하드웨어 주소를 설정하거나
   가져옵니다.\n
     - When @ref Sn_MR2_DHAM = 1 and @ref _Sn_MR_[3:0] != @ref Sn_MR_MACRAW
       The destination hardware address is set by @ref _Sn_DHAR_ without ARP
   processed by @ref Sn_CR_CONNECT, @ref Sn_CR_CONNECT6, @ref Sn_CR_SEND, and
   @ref Sn_CR_SEND6.\n Also, when SOCKETn is opened with @ref Sn_MR_UDP4 or @ref
   Sn_MR_UDP6 and @ref Sn_MR_MULTI is set, @ref _Sn_DHAR_ sets the Multicast
   Group Hardware address.
     - Others
       In TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6, and @ref
   Sn_MR_TCPD, \n
       @ref _Sn_DHAR_ gets the destination hardware address when @ref _Sn_SR_ is
   @ref SOCK_ESTABLISHED.
    @sa _Sn_MR_, _Sn_MR2_, _Sn_CR_, _Sn_SR_
    @sa getSn_DHAR(), setSn_DHAR(), getSn_MR(), setSn_MR(), getSn_MR2(),
   setSn_MR2(), getSn_CR(), setSn_CR(), getSn_SR()
*/
#define _Sn_DHAR_(N) (_W6100_IO_BASE_ + (0x0118 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn 목적지 IPv4 주소 레지스터 주소 [RW][0.0.0.0]
    @details @ref _Sn_DIPR_(= @ref _Sn_DIP4R_)는 SOCKETn의 목적지 IPv4 주소를
   설정하거나 가져옵니다. \n
     - In TCP mode such as @ref Sn_MR_TCP4, and @ref Sn_MR_TCPD
        - <b>TCP CLIENT</b> mode : It sets the IPv4 address of <b>TCP SERVER</b>
   before @ref Sn_CR_CONNECT is performed.
        - <b>TCP SERVER</b> mode : It gets the IPv4 address of <b>TCP CLIENT</b>
   when @ref _Sn_SR_ is @ref SOCK_ESTABLISHED.
     - In UDP(@ref Sn_MR_UDP4, @ref Sn_MR_UDPD) mode & IPRAW4(@ref Sn_MR_IPRAW4)
   mode It sets the destination IPv4 address before @ref Sn_CR_SEND is
   performed. \n When Sn_MR_MULTI = 1, It sets the multicast group IPv4 address.
    @sa _Sn_DIP4R_, _Sn_MR_, _Sn_CR_, _Sn_SR_
    @sa getSn_DIPR(), getSn_DIPR(), getSn_DIP4R(), getSn_DIP4R(), getSn_MR(),
   setSn_MR(), getSn_CR(), setSn_CR(), getSn_SR()
*/
#define _Sn_DIPR_(N) (_W6100_IO_BASE_ + (0x0120 << 8) + WIZCHIP_SREG_BLOCK(N))
#define _Sn_DIP4R_(N) (_Sn_DIPR_(N)) ///< Refer to @ref _Sn_DIPR_.

/**
    @brief SOCKETn 목적지 IPv6 주소 레지스터 주소 [RW][::]
    @details @ref _Sn_DIP6R_는 SOCKETn의 목적지 IPv6 주소를 설정하거나
   가져옵니다.
     - In TCP mode such as @ref Sn_MR_TCP6, and @ref Sn_MR_TCPD
        - <b>TCP CLIENT</b> mode : It sets the IPv6 address of <b>TCP SERVER</b>
   before @ref Sn_CR_CONNECT6 is performed.
        - <b>TCP SERVER</b> mode : It gets the IPv6 address of <b>TCP CLIENT</b>
   when @ref _Sn_SR_ is @ref SOCK_ESTABLISHED.
     - In UDP(@ref Sn_MR_UDP6, @ref Sn_MR_UDPD) mode & IPRAW4(@ref Sn_MR_IPRAW6)
   mode It sets the destination IPv6 address before @ref Sn_CR_SEND6 is
   performed.\n When Sn_MR_MULTI = 1, It sets the multicast group IPv6 address.
    @sa _Sn_MR_, _Sn_CR_, _Sn_SR_
    @sa getSn_DIP6R(), setSn_DIP6R(), getSn_MR(), setSn_MR(), getSn_CR(),
   setSn_CR(), getSn_SR()
*/
#define _Sn_DIP6R_(N) (_W6100_IO_BASE_ + (0x0130 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn 목적지 포트 레지스터 주소 [RW][0x0000]
    @details @ref _Sn_DPORTR_는 SOCKETn의 목적지 포트 번호를 설정하거나
   가져옵니다.
     - In TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6, and @ref Sn_MR_TCPD
        - <b>TCP CLIENT</b> mode : It sets the port number of <b>TCP SERVER</b>
   before @ref Sn_CR_CONNECT is performed.
        - <b>TCP SERVER</b> mode : It gets the port number of <b>TCP CLIENT</b>
   when @ref _Sn_SR_ is @ref SOCK_ESTABLISHED.
     - In UDP mode such as @ref Sn_MR_UDP4, @ref Sn_MR_UDP6, and @ref Sn_MR_UDPD
       It sets the destination port number before @ref Sn_CR_SEND is performed.
   \n When Sn_MR_MULTI = 1, It sets the multicast group group port number.

    @note It is valid SOCKETn is opened with @ref Sn_MR_TCP4,  @ref Sn_MR_TCP6,
   @ref Sn_MR_TCPD, @ref Sn_MR_UDP4, @ref Sn_MR_UDP4, and @ref Sn_MR_UDPD.
    @note It should be set before OPEN command is ordered.
    @sa _Sn_MR_, _Sn_CR_, _Sn_SR_
    @sa getSn_DPORTR(), getSn_DPORTR(), getSn_MR(), setSn_MR(), getSn_CR(),
   setSn_CR(), getSn_SR()
*/
#define _Sn_DPORTR_(N) (_W6100_IO_BASE_ + (0x0140 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn 모드 레지스터 2 주소 [R=W][0x00]
    @details @ref _Sn_MR2_는 @ref Sn_CR_OPEN을 수행하기 전에 @ref _Sn_MR_과 함께
   SOCKETn의 옵션을 설정합니다.\n Each bit of @ref _Sn_MR2_ is defined as the
   following. <table> <tr> <td>7 ~ 2   </td> <td>1   </td> <td>0   </td> </tr>
      <tr> <td>Reserved</td> <td>DHAM</td> <td>FARP</td> </tr>
    </table>
     - @ref Sn_MR2_DHAM : @ref Sn_MR2_DHAM_AUTO, @ref Sn_MR2_DHAM_MANUAL
     - @ref Sn_MR2_FARP
    @sa _Sn_MR_, _Sn_CR_
    @sa getSn_MR2(), setSn_MR2(), getSn_MR(), getSn_MR(), getSn_CR()
*/
#define _Sn_MR2_(N) (_W6100_IO_BASE_ + (0x0144 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn 재전송 시간 레지스터 주소 [R=W][0x0000]
    @details @ref _Sn_RTR_은 @ref _SLCR_에 의해 재전송될 패킷의 시간 초과 값을
   설정합니다.\n
    @note It should be set before @ref Sn_CR_OPEN is performed.\n
         It is initialized as @ref _RTR_ if you do not set it to none-zero
   value.
    @sa _RTR_, _Sn_CR_
    @sa getSn_RTR(), setSn_RTR(), getSn_CR(), setSn_CR()
*/
#define _Sn_RTR_(N) (_W6100_IO_BASE_ + (0x0180 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn 재전송 카운트 레지스터 주소 [R=W][0x00]
    @details @ref _Sn_RCR_은 @ref _SLCR_에 의해 재전송될 패킷의 재시도 횟수를
   설정합니다.\n
    @note It should be set before @ref Sn_CR_OPEN is performed.\n
         It is initialized as @ref _RTR_ if you do not set it to any none-zero
   value.
    @sa _RTR_, _Sn_CR_
    @sa getSn_RTR(), setSn_RTR(), getSn_CR(), setSn_CR()
*/
#define _Sn_RCR_(N) (_W6100_IO_BASE_ + (0x0184 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn Keep Alive 시간 레지스터 주소 [R=W][0x00]
    @details @ref _Sn_KPALVTR_은 KA(Keep Alive) 패킷의 자동 재전송 시간을
   설정합니다. \n If the destination can not respond to the KA packet during the
   time set by @ref _Sn_KPALVTR_,\n the connection is terminated, @ref
   Sn_IR_TIMEOUT is set and then @ref _Sn_SR_ is changed @ref SOCK_CLOSED.\n
            Before the time is expierd, if the destination sends a KA/ACK packet
   or any packet, the connection is still valid,\n
            @ref _Sn_SR_ remained at @ref SOCK_ESTABLISHED.
    @note It is valid only after sending data over 1 byte in TCP mode such as
   @ref Sn_MR_TCP4, @ref Sn_MR_TCP6, and @ref Sn_MR_TCPD.
    @note If it is set to 0, KA packet can be sent by @ref Sn_CR_SEND_KEEP.
    @sa Sn_CR_SEND_KEEP, Sn_IR_TIMEOUT, Sn_IRCLR, Sn_SR, Sn_MR
    @sa getSn_KPALVTR(), setSn_KPALVTR(), getSn_IR(), setSn_IRCLR(), getSn_SR(),
   getSn_MR(), setSn_MR()
*/
#define _Sn_KPALVTR_(N)                                                        \
  (_W6100_IO_BASE_ + (0x0188 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn TX 버퍼 크기 레지스터 주소 [R=W][0x02]
    @details @ref _Sn_TX_BSR_은 16KB TX 메모리 내에서 SOCKETn의 TX 버퍼 크기를
   설정합니다.\n It can be set only with 0,1,2,4,8, and 16K bytes.
    @note The 16KB TX memory is allocated as many as @ref _Sn_TX_BSR_
   sequentially from SOCKET0 to SOCKETn(Here, 0 <= n <= @ref _WIZCHIP_SOCK_NUM_
   - 1).\n The total sum of Sn_TX_BSR can not be exceed 16KB of TX memory. \n If
   the total size is exceeded, SOCKETn can't be normally sent data to a
   destination.
    @sa _Sn_RX_BSR_
    @sa getSn_TX_BSR(), setSn_TX_BSR(), getSn_TXBUF_SIZE(), setSn_TXBUF_SIZE(),
   getSn_TxMAX(), setSn_TX_BSR(), getSn_RX_BSR(), setSn_RX_BSR()
*/
#define _Sn_TX_BSR_(N) (_W6100_IO_BASE_ + (0x0200 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn TX 여유 버퍼 크기 레지스터 주소 [RO][0x0800]
    @details SOCKETn의 송신(TX) 버퍼에서 비어 있는(사용 가능한) 공간의 크기를
   읽어옵니다.
    @note Data should not be saved bigger than it because the data overwrites
   the previous saved data not to be sent yet.\n Therefore, Check it before
   saving the data to the SOCKETn TX buffer. \n If the data size is equal or
   smaller than it, transmit the data with @ref Sn_CR_SEND / @ref Sn_CR_SEND6
   after saving the data in SOCKETn TX buffer.\n If the data size is greater
   than it, transmit the data after dividing into it and saving in the SOCKETn
   TX buffer.
    @note \n
     - In TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6 and @ref Sn_MR_TCPD,
   \n It is automatically increased by the absolute difference between @ref
   _Sn_TX_WR_ and interanl TX ACK pointer.
     - In other mode \n
       It is automatically increased by the absolute difference between @ref
   _Sn_TX_WR_ and @ref _Sn_TX_RD_.
    @sa _Sn_RX_RSR_, _Sn_TX_WR_, _Sn_TX_RD_, _Sn_CR_
    @sa getSn_TX_FSR(), getSn_TX_WR(), getSn_TX_WR(), getSn_TX_RD(), getSn_CR(),
   setSn_CR()
*/
#define _Sn_TX_FSR_(N) (_W6100_IO_BASE_ + (0x0204 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKET TX 메모리 읽기 포인터 레지스터 주소[R][0x0000]
    @details @ref _Sn_TX_RD_는 @ref Sn_CR_SEND에 의해 전송될 데이터의 시작
   포인터를 가져옵니다. \n
            @ref Sn_CR_SEND / @ref Sn_CR_SEND6 starts to transmit the saved data
   from @ref _Sn_TX_RD_ to @ref _Sn_TX_WR_ in the SOCKETn TX Buffer,\n and when
   @ref Sn_IR_SENDOK is set, It is automatically increased to equal @ref
   _Sn_TX_WR_.
    @note It is initialized by @ref Sn_CR_OPEN, But, In TCP mode such as @ref
   Sn_MR_TCP4, @ref Sn_MR_TCP6, and @ref Sn_MR_TCPD,\n it is re-initialized when
   the TCP connection is completed.
    @note If it exceeds the maximum value 0xFFFF, (that is, it is greater than
   0x10000 and the carry bit occurs),\n then the carry bit is ignored and it
   automatically is updated with its the lower 16bits value.
    @sa _Sn_TX_WR_, _Sn_TX_FSR_, _Sn_CR_, _Sn_IR_, _Sn_IRCLR_, _Sn_MR_
    @sa getSn_TX_RD(), getSn_TX_WR(), setSn_TX_WR(), getSn_TX_FSR(), getSn_CR(),
   setSn_CR(), getSn_IR(), setSn_IRCLR(), getSn_MR(), setSn_MR()
*/
#define _Sn_TX_RD_(N) (_W6100_IO_BASE_ + (0x0208 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn TX 메모리 쓰기 포인터 레지스터 주소 [RW][0x0000]
    @details @ref _Sn_TX_WR_은 SOCKETn TX 버퍼에 저장될 데이터의 시작 포인터를
   가져옵니다, \n or sets the end pointer of data to be sent by @ref Sn_CR_SEND.
   \n If you have completed to save the data to be sent in the SOCKETn TX
   buffer, increase it as many as the saved size of data before @ref Sn_CR_SEND
   is performed.\n
            @ref Sn_CR_SEND starts to transmit the saved data from @ref
   _Sn_TX_RD_ to @ref _Sn_TX_WR_ in the SOCKETn TX Buffer, \n and when @ref
   Sn_IR_SENDOK is set, @ref _Sn_TX_RD_ is automatically increased to equal it.
    @note It is initialized by @ref Sn_CR_OPEN.\n
         But, In TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6, and @ref
   Sn_MR_TCPD,\n it is re-initialized when the TCP connection is completed.
    @note The size of data to be saved can't exceed @ref _Sn_TX_FSR_.
    @note If it exceeds the maximum value 0xFFFF(that is, it is greater than
   0x10000 and the carry bit occurs),\n then ignore the carry bit and update it
   with its lower 16bits value.
    @sa _Sn_TX_RD_, _Sn_TX_FSR_, _Sn_CR_, _Sn_IR_, _Sn_IRCLR_, _Sn_MR_
    @sa getSn_TX_WR(), setSn_TX_WR(), getSn_TX_RD(), getSn_TX_FSR(), getSn_CR(),
   setSn_CR(), getSn_IR(), setSn_IRCLR(), getSn_MR(), setSn_MR()
*/
#define _Sn_TX_WR_(N) (_W6100_IO_BASE_ + (0x020C << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn RX 버퍼 크기 레지스터 주소 [R=W][0x02]
    @details @ref _Sn_RX_BSR_은 16KB RX 메모리 내에서 SOCKETn의 RX 버퍼 크기를
   설정합니다.\n It can be set only with 0,1,2,4,8, and 16K bytes.
    @note The 16KB RX memory is allocated as many as @ref _Sn_RX_BSR_
   sequentially from SOCKET0 to SOCKETn(Here, 0 <= n <= @ref _WIZCHIP_SOCK_NUM_
   - 1).\n The total sum of @ref _Sn_RX_BSR_ can not be exceed 16KB of RX
   memory. \n If the total size is exceeded, SOCKETn can't be normally received
   data from a destination.
    @sa _Sn_RX_BSR_
    @sa getSn_TX_BSR(), setSn_TX_BSR(), getSn_RXBUF_SIZE(), setSn_RXBUF_SIZE(),
   getSn_RxMAX(), getSn_RX_BSR(), setSn_RX_BSR()
*/
#define _Sn_RX_BSR_(N) (_W6100_IO_BASE_ + (0x0220 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn RX 수신 크기 레지스터 주소 [RO][0x0000]
    @details SOCKETn의 수신(RX) 버퍼에 수신되어 있는 데이터의 크기를 읽어옵니다.
    @note The real received data size maybe smaller than it, \n
         because it maybe included the size of 'PACKET NFO' such like as \n
         the destination IP address, destination port number and data size of
   the received DATA PACKET.
    @note Do not read bigger data than @ref _Sn_RX_RSR_.
    @note It is automatically increased by the absolute difference between @ref
   _Sn_RX_WR_ and @ref _Sn_RX_RD_ \n after @ref Sn_CR_RECV is performed.
    @sa _Sn_RX_RSR_, _Sn_TX_WR_, _Sn_TX_RD_, _Sn_CR_, _Sn_TX_FSR_
    @sa getSn_RX_RSR(), getSn_TX_WR(), getSn_TX_WR(), getSn_CR(), setSn_CR(),
   getSn_TX_FSR()
*/
#define _Sn_RX_RSR_(N) (_W6100_IO_BASE_ + (0x0224 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKET RX 메모리 읽기 포인터 레지스터 주소[R][0x0000]
    @details @ref _Sn_RX_RD_는 SOCKETn RX 버퍼에 수신된 데이터의 시작 포인터를
   가져옵니다,\n or sets the end data pointer of the read completed data by @ref
   Sn_CR_RECV. \n You can read the received data from it to @ref _Sn_RX_WR_ in
   the SOCKET RX buffer.\n After completing to read data, you should increase it
   as many as the read size before @ref Sn_CR_RECV is performed.
    @note It is initialized by @ref Sn_CR_OPEN, But, In TCP mode such as @ref
   Sn_MR_TCP4, @ref Sn_MR_TCP6, and @ref Sn_MR_TCPD,\n it is re-initialized when
   the TCP connection is completed.
    @note If it exceeds the maximum value 0xFFFF, (that is, it is greater than
   0x10000 and the carry bit occurs),\n Ignore the carry bit and update with its
   the lower 16bits value.
    @sa _Sn_RX_WR_, _Sn_RX_RSR_, _Sn_CR_, _Sn_IR_, _Sn_IRCLR_, _Sn_MR_
    @sa getSn_RX_WR(), setSn_RX_RD(), getSn_RX_WR(), getSn_TX_FSR(), getSn_CR(),
   setSn_CR(), getSn_IR(), setSn_IRCLR(), getSn_MR(), setSn_MR()
*/
#define _Sn_RX_RD_(N) (_W6100_IO_BASE_ + (0x0228 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn TX 메모리 쓰기 포인터 레지스터 주소 [RW][0x0000]
    @details @ref _Sn_TX_WR_은 SOCKETn RX 버퍼에 수신 완료된 데이터의 끝
   포인터를 가져옵니다. \n Whenever a data has been completely received from a
   destination, \n It is automatically increased as many as the sum size of the
   received data and the 'PACKET INFO'. \n You can read the recevied data from
   @ref _Sn_RX_RD_ to it in the SOCKET RX buffer.
    @note It is initialized by @ref Sn_CR_OPEN. But, In TCP mode such as @ref
   Sn_MR_TCP4, @ref Sn_MR_TCP6, and @ref Sn_MR_TCPD,\n it is re-initialized when
   the TCP connection is completed.
    @note If it exceeds the maximum value 0xFFFF(that is, it is greater than
   0x10000 and the carry bit occurs),\n then ignore the carry bit and update it
   with its lower 16bits value.
    @sa _Sn_TX_RD_, _Sn_TX_FSR_, _Sn_CR_, _Sn_IR_, Sn_IRCLR_, _Sn_MR_
    @sa getSn_TX_WR(), setSn_TX_WR(), getSn_TX_RD(), getSn_TX_FSR(), getSn_CR(),
   setSn_CR(), getSn_IR(), setSn_IRCLR(), getSn_MR(), setSn_MR()
*/
#define _Sn_RX_WR_(N) (_W6100_IO_BASE_ + (0x022C << 8) + WIZCHIP_SREG_BLOCK(N))

/*----------------------------- W6100 Register values
 * -----------------------------*/

/* System Status Register Bit Definition */
/**
    @brief @ref _SYSR_의 CHIP 잠금 상태 비트.
    @details @ref SYSR_CHPL은 @ref _SYCR0_ 및 @ref _SYCR1_의 잠금 상태를
   나타냅니다.\n 1 : Lock \n 0 : unlock
    @note It is set by only @ref _CHPLCKR_.
    @sa _SYSR_, _CHPLCKR_, _SYCR0_, _SYCR1_
    @sa getSYSR(), getCHPLCKR(), setCHPLCKR(), CHIPLOCK(), CHIPUNLOCK(),
   setSYCR0(), setSYCR1()
*/
#define SYSR_CHPL (1 << 7)

/**
    @brief @ref _SYSR_의 NET 잠금 상태 비트.
    @details @ref SYSR_NETL은 다음과 같은 네트워크 정보 레지스터의 잠금 상태를
   나타냅니다.
            @ref _SHAR_, @ref _GAR_, @ref _SUBR_, @ref _SIPR_, @ref _LLAR_, @ref
   _GUAR_, and @ref _SUB6R_. \n 1 : Lock \n 0 : unlock
    @note It is set by only @ref _NETLCKR_.
    @note @ref _GA6R_ can be accessed regardless of @ref SYSR_NETL.
    @sa _SYSR_, _NETLCKR_, _SHAR_, _GAR_, _SUBR_, _SIPR_, _LLAR_, _GUAR_,
   _SUB6R_
    @sa getSYSR(), getNETLCKR(), setNETLCKR(), NETLOCK(), NETUNLOCK(),\n
       getSHAR(), setSHAR(), getGAR(), setGAR(), getSUBR(), getSIR(), setSIPR(),
   \n getLLAR(), setLLAR(), getGUAR(),setGUAR(), getSUB6R(), setSUB6R()
*/
#define SYSR_NETL (1 << 6)

/**
    @brief @ref _SYSR_의 PHY 잠금 상태 비트. @ref _PHYLCKR_ 참조.
    @details @ref SYSR_PHYL은 @ref _PHYCR0_ 및 _PHYCR1_의 잠금 상태를
   나타냅니다.\n 1 : Lock \n 0 : unlock
    @note It is set by only @ref _PHYLCKR_.
    @sa _SYSR_, _PHYCLKR_, _PHYCR0_, _PHYCR1_
    @sa getSYSR(), getPHYLCKR(), setPHYLCKR(), setPHYCR0(), getPHYCR1(),
   setPHYCR1()
*/
#define SYSR_PHYL (1 << 5)

/**
    @brief @ref _SYSR_의 병렬 버스 모드(Parallel Bus Mode) 비트
    @details @ref SYSR_IND는 @ref _WIZCHIP_ 핀 MODE[3:0] == "010X"일 때
   설정됩니다. It indicates to use the parallel BUS mode.
    @sa _SYSR_, _WIZCHIP_IO_MODE_BUS_
    @sa getSYSR()
*/
#define SYSR_IND (1 << 5)

/**
    @brief @ref _SYSR_의 SPI 인터페이스 모드 비트.
    @details @ref SYSR_SPI는 @ref _WIZCHIP_ 핀 MODE[3:0] == "000X"일 때
   설정됩니다. It indicates to use the SPI mode.
    @sa _SYSR_, _WIZCHIP_IO_MODE_SPI_
    @sa getSYSR()
*/
#define SYSR_SPI (1 << 0)

/* System Config Register Bit Definition */
/**
    @brief @ref _SYCR0_의 RST 비트
    @details @ref SYCR0_RST는 @ref _WIZCHIP_을 소프트(softly) 리셋합니다. \n
            0 : Soft reset \n
            1 : Normal operation
    @note It can be set only when @ref SYSR_CHPL = 1.
    @sa _SYSR0_, _CHPLCKR_, _SYSR_, SYSR_CHPL
    @sa setSYCR0(), setCHPLCKR(), getCHPLCKR(), CHIPLOCK(), CHIPUNLOCK(),
   getSYSR()
*/
#define SYCR0_RST (0x00)

/**
    @brief @ref _SYCR1_의 IEN 비트.
    @details @ref SYCR1_IEN은 전역적으로 @ref _WIZCHIP_의 인터럽트를
   활성화하거나 비활성화합니다,\n regardless of the related interrupt mask
   registers such as @ref _IMR_, @ref _SIMR_, @ref _SLIMR_, and @ref _Sn_IMR_.\n
            1 : Enable  \n
            0 : Disable
    @sa _SYCR1_, _IR_, _SIR_, _SLIR_, _Sn_IR_, _IRCLR_,  _SLIRCLR_, _Sn_IRCLR_
    @sa getSYCR1(), setSYCR1(), getIR(), getSIR(), getSLIR(), getSn_IR(),
   setIRCLR(), setSLIRCLR(), setSn_IRCLR()
*/
#define SYCR1_IEN (1 << 7)

/**
    @brief @ref _SYCR1_의 시스템 클럭 선택 마스크 비트.
    @details @ref SYCR1_CLKSEL은 시스템 클럭을 100MHz 또는 25MHz로 선택합니다.
   \n The masked bit values are as following.
              - @ref SYCR1_CLKSEL_25M
              - @ref SYCR1_CLKSEL_100M
    @note It can be set only when @ref SYSR_CHPL = 1.
    @note The system clock is automatically changed to 25MHz while the reset of
   @ref _WIZCHIP_ H/W reset, the Ethernet PHY H/W reset and power down. \n On
   the other hand, the system clock is set by @ref SYCR1_CLKSEL during normal
   operating.
    @sa _SYCR1_, _SYSR_, _CHPLCKR_, SYSL_CHPL, PHYCR1_RST, PHYCR1_PWDN
    @sa getSYCR1(), setSYCR1(), getSYSR(), getCHPLCKR(), setCHIPLCKR(),
   CHIPLOCK(), CHIPUNLOCK(), getPHYCR1(), setPHYCR1()
*/
#define SYCR1_CLKSEL (1 << 0)

/**
    @brief 시스템 클럭 - 25MHz
    @details @ref SYCR1_CLKSEL_25M은 시스템 클럭을 25MHz로 선택합니다.
    @note It can be set only when @ref SYSR_CHPL = 1.
    @sa _SYCR1_, SYCR1_CLKSEL, SYCR1_CLKSEL_100M
    @sa getSYCR1(), setSYCR1(), getSYSR(), getCHPLCKR(), setCHIPLCKR(),
   CHIPLOCK(), CHIPUNLOCK()
*/
#define SYCR1_CLKSEL_25M 1

/**
    @brief 시스템 클럭 - 100MHz
    @details @ref SYCR1_CLKSEL_100M은 시스템 클럭을 100MHz로 선택합니다.
    @note It can be set only when @ref SYSR_CHPL = 1.
    @sa _SYCR1_, SYCR1_CLKSEL, SYCR1_CLKSEL_25M
    @sa getSYCR1(), setSYCR1(), getSYSR(), getCHPLCKR(), setCHIPLCKR(),
   CHIPLOCK(), CHIPUNLOCK()
*/
#define SYCR1_CLKSEL_100M 0

/* Interrupt Register Bit Definition */
/**
    @brief @ref _IR_의 WOL 비트
    @details @ref IR_WOL은 @ref _WIZCHIP_이 WOL의 매직 패킷을 수신할 때
   설정됩니다.
    @sa _IR_, _IRCLR_, _IMR_
    @sa getIR(), setIRCLR(), getIMR(), setIMR()
*/
#define IR_WOL (1 << 7)

/**
    @brief @ref _IR_의 UNR6 비트
    @details @ref IR_UNR6은 @ref _WIZCHIP_이 ICMPv6의 Unreachable 메시지를
   수신할 때 설정됩니다.
    @sa _IR_, _IRCLR_, _IMR_
    @sa getIR(), setIRCLR(), getIMR(), setIMR()
*/
#define IR_UNR6 (1 << 4)

/**
    @brief @ref _IR_의 IPCONF 비트
    @details @ref IR_IPCONF는 @ref _WIZCHIP_이 @ref _SIPR_과 동일한 IPv4 주소를
   가진 ARP 응답을 수신할 때 설정됩니다.
    @sa _IR_, _IRCLR_, _IMR_
    @sa getIR(), setIRCLR(), getIMR(), setIMR()
*/
#define IR_IPCONF (1 << 2)

/**
    @brief @ref _IR_의 UNR4 비트
    @details @ref IR_UNR4는 @ref _WIZCHIP_이 ICMPv4의 Unreachable 메시지를
   수신할 때 설정됩니다.
    @sa _IR_, _IRCLR_, _IMR_
    @sa getIR(), setIRCLR(), getIMR(), setIMR()
*/
#define IR_UNR4 (1 << 1)

/**
    @brief @ref _IR_의 PTERM 비트
    @details @ref IR_PTERM은 @ref _WIZCHIP_이 PPP 종료 패킷을 수신할 때
   설정됩니다.
    @sa _IR_, _IRCLR_, _IMR_
    @sa getIR(), setIRCLR(), getIMR(), setIMR()
*/
#define IR_PTERM (1 << 0)

/* SOCKET Interrupt Register Bit Definition */
/**
    @brief @ref _SIR_의 N번째 INT 비트
    @details @ref SIR_INT(N)은 @ref _Sn_IR_(N)이 0이 아닐 때 설정됩니다.
    @sa _SIR_, _Sn_IRCLR_, _SIMR_
    @sa getSIR(), setSn_IRCLR(), getSIMR()
*/
#define SIR_INT(N) (1 << N)

/* SOCKET-less Interrupt Register Bit Definition */
/**
    @brief @ref _SLIR_의 TOUT 비트
    @details @ref SLIR_TOUT은 @ref _SLCR_ 수행 후 시간 초과가 발생할 때
   설정됩니다.
    @sa _SLIR_, _SLIRCLR_, _SLCR_
    @sa getSLIR(), setSLIRCLR(), getSLCR(), setSLCR()
*/
#define SLIR_TOUT (1 << 7)

/**
    @brief @ref _SLIR_의 ARP4 비트
    @details @ref SLCR_ARP4 수행 후 ARP-reply를 성공적으로 수신하면 @ref
   SLIR_ARP4가 설정됩니다. \n and the destination hardware address can be
   checked by @ref _SLDHAR_. \n Otherwise, @ref SLIR_TOUT is set.
    @sa _SLIR_, _SLIRCLR_, _SLCR_, _SLDIPR_, _SLDIP4R_, _SLDHAR_, SLIR_TOUT
    @sa getSLIR(), setSLIRCLR(), getSLCR(), setSLCR(), getSLDIPR(), setSLDIPR(),
   getSLDIP4R(), setSLDIP4R(), getSLDHAR()
*/
#define SLIR_ARP4 (1 << 6)

/**
    @brief @ref _SLIR_의 PING4 비트
    @details @ref SLCR_PING4 수행 후 PING-reply를 성공적으로 수신하면 @ref
   SLIR_PING4가 설정됩니다 \n and the destination hardware address can be
   checked by @ref _SLDHAR_ like as @ref SLIR_ARP4.\n Otherwise, @ref SLIR_TOUT
   is set.
    @sa _SLIR_, _SLIRCLR_, _SLCR_, _SLDIPR_, _SLDIP4R_, _SLDHAR_, SLIR_TOUT
    @sa getSLIR(), setSLIRCLR(), getSLCR(), setSLCR(), getSLDIPR(), setSLDIPR(),
   getSLDIP4R(), setSLDIP4R(), getSLDHAR()
*/
#define SLIR_PING4 (1 << 5)

/**
    @brief @ref _SLIR_의 ARP6 비트
    @details @ref SLCR_ARP6 수행 후 ARP6-reply를 성공적으로 수신하면 @ref
   SLIR_ARP6이 설정됩니다. \n and the destination hardware address can be
   checked by @ref _SLDHAR_. \n Otherwise, @ref SLIR_TOUT is set.
    @sa _SLIR_, _SLIRCLR_, _SLCR_, _SLDIP6R_, _SLDHAR_, SLIR_TOUT
    @sa getSLIR(), setSLIRCLR(), getSLCR(), setSLCR(), getSLDIP6R(),
   setSLDIP6R(), getSLDHAR()
*/
#define SLIR_ARP6 (1 << 4)

/**
    @brief @ref _SLIR_의 PING6 비트
    @details @ref SLCR_PING6 수행 후 PING-reply를 성공적으로 수신하면 @ref
   SLIR_PING6가 설정됩니다 \n and the destination hardware address can be
   checked by @ref _SLDHAR_ like as @ref SLIR_ARP6. \n Otherwise, @ref SLIR_TOUT
   is set.
    @sa _SLIR_, _SLIRCLR_, _SLCR_, _SLDIP6R_, _SLDHAR_, SLIR_TOUT
    @sa getSLIR(), setSLIRCLR(), getSLCR(), setSLCR(), getSLDIP6R(),
   setSLDIP6R(), getSLDHAR()
*/
#define SLIR_PING6 (1 << 3)

/**
    @brief @ref _SLIR_의 NS 비트
    @details @ref SLCR_NS 수행 후 ICMPv6 NA를 수신하면 @ref SLIR_NS가
   설정됩니다. \n Its set means IPv6 address such like as @ref _LLAR_ or @ref
   _GUAR_ is conflict. \n If @ref SLIR_TOUT is set, You can use @ref _SLDIP6R_
   to @ref _LLAR_ or @ref _GUAR_.
    @note It is used for IPv6 state-less address auto-configuration(SLAAC).
    @sa _SLIR_, _SLIRCLR_, _SLCR_, _SLDIP6R_, SLIR_TOUT, _LLAR_, _GUAR_
    @sa getSLIR(), setSLIRCLR(), getSLCR(), setSLCR(), getSLDIP6R(),
   setSLDIP6R(), getLLAR(), setLLAR(), getGAUR(), setGUAR()
*/
#define SLIR_NS (1 << 2)

/**
    @brief @ref _SLIR_의 RS 비트
    @details @ref SLCR_RS를 수행한 후 ICMPv6 RA를 성공적으로 수신하면 @ref
   SLIR_RS가 설정됩니다 \n and the prefix length, the prefix flag, the valid
   life time, the preferred life time and the prefix address of RA option
   message \n can be checked by @ref _PLR_, @ref _PFR_, @ref _VLTR_, @ref _PLTR_
   and @ref _PAR_, respectively.\n Otherwise, @ref SLIR_TOUT is set.
    @bug Only when the first received RA option is the source link-layer
   address(0x01) and the second is prefix information(0x03),\n and the prefix
   information is in the order of prefix length, prefix flag, valid lifetime,
   default lifetime and prefix address,\n
        @ref _PLR_, @ref _PFR_, @ref _VLTR_, @ref _PLTR_ and @ref _PAR_ is
   correctly set.\n Other case, these registers are not valid.\n\n To solve this
   errata,\n You should use a IPRAW6 mode SOCKETn opened with Sn_MR_IPRAW6 and
   set the @ref _Sn_PNR_ to ICMPv6 number 58.\n This SOCKETn can be received a
   RA message or other ICMPv6 message, and a ICMPv6 message can be selectively
   filtered out by @ref _ICMP6BLKR_.\n For more details, Refer to "IPv6
   Auto-configuration" document.

    @sa _SLIR_, _SLIRCLR_, _SLCR_, SLIR_TOUT
    @sa getSLIR(), setSLIRCLR(), getSLCR(), setSLCR()
*/
#define SLIR_RS (1 << 1)

/**
    @brief ICMPv6 RA 수신 인터럽트
    @details @ref SLIR_RA는 @ref SLCR_RS 없이 라우터로부터 RA를 수신할 때
   설정됩니다.\n Like as @ref SLIR_RS, a RA option message can be checked by
   @ref _PLR_, @ref _PFR_, @ref _VLTR_, @ref _PLTR_ and @ref _PAR_.\n
    @bug Only when RA options are received in the order of prefix length, prefix
   flag, valid lifetime, default lifetime and prefix address,\n
        @ref _PLR_, @ref _PFR_, @ref _VLTR_, @ref _PLTR_ and @ref _PAR_ is
   correctly set. \n Other case, these registers are not valid.\n\n To solve
   this errata, You should use a IPRAW6 mode SOCKETn opened with @ref
   Sn_MR_IPRAW6 and set the @ref _Sn_PNR_ to ICMPv6 number 58.\n This SOCKETn
   can be received a RA message or other ICMPv6 message, and a ICMPv6 message
   can be selectively filtered out by @ref _ICMP6BLKR_.\n For more details,
   Refer to "IPv6 Auto-configuration" document.

    @sa _SLIR_, _SLIRCLR_
    @sa getSLIR(), setSLIRCLR(),
*/
#define SLIR_RA (1 << 0)

/* SOCKET-less & SOCKETn  Prefer Source IPv6 Address Register Bit Definition */
/**
    @brief @ref _SLCR_ 또는 @ref _Sn_CR_로 전송하는 패킷의 소스 IPv6 주소를
   자동으로 선택합니다
    @details 소스 IPv6 주소는 @ref _SLDIP6R_ 또는 @ref _Sn_DIP6R_의 IPv6 주소
   유형에 따라 달라집니다.\n If @ref _Sn_DIP6R_ is a link-local, the source IPv6
   address is selected to @ref _LLAR_.\n Otherwise, the source IPv6 address is
   selected to @ref _GUAR_.
    @sa _SLPSR_, _Sn_PSR_, _SLCR_, _Sn_CR_, _SLDI6PR_, _Sn_DIP6R_, _LLAR_,
   _GUAR_
    @sa getSLPSR(), setSLPSR(), getSn_PSR(), setSn_PSR(), getSLCR(), setSLCR(),
   getSn_CR(), setSn_CR(), \n getSLDIP6R(), setSLDIP6R(), getSn_DIP6R(),
   setSn_DIP6R(), getLLAR(), setLLAR(), getGUAR(), setGUAR()
*/
#define PSR_AUTO (0x00)

/**
    @brief @ref _SLCR_ 또는 @ref _Sn_CR_로 전송하는 패킷의 소스 IP 주소를 @ref
   _LLAR_로 선택합니다
    @details @ref _SLDIP6R_ 또는 @ref _Sn_DIP6R_의 목적지 IPv6 주소 유형에
   상관없이, 소스 IP는 @ref _LLAR_로 선택됩니다.
    @sa _SLPSR_, _Sn_PSR_, _SLCR_, _Sn_CR_, _SLDIP6R_, _Sn_DIP6R_, _LLAR_,
   _GUAR_
    @sa getSLPSR(), setSLPSR(), getSn_PSR(), setSn_PSR(), getSLCR(), setSLCR(),
   getSn_CR(), setSn_CR(), \n getSLDIP6R(), setSLDIP6R(), getSn_DIP6R(),
   setSn_DIP6R(), getLLAR(), setLLAR(), getGUAR(), setGUAR()
*/
#define PSR_LLA (0x02)

/**
    @brief @ref _SLCR_ 또는 @ref _Sn_CR_로 전송하는 패킷의 소스 IP 주소를 @ref
   _GUAR_로 선택합니다
    @details @ref _SLDIP6R_ 또는 @ref _Sn_DIP6R_의 목적지 IPv6 주소 유형에
   상관없이, 소스 IP는 @ref _GUAR_로 선택됩니다.
    @sa _SLPSR_, _Sn_PSR_, _SLCR_, _Sn_CR_, _SLDIP6R_, _Sn_DIP6R_, _LLAR_,
   _GUAR_
    @sa getSLPSR(), setSLPSR(), getSn_PSR(), setSn_PSR(), getSLCR(), setSLCR(),
   getSn_CR(), setSn_CR(), \n getSLDIP6R(), setSLDIP6R(), getSn_DIP6R(),
   setSn_DIP6R(), getLLAR(), setLLAR(), getGUAR(), setGUAR()
*/
#define PSR_GUA (0x03)

/* SOCKET-less Command Register Bit Definition */
/**
    @brief IPv4 ARP 명령어
    @details SOCKETn 없이 @ref _SLDIP4R_로 IPv4 ARP 요청 메시지를 보냅니다. \n
            The results can be ether @ref SLIR_TOUT or @ref SLIR_ARP4.\n
            If the result is @ref SLIR_ARP4, It is success to receive the reply
   from @ref _SLDIP4R_. \n You can check the destination hardware address thru
   @ref _SLDHAR_. \n
            @ref SLIR_TOUT is set when it is no reply from @ref _SLDIP4R_ \n
            while both the time set by @ref _SLRTR_ and the retry count set by
   @ref _SLRCR_ are expired.
    @sa _SLCR_, _SLDIPR_, _SLDIP4R_ _SLDHAR_, _SLIR_, _SLIRCLR_, _SLRTR_,
   _SLRCR_
    @sa getSLCR(), setSLCR(), getSLDIPR(), setSLDIPR(), getSLDIP4R(),
   setSLDIP4R(), getSLDHAR(), getSLIR(), setSLIRCR(), getSLRTR(), setSLRTR(),
   getSLRCR(), setSLRTR()
*/
#define SLCR_ARP4 (1 << 6)

/**
    @brief IPv4 PING 명령어
    @details SOCKETn 없이 @ref _SLDIP4R_로 IPv4 PING 요청 메시지를 보냅니다.\n
            The results can be ether @ref SLIR_TOUT or @ref SLIR_PING4.\n
            If the result is @ref SLIR_PING4, It is success to receive the reply
   from @ref _SLDIP4R_. \n Also such like as @ref SLCR_ARP4, You can check the
   destination hardware address thru @ref _SLDHAR_.\n
            @ref SLIR_TOUT is set when it is no reply from @ref _SLDIP4R_ \n
            while both the time set by @ref _SLRTR_ and the retry count set by
   @ref _SLRCR_ are expired.
    @sa _SLCR_, _SLDIPR_, _SLDIP4R_, _SLDHAR_, _SLIR_, _SLIRCLR_, _SLRTR_,
   _SLRCR_
    @sa getSLCR(), setSLCR(), getSLDIPR(), setSLDIPR(), getSLDIP4R(),
   setSLDIP4R(), getSLDHAR(), getSLIR(), setSLIRCR(), getSLRTR(), setSLRTR(),
   getSLRCR(), setSLRTR()
*/
#define SLCR_PING4 (1 << 5)

/**
    @brief IPv6 ARP 명령어
    @details SOCKETn 없이 @ref _SLDIP6R_로 IPv6 ARP 요청 메시지를 보냅니다. \n
            The results can be either @ref SLIR_TOUT or @ref SLIR_ARP6.
            If the result is @ref SLIR_ARP6, It is success to receive the reply
   from @ref _SLDIP6R_. \n You can check the destination hardware address thru
   @ref _SLDHAR_.\n
            @ref SLIR_TOUT is set when it is no reply from @ref _SLDIP6R_
            while both the time set by @ref _SLRTR_ and the retry count set by
   @ref _SLRCR_ are expired.
    @sa _SLCR_, _SLDIP6R_, _SLDHAR_, _SLIR_, _SLIRCLR_, _SLRTR_, _SLRCR_
    @sa getSLCR(), setSLCR(), getSLDIP6R(), setSLDIP6R(), getSLDHAR(),
   getSLIR(), setSLIRCR(), getSLRTR(), setSLRTR(), getSLRCR(), setSLRTR()
*/
#define SLCR_ARP6 (1 << 4)

/**
    @brief IPv6 PING 명령어
    @details SOCKET 없이 @ref _SLDIP6R_로 IPv6 PING 요청 메시지를 보냅니다. \n
            The results can be either @ref SLIR_TOUT or @ref SLIR_PING6.\n
            If the result is @ref SLIR_PING6, It is success to receive the reply
   from @ref _SLDIP6R_.\n Also such like as @ref SLCR_ARP6, You can check the
   destination hardware address thru @ref _SLDHAR_.\n
            @ref SLIR_TOUT is set when it is no reply from @ref _SLDIP6R_ \n
            while both the time set by @ref _SLRTR_ and the retry count set by
   @ref _SLRCR_ are expired.
    @sa _SLCR_, _SLDIP6R_, _SLDHAR_, _SLIR_, _SLIRCLR_, _SLRTR_, _SLRCR_
    @sa getSLCR(), setSLCR(), getSLDIP6R(), setSLDIP6R(), getSLDHAR(),
   getSLIR(), setSLIRCR(), getSLRTR(), setSLRTR(), getSLRCR(), setSLRTR()
*/
#define SLCR_PING6 (1 << 3)

/**
    @brief IPv6 DAD(Duplicate Address Detection) NS 명령어
    @details SOCKET 없이 @ref _LLAR_ 또는 @ref _GUAR_로 사용할 주소로 설정된
   @ref _SLDIP6R_로 DAD용 NS 메시지를 보냅니다.\n The result can be ether @ref
   SLIR_TOUT and @ref SLIR_NS.\n If @ref SLIR_TOUT is set then you can use @ref
   _SLDIP6R_ to @ref _LLAR_ or @ref _GUAR_,\n else if @ref SLIR_NS is set then
   you can not use _SLDIP6R_ to @ref _LLAR_ or @ref _GUAR_.\n That means the
   IPv6 Address are Conflict.
    @sa _SLCR_, _SLDIP6R_, _SLIR_, _SLIRCLR_, _SLRTR_, _SLRCR_, _LLAR_, _GUAR_
    @sa getSLCR(), setSLCR(), getSLDIP6R(), setSLDIP6R(), getSLIR(),
   setSLIRCR(), getSLRTR(), setSLRTR(), getSLRCR(), setSLRTR(), \n getLLAR(),
   setLLAR(), getGUAR(), setGUAR()
*/
#define SLCR_NS (1 << 2)

/**
    @brief IPv6 자동 구성(Auto-configuration) RS 명령어
    @details SOCKET 없이 IPv6 자동 구성을 위해 모든 라우터(All-router)에게 RS
   메시지를 보냅니다.\n The result can be ether @ref SLIR_RS or @ref SLIR_TOUT.
   \n If the result is @ref SLIR_RS, You can some information of router such as
   a prefix length, a Prefix flag, a valid life time, \n a preferred life time,
   and a prefix address respectively thru @ref _PLR_, @ref _PFR_, @ref _VLTR_,
   @ref _PLTR_, and @ref _PAR_.\n
           @ref SLIR_TOUT is set when it is no reply from a IPv6 router \n
           while both the time set by @ref _SLRTR_ and the retry count set by
   @ref _SLRCR_ are expired.
    @sa _SLCR_, _SLIR_, _SLIRCLR_, _SLRTR_, _SLRCR_, _PLR_, _PFR_, _VLTR_,
   _PLTR_, _PAR_.
    @sa getSLCR(), setSLCR(), getSLIR(), setSLIRCR(), getSLRTR(), setSLRTR(),
   getSLRCR(), setSLRTR(), \n getPLR(), getPFR(), getVLTR(), getPLTR(),
   getPAR().
*/
#define SLCR_RS (1 << 1)

/**
    @brief IPv6 Unsolicited NA 명령어
    @details @ref _LLAR_, @ref _GUAR_, @ref _SHAR_와 같은 네트워크 정보를
   업데이트하기 위해 IPv6 Unsolicited NA 메시지를 전송합니다.\n The result is
   none.\n When @ref _SLPSR_ = @ref PSR_GUA, It can send the GUA unsolicited NA
   message.\n When @ref _SLPSR_ = Others, It can send the LLA unsolicited NA
   message.
    @sa _SLCR_, _SLIR_, _SLIRCLR_, _SLPSR_
    @sa getSLCR(), setSLCR(), getSLIR(), setSLIRCR(), getSLPFR(), setSLPFR()
*/
#define SLCR_UNA (1 << 0)

/* PHY Status Register Bit Definition */
/**
    @brief CAB 마스크 비트
    @details @ref PHYSR_CAB는 @ref _PHYSR_의 CAB 비트를 마스킹합니다.\n
            The masked bit values are as following. \n
    - @ref PHYSR_CAB_OFF
    - @ref PHYSR_CAB_ON
    @sa getPHYSR()
*/
#define PHYSR_CAB (1 << 7)

/* PHY Status Register Bit Definition */
/**
    @brief 이더넷 케이블 분리됨(Off)
    @details @ref PHYSR_CAB_OFF는 케이블이 이더넷 PHY에서 분리되었음을
   나타냅니다.
    @sa _PHYSR_, PHYSR_CAB, PHYSR_CAB_ON
    @sa getPHYSR()
*/
#define PHYSR_CAB_OFF (1 << 7)

/**
    @brief 이더넷 케이블 연결됨(On)
    @details @ref PHYSR_CAB_OFF는 케이블이 이더넷 PHY에 연결되었음을 나타냅니다.
    @sa _PHYSR_, PHYSR_CAB, PHYSR_CAB_OFF
    @sa getPHYSR()
*/
#define PHYSR_CAB_ON (0 << 7)

/**
    @brief @ref _PHYSR_의 마스크 비트
    @details @ref PHYSR_MODE는 @ref _PHYSR_의 MODE 비트를 마스킹합니다.\n
            The masked bits values are as following. \n
              - @ref PHYSR_MODE_AUTO
              - @ref PHYSR_MODE_100F
              - @ref PHYSR_MODE_100H
              - @ref PHYSR_MODE_10F
              - @ref PHYSR_MODE_10H
    @sa _PHYSR_, _PHYCR0_, _PHYCLKR_, _SYSR_, SYSR_NETL
    @sa getPHYSR(), setPHYCR0(), setPHYLCKR(), PHYLOCK(), PHYUNLOCK(), getSYSR()
*/
#define PHYSR_MODE (7 << 3)

/**
    @brief PHY 모드 - 자동(AUTO)
    @details @ref PHYSR_MODE_AUTO는 이더넷 PHY가 자동 협상 모드로 동작함을
   나타냅니다.
    @sa _PHYSR_, PHYSR_MODE, PHYSR_MODE_100F, PHYSR_MODE_100H, PHYSR_MODE_10F,
   PHYSR_MODE_10H
    @sa getPHYSR()
*/
#define PHYSR_MODE_AUTO (0 << 3)

/**
    @brief PHY 모드 - 100F
    @details @ref PHYSR_MODE_100F는 이더넷 PHY가 100M 전이중 모드로 동작함을
   나타냅니다.
    @sa _PHYSR_, PHYSR_MODE, PHYSR_MODE_AUTO, PHYSR_MODE_100H, PHYSR_MODE_10F,
   PHYSR_MODE_10H
    @sa getPHYSR()
*/
#define PHYSR_MODE_100F (4 << 3)

/**
    @brief PHY 모드 - 100H
    @details @ref PHYSR_MODE_100H는 이더넷 PHY가 100M 반이중 모드로 동작함을
   나타냅니다.
    @sa _PHYSR_, PHYSR_MODE, PHYSR_MODE_AUTO, PHYSR_MODE_100F, PHYSR_MODE_10F,
   PHYSR_MODE_10H
    @sa getPHYSR()
*/
#define PHYSR_MODE_100H (5 << 3)

/**
    @brief PHY 모드 - 10F
    @details @ref PHYSR_MODE_10F는 이더넷 PHY가 10M 전이중 모드로 동작함을
   나타냅니다.
    @sa _PHYSR_, PHYSR_MODE, PHYSR_MODE_AUTO, PHYSR_MODE_100F, PHYSR_MODE_100H,
   PHYSR_MODE_10H
    @sa getPHYSR()
*/
#define PHYSR_MODE_10F (6 << 3)

/**
    @brief PHY 모드 - 10H
    @details @ref PHYSR_MODE_10H는 이더넷 PHY가 10M 반이중 모드로 동작함을
   나타냅니다.
    @sa _PHYSR_, PHYSR_MODE, PHYSR_MODE_AUTO, PHYSR_MODE_100F, PHYSR_MODE_100H,
   PHYSR_MODE_10F
    @sa getPHYSR()
*/
#define PHYSR_MODE_10H (7 << 3)

/**
    @brief @ref _PHYSR_의 DPX 마스크 비트
    @details @ref PHYSR_DPX는 @ref _PHYSR_의 DPX 비트를 마스킹합니다. \n
            The masked bit values are as following. \n
              - @ref PHYSR_DPX_HALF
              - @ref PHYSR_DPX_FULL
    @sa _PHYSR_, _PHYCR0_, _PHYCLKR_, _SYSR_, SYSR_NETL
    @sa getPHYSR(), setPHYCR0(), setPHYLCKR(), PHYLOCK(), PHYUNLOCK(), getSYSR()
*/
#define PHYSR_DPX (1 << 2)

/**
    @brief PHY 이중(Duplex) 모드 - 반이중(HALF)
    @details @ref PHYSR_DPX_HALF는 이더넷 PHY가 반이중 모드로 동작함을
   나타냅니다.
    @sa _PHYSR_, PHYSR_DPX_FULL
    @sa getPHYSR()
*/
#define PHYSR_DPX_HALF (1 << 2)

/**
    @brief PHY 이중(Duplex) 모드 - 전이중(FULL)
    @details @ref PHYSR_DPX_FULL은 이더넷 PHY가 전이중 모드로 동작함을
   나타냅니다.
    @sa _PHYSR_, PHYSR_DPX_HALF
    @sa getPHYSR()
*/
#define PHYSR_DPX_FULL (0 << 2)

/**
    @brief @ref _PHYSR_의 SPD 마스크 비트
    @details @ref PHYSR_SPD는 @ref _PHYSR_의 SPD 비트를 마스킹합니다. 마스킹된
   비트 값은 다음과 같습니다. \n
     - @ref PHYSR_SPD_10M
     - @ref PHYSR_SPD_100M
    @sa _PHYSR_, _PHYCR0_, _PHYCLKR_, _SYSR_, SYSR_NETL
    @sa getPHYSR(), setPHYCR0(), setPHYLCKR(), PHYLOCK(), PHYUNLOCK(), getSYSR()
*/
#define PHYSR_SPD (1 << 1)

/**
    @brief PHY 속도 - 10M
    @details @ref PHYSR_SPD_10M은 이더넷 PHY가 10Mbps 속도로 동작함을
   나타냅니다.
    @sa _PHYSR_, PHYSR_SPD_100M
    @sa getPHYSR()
*/
#define PHYSR_SPD_10M (1 << 1)

/**
    @brief PHY 속도 - 100M
    @details @ref PHYSR_SPD_100M은 이더넷 PHY가 100Mbps 속도로 동작함을
   나타냅니다.
    @sa _PHYSR_, PHYSR_SPD_10M
    @sa getPHYSR()
*/
#define PHYSR_SPD_100M (0 << 1)

/**
    @brief @ref _PHYSR_의 LNK 마스크 비트
    @details @ref PHYSR_LNK는 @ref _PHYSR_의 LNK 비트를 마스킹합니다. 마스킹된
   비트 값은 다음과 같습니다. \n
     - @ref PHYSR_LNK_DOWN
     - @ref PHYSR_LNK_UP
    @sa _PHYSR_, _PHYCR0_, _PHYCLKR_, _SYSR_, SYSR_NETL
    @sa getPHYSR(), setPHYCR0(), setPHYLCKR(), PHYLOCK(), PHYUNLOCK(), getSYSR()
*/
#define PHYSR_LNK (1 << 0)

/**
    @brief PHY 링크 - 업(Up)
    @details @ref PHYSR_LNK_UP은 이더넷 PHY의 링크가 성공적으로 설정되었음을
   나타냅니다.\n
    @sa _PHYSR_, PHYSR_LNK_DOWN
    @sa getPHYSR()
*/
#define PHYSR_LNK_UP (1 << 0)

/**
    @brief PHY 링크 - 다운(Down)
    @details @ref PHYSR_LNK_DOWN은 이더넷 PHY의 링크가 아직 설정되지 않았음을
   나타냅니다.\n
    @sa _PHYSR_, PHYSR_LNK_UP
    @sa getPHYSR()
*/
#define PHYSR_LNK_DOWN (0 << 0)

/**
    @brief @ref _PHYRAR_에 지정된 이더넷 PHY 레지스터에서 값을 읽습니다.\n
          The read value can be checked by _PHYDOR_.
    @sa _PHYACR_, _PHYDOR_, _PHYRAR_, _PHYDIR_, PHYACR_WRITE
    @sa getPHYACR(), setPHYACR(), getPHYDOR(), getPHYRAR(), setPHYRAR(),
   setPHYDIR()
*/
#define PHYACR_READ (0x02)

/**
    @brief @ref _PHYRAR_에 지정된 이더넷 PHY 레지스터에 @ref _PHYDIR_을
   기록합니다.
    @sa _PHYACR_, _PHYDIR_, _PHYRAR_, _PHYDOR_, PHYACR_READ
    @sa getPHYACR(), setPHYACR(), setPHYDIR(), getPHYRAR(), setPHYRAR(),
   getPHYDOR()
*/
#define PHYACR_WRITE (0x01)

/**
    @brief PHY의 MDC 클럭은 시스템 클럭을 32로 나눈 값입니다
    @sa _PHYDIVR_
    @sa getPHYDIVR(), setPHYDIVR()
*/
#define PHYDIVR_32 (0x00)

/**
    @brief PHY의 MDC 클럭은 시스템 클럭을 64로 나눈 값입니다
    @sa _PHYDIVR_
    @sa getPHYDIVR(), setPHYDIVR()
*/
#define PHYDIVR_64 (0x01)

/**
    @brief PHY의 MDC 클럭은 시스템 클럭을 128로 나눈 값입니다
    @sa _PHYDIVR_
    @sa getPHYDIVR(), setPHYDIVR()
*/
#define PHYDIVR_128 (0xFF)

/* PHY Command Register Bit Definition */
/**
    @brief PHY 동작 모드 - 자동 협상(Auto Negotiation)
    @details @ref PHYCR0_AUTO는 이더넷 PHY를 자동 협상 모드로 동작하도록
   설정합니다.\n The Ethernet PHY can operate on auto-negotiation after @ref
   PHYCR1_RST is performed, \n and the result of @ref PHYCR0_AUTO can be checked
   by @ref PHYSR_SPD, @ref PHYSR_DPX, and @ref PHYSR_LNK.
    @note It can be set only when @ref SYSR_PHYL = 1.
    @sa _PHYCR0_, _PHYLCKR_, _SYSR_, SYSR_PHYL, _PHYSR_, PHYCR0_100F,
   PHYCR0_100H, PHYCR0_10F, PHYCR0_10H, BMCR_ANE
    @sa setPHYCR0(), getPHYLCKR(), setPHYLCKR(), PHYLOCK(), PHYUNLOCK(),
   getSYSR(), getPHYSR(), getPHYRAR_BMCR(), setPHYRAR_BMCR()
*/
#define PHYCR0_AUTO (0x00)

/**
    @brief PHY 동작 모드 - 100F
    @details @ref PHYCR0_100F는 이더넷 PHY를 100F로 동작하도록 설정합니다\n
            The Ethernet PHY can operate on 100F after @ref PHYCR1_RST is
   performed,\n and the result of @ref PHYCR0_100F can be checked by @ref
   PHYSR_SPD, @ref PHYSR_DPX, and @ref PHYSR_LNK.
    @note It can be set only when @ref SYSR_PHYL = 1.
    @sa _PHYCR0_, _PHYLCKR_, _SYSR_, SYSR_PHYL, _PHYSR_, BMCR_SPD, BMCR_DPX
    @sa setPHYCR0(), getPHYLCKR(), setPHYLCKR(), PHYLOCK(), PHYUNLOCK(),
   getSYSR(), getPHYSR(), getPHYRAR_BMCR(), setPHYRAR_BMCR()
*/
#define PHYCR0_100F (0x04)

/**
    @brief PHY 동작 모드 - 100H
    @details @ref PHYCR0_100H는 이더넷 PHY를 100H로 동작하도록 설정합니다 \n
            The Ethernet PHY can operate 100H after @ref PHYCR1_RST is
   performed, \n and the result of @ref PHYCR0_100H can be checked by @ref
   PHYSR_SPD, @ref PHYSR_DPX, and @ref PHYSR_LNK.
    @note It can be set only when @ref SYSR_PHYL = 1.
    @sa _PHYCR0_, _PHYLCKR_, _SYSR_, SYSR_PHYL, _PHYSR_, PHYCR0_AUTO,
   PHYCR0_100H, PHYCR0_10F, PHYCR0_10H, BMCR_SPD, BMCR_DPX
    @sa setPHYCR0(), getPHYLCKR(), setPHYLCKR(), PHYLOCK(), PHYUNLOCK(),
   getSYSR(), getPHYSR(), getPHYRAR_BMCR(), setPHYRAR_BMCR()
*/
#define PHYCR0_100H (0x05)

/**
    @brief PHY 동작 모드 - 10F
    @details @ref PHYCR0_10F는 이더넷 PHY를 10F로 동작하도록 설정합니다 \n
            The Ethernet PHY can operate 10H after @ref PHYCR1_RST is performed,
   \n and the result of @ref PHYCR0_10F can be checked by @ref PHYSR_SPD, @ref
   PHYSR_DPX, and @ref PHYSR_LNK.
    @note It can be set only when @ref SYSR_PHYL = 1.
    @sa _PHYCR0_, _PHYLCKR_, _SYSR_, SYSR_PHYL, _PHYSR_, PHYCR0_AUTO,
   PHYCR0_100F, PHYCR0_100H, PHYCR0_10H, BMCR_SPD, BMCR_DPX
    @sa setPHYCR0(), getPHYLCKR(), setPHYLCKR(), PHYLOCK(), PHYUNLOCK(),
   getSYSR(), getPHYSR(), getPHYRAR_BMCR(), setPHYRAR_BMCR()
*/
#define PHYCR0_10F (0x06)

/**
    @brief PHY 동작 모드 - 10H
    @details @ref PHYCR0_10H는 이더넷 PHY를 10H로 동작하도록 설정합니다 \n
            The Ethernet PHY can operate 10H after @ref PHYCR1_RST is performed,
   \n and the result of @ref PHYCR0_10H can be checked by @ref PHYSR_SPD, @ref
   PHYSR_DPX, and @ref PHYSR_LNK.
    @note It can be set only when @ref SYSR_PHYL = 1.
    @sa _PHYCR0_, _PHYLCKR_, _SYSR_, SYSR_PHYL, _PHYSR_, PHYCR0_AUTO,
   PHYCR0_100F, PHYCR0_100H, PHYCR0_10H, BMCR_SPD, BMCR_DPX
    @sa setPHYCR0(), getPHYLCKR(), setPHYLCKR(), PHYLOCK(), PHYUNLOCK(),
   getSYSR(), getPHYSR(), getPHYRAR_BMCR(), setPHYRAR_BMCR()
*/
#define PHYCR0_10H (0x07)

/**
    @brief PHY 기능 - 전원 차단(Power Down)
    @details @ref PHYCR1_PWDN은 이더넷 PHY를 전원 차단 모드로 진입시킵니다. \n
            0 : Normal mode \n
            1 : Power down mode
    @note The system clock changes to 25MHz in power down mode, and depends on
   @ref SYCR1_CLKSEL in normal mode.
    @note It can be set only when @ref SYSR_PHYL = 1.
    @sa _PHYCR1_, SYCR1_CLKSEL, BMCR_PWDN
    @sa getPHYCR1(), setPHYCR1(), getSYCR1(), setSYCR1(), getPHYRAR_BMCR(),
   setPHYRAR_BMCR()
*/
#define PHYCR1_PWDN (1 << 5)

/**
    @brief PHY 기능 - 10Base-TE 모드
    @details @ref PHYCR1_TE는 이더넷 PHY의 동작을 10base-Te로 설정합니다.
    @note It is valid only when @ref PHYSR_MODE = @ref PHYSR_MODE_AUTO.
    @note It can be set only when @ref SYSR_PHYL = 1.
    @sa _PHYCR1_
    @sa getPHYCR1(), setPHYCR1()
*/
#define PHYCR1_TE (1 << 3)

/**
    @brief PHY 기능 - HW 리셋
    @details @ref PHYCR1_RST는 이더넷 PHY를 하드웨어적으로 리셋합니다, \n
            and it is automatically cleared after the H/W reset and it
   takes 60.3ms to stabilize.\n 0 : Normal mode \n 1 : H/W Reset \n
    @note The system clock changes to 25MHz in H/W reset time, and depends on
   @ref SYCR1_CLKSEL in normal mode.
    @note It can be set only when @ref SYSR_PHYL = 1.
    @sa _PHYCR1_, BMCR_RST
    @sa getPHYCR1(), setPHYCR1(), getPHYRAR_BMCR(), setPHYRAR_BMCR()
*/
#define PHYCR1_RST (1 << 0)

/* IPv4 Network Mode Register Bit Definition */
/**
    @brief UDP Unreachable 패킷 차단
    @details @ref NETxMR_UNRB는 상대방에게 ICMPv4 또는 ICMPv6 Unreachable
   메시지를 보내는 것을 차단할 수 있습니다.
    @sa _NET4MR_, _NET6MR_
    @sa getNET4MR(), setNET4MR(), getNET6MR(), setNET6MR()
*/
#define NETxMR_UNRB (1 << 3)

/**
    @brief PING ARP 요청
    @details @ref NETxMR_PARP는 ICMPv4 또는 ICMPv6 PING 응답을 보내기 전에 ARP
   요청을 보낼 수 있습니다.
    @sa _NET4MR_, _NET6MR_
    @sa getNET4MR(), setNET4MR(), getNET6MR(), setNET6MR()
*/
#define NETxMR_PARP (1 << 2)

/**
    @brief TCP 리셋(Reset) 패킷 차단
    @details @ref NETxMR_RSTB는 IPv4 또는 IPv6 기반의 TCP RST 패킷 전송을 차단할
   수 있습니다. \n when there is no SOCKET n opened with a listen port.
    @sa _NET4MR_, _NET6MR_
    @sa getNET4MR(), setNET4MR(), getNET6MR(), setNET6MR()
*/
#define NETxMR_RSTB (1 << 1)

/**
    @brief PING 응답 차단
    @details @ref NETxMR_PB는 상대방에게 ICMPv4 또는 ICMPv6 PING 응답을 보내는
   것을 차단할 수 있습니다.
    @sa _NET4MR_, _NET6MR_
    @sa getNET4MR(), setNET4MR(), getNET6MR(), setNET6MR()
*/
#define NETxMR_PB (1 << 0)

/* Network Mode Register Bit Definition */
/**
    @brief All-node 멀티캐스팅 PING 응답 차단
    @details @ref NETMR_ANB는 all-node 멀티캐스트 주소로 PING을 요청한
   상대방에게 IPv6 PING 응답을 보내는 것을 차단할 수 있습니다.
    @sa _NETMR_
    @sa getNETMR(), setNETMR()
*/
#define NETMR_ANB (1 << 5)

/**
    @brief Solicited Multicasting PING 응답 차단
    @details @ref NETMR_M6B는 자신만의 Solicited 멀티캐스트 주소로 PING을 요청한
   상대방에게 IPv6 PING 응답을 전송하는 것을 차단할 수 있습니다.
    @sa _NETMR_
    @sa getNETMR(), setNETMR()
*/
#define NETMR_M6B (1 << 4)

/**
    @brief Wake On LAN 모드
    @details @ref NETMR_WOL은 WOL의 매직 패킷을 수신할 수 있습니다.
    @sa _NETMR_
    @sa getNETMR(), setNETMR()
*/
#define NETMR_WOL (1 << 2)

/**
    @brief IPv6 패킷 차단
    @details @ref NETMR_IP6B는 모든 IPv6 패킷 수신을 차단할 수 있습니다.
    @sa _NETMR_
    @sa getNETMR(), setNETMR()
*/
#define NETMR_IP6B (1 << 1)

/**
    @brief IPv4 패킷 차단
    @details @ref NETMR_IP4B는 모든 IPv4 패킷 수신을 차단할 수 있습니다.
    @sa _NETMR_
    @sa getNETMR(), setNETMR()
*/
#define NETMR_IP4B (1 << 0)

/**
    @brief 목적지 하드웨어 주소 선택
    @details @ref NETMR2_DHAS는 @ref _NETMR2_의 DHAS 비트를 마스킹합니다. \n
            The masked bit values are as following.
              - @ref NETMR2_DHAS_ARP
              - @ref NETMR2_DHAS_ETH

    @note It is useful when the destination hardware address of Ethernet frame
   is different from the target address of ARP.
    @sa _NETMR2_
    @sa getNETMR2(), setNETMR2()
*/
#define NETMR2_DHAS (1 << 7)

/**
    @brief 목적지 하드웨어 주소 선택 - ARP
    @details @ref NETMR2_DHAS_ARP는 ARP 응답 패킷의 타겟 주소를 목적지 하드웨어
   주소로 선택합니다.
    @sa _NETMR2_, NETMR2_DHAS_ETH
    @sa getNETMR2(), setNETMR2()
*/
#define NETMR2_DHAS_ARP (1 << 7)

/**
    @brief 목적지 하드웨어 주소 선택 - Ethernet Frame
    @details @ref NETMR2_DHAS_ETH는 이더넷 프레임의 목적지 주소를 목적지
   하드웨어 주소로 선택합니다.
    @sa _NETMR2_, NETMR2_DHAS_ARP
    @sa getNETMR2(), setNETMR2()
*/
#define NETMR2_DHAS_ETH (0 << 7)

/**
    @brief PPPoE 모드
    @details @ref NETMR2_PPPoE는 PPPoE 모드를 활성화합니다 \n
            0 : Disable \n
            1 : Enable
    @note For enabling a PPPoE mode, some information such like as _PTMR_,
   _PHAR_, _PSIDR_, and _PMRUR_ are needed. \n To get these information, You can
   use a SOCKET0 opened with @ref Sn_MR_MACRAW.
    @sa _NETMR2_, _PTMR_, _PHAR_, _PSIDR_, _PMRUR_, Sn_MR_MACRAW
    @sa getNETMR2(), setNETMR2()
*/
#define NETMR2_PPPoE (1 << 0)

/* ICMPv6 Block Register Bit Definition */
/**
    @brief ICMPv6 PING 차단
    @details @ref ICMP6BLKR_PING6은 상대방의 핑(ping) 요청을 차단할 수 있습니다.
    @sa _IMCP6BLKR_, NETxMR_PB
    @sa getICMP6BLKR(), setICMP6BLKR(), getNET6MR(), setNET6MR()
*/
#define ICMP6BLKR_PING6 (1 << 4)

/**
    @brief ICMPv6 MLD 차단
    @details @ref ICMP6BLKR_MLD는 MLD(Multicast Listener Discovery) 쿼리를
   차단할 수 있습니다.
    @sa _ICMP6BLKR_
    @sa getICMP6BLKR(), setICMP6BLKR()
*/
#define ICMP6BLKR_MLD (1 << 3)

/**
    @brief ICMPv6 RA 차단
    @details @ref ICMP6BLKR_RA는 라우터의 RA 패킷을 차단할 수 있습니다.
    @sa _ICMP6BLKR_
    @sa getICMP6BLKR(), setICMP6BLKR()
*/
#define ICMP6BLKR_RA (1 << 2)

/**
    @brief ICMPv6 NA 차단
    @details @ref ICMP6BLKR_NA는 상대방의 NA(Neighbor Advertisement)를 차단할 수
   있습니다.
    @sa _ICMP6BLKR_
    @sa getICMP6BLKR(), setICMP6BLKR()
*/
#define ICMP6BLKR_NA (1 << 1)

/**
    @brief ICMPv6 NS 차단
    @details @ref ICMP6BLKR_NS는 상대방의 NS(Neighbor Solicitation)를 차단할 수
   있습니다.
    @sa _ICMP6BLKR_
    @sa getICMP6BLKR(), setICMP6BLKR()
*/
#define ICMP6BLKR_NS (1 << 0)

/* Sn_MR values */
/**
    @brief UDP 멀티캐스팅
    @details  @ref Sn_MR_MULTI는 UDP 모드 SOCKETn에서 멀티캐스트 그룹의
   멀티캐스트 패킷을 활성화합니다. \n To use multicasting, @ref _Sn_DIPR_, @ref
   _Sn_DIP6R_, & @ref _Sn_DPORTR_ should be respectively set with \n the
   multicast group IPv4, IPv6 address & port number before @ref Sn_CR_OPEN. \n
              0 : Disable Multicasting \n
              1 : Enable Multicasting \n
    @note It is valid only in UDP mode such like as @ref Sn_MR_UDP4, @ref
   Sn_MR_UDP6, and @ref Sn_MR_UDPD.
    @sa _Sn_MR_, _Sn_DIPR_, _Sn_DIP6R_, _Sn_DPORTR_
    @sa getSn_MR(), setSn_MR(), getSn_DIPR(), setSn_DIPR(), getSn_DIP6R(),
   setSn_DIP6R(), getSn_DPORTR().
*/
#define Sn_MR_MULTI (1 << 7)

/**
    @brief MAC 필터
    @details @ref Sn_MR_MF는 브로드캐스팅, 멀티캐스팅 및 자신에게 전송된 패킷을
   제외한 다른 패킷을 필터링합니다.\n 0 : Disable MAC 필터ing \n 1 : Enable MAC
   필터ing \n
    @note It is valid only in MACRAW SOCKET0 opened with @ref Sn_MR_MACRAW \n
    @note If you want to implement a hybrid TCP/IP stack, \n
         It is recommended that @ref Sn_MR_MF enable for reducing host overhead
   to process the all received packets.
    @sa _Sn_MR_, Sn_MR_MULTI
    @sa getSn_MR(), setSn_MR()
*/
#define Sn_MR_MF (1 << 7)

/**
    @brief 브로드캐스팅 패킷 차단
    @details @ref Sn_MR_BRDB는 MACRAW SOCKET0 또는 UDP 모드의 SOCKETn에서
   브로드캐스팅 패킷을 차단할 수 있습니다. \n 0 : Disable Broadcast Blocking \n
            1 : Enable Broadcast Blocking \n
    @note It is valid only in MACRAW 모드 such as @ref Sn_MR_MACRAW, or in UDP
   mode such as @ref Sn_MR_UDP4, @ref Sn_MR_UDP6, and @ref Sn_MR_UDPD.
    @sa _Sn_MR_, Sn_MR_FPSH
    @sa getSn_MR(), setSn_MR()
*/
#define Sn_MR_BRDB (1 << 6)

/**
    @brief 강제 PUSH 플래그
    @details @ref Sn_MR_FPSH가 설정되면 PSH 플래그가 설정된 모든 TCP DATA 패킷이
   @ref Sn_CR_SEND를 통해 전송될 수 있습니다. \n When @ref Sn_MR_FPSH is not
   set, the PSH flag is set only in the last DATA packet among the DATA packets
   transmitted by @ref Sn_CR_SEND. \n 0 : No Force PSH flag \n 1 : Force PSH
   flag \n
    @note It is valid only in TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6,
   and @ref Sn_MR_TCPD.
    @sa _Sn_MR_, Sn_CR_SEND, Sn_MR_BRDB
    @sa getSn_MR(), setSn_MR(), getSn_CR(), setSn_CR()
*/
#define Sn_MR_FPSH (1 << 6)

/**
    @brief 지연(Delayed) Ack 없음
    @details @ref Sn_MR_FPSH가 설정되면 상대방으로부터 DATA 패킷을 수신하자마자
   지연 없이 ACK 패킷을 전송합니다.\n Otherwise, It sends the ACK packet after
   waiting the time set by @ref _Sn_RTR_. \n 0 : Delayed ACK \n 1 : No Delayed
   ACK \n
    @note It is valid only in TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6,
   and @ref Sn_MR_TCP6.
    @note Regardless of @ref Sn_MR_ND, It sends the ACK packet when SOCKETn
   window size is less than MSS after @ref Sn_CR_RECV.
    @sa _Sn_MR_, _Sn_RTR_, Sn_CR_RECV, Sn_MR_MC, Sn_MR_SMB, Sn_MR_MMB
    @sa getSn_MR(), setSn_MR(), getSn_RTR(), setSn_RTR(), getSn_CR(), setSn_CR()
*/
#define Sn_MR_ND (1 << 5)

/**
    @brief IPv4 멀티캐스팅을 위한 IGMP 버전
    @details @ref Sn_MR_MC는 IGMP 버전을 결정합니다. \n
            0 : IGMPv2 \n
            1 : IGMPv1 \n
    @note It is valid only when @ref Sn_MR_MULTI = '1' and UDP mode is @ref
   Sn_MR_UDP4.
    @note IGMP packet can be automatically sent to the multicast group by @ref
   Sn_CR_OPEN.
    @note @ref _WIZCHIP_ doesn't not support IGMP version 3.
    @sa _Sn_MR_, Sn_MR_MULTI, Sn_MR_ND, Sn_MR_SMB, Sn_MR_MMB
    @sa getSn_MR(), setSn_MR()
*/
#define Sn_MR_MC (1 << 5)

/**
    @brief Solicited 멀티캐스트 차단
    @details @ref Sn_MR_SMB는 자신만의 Solicited 멀티캐스트 주소를 가진 수신
   패킷을 차단할 수 있습니다. \n 0 : Unblock a solicited multicast packet \n 1 :
   Block a solicited multicast packet \n
    @note It is valid only when UDP mode is @ref Sn_MR_UDP6 or @ref Sn_MR_UDPD.
    @sa _Sn_MR_,  Sn_MR_ND, Sn_MR_MC, Sn_MR_MMB
    @sa getSn_MR(), setSn_MR()
*/
#define Sn_MR_SMB (1 << 5)

/**
    @brief UDP4 멀티캐스트 차단
    @details @ref Sn_MR_MMB는 SOCKET0이 @ref Sn_MR_MACRAW로 열리고 @ref
   Sn_MR_MF가 설정되었을 때 UDP4 멀티캐스트 패킷을 차단할 수 있습니다.\n 0 :
   Unblock a UDP multicast packet with IPv4 address \n 1 : Block a UDP multicast
   packet with IPv4 address \n
    @note It is valid only in MACRAW SOCKET0 with Sn_MR_MF = '1'.
    @sa _Sn_MR_, Sn_MR_MMB6, Sn_MR_ND, Sn_MR_MC, Sn_MR_SMB
    @sa getSn_MR(), setSn_MR()
*/
#define Sn_MR_MMB (1 << 5)
#define Sn_MR_MMB4 (Sn_MR_MMB) /// Refer to @ref Sn_MR_MMB.

/**
    @brief 유니캐스트(Unicast) 차단
    @details @ref Sn_MR_UNIB는 유니캐스트 패킷을 차단할 수 있습니다. \n
            0 : Unblock a UDP unicast packet \n
            1 : Block a UDP unicast packet \n
    @note It is valid only when SOCKETn is opened with UDP mode such as @ref
   Sn_MR_UDP4, @ref Sn_MR_UDP6 and @ref Sn_MR_UDPD, and @ref Sn_MR_MULTI is set.
    @sa _Sn_MR_, Sn_MR_MULTIL, Sn_MR_MMB6
    @sa getSn_MR(), setSn_MR()
*/
#define Sn_MR_UNIB (1 << 4)

/**
    @brief UDP6 멀티캐스트 차단
    @details @ref Sn_MR_MMB6는 UDP6 멀티캐스트 패킷을 차단할 수 있습니다. \n
            0 : Unblock a UDP multicast packet with IPv6 address \n
            1 : Block a UDP multicast packet with IPv6 address \n
    @note  It is valid only when SOCKET0 is opend with @ref Sn_MR_MACRAW and
   @ref Sn_MR_MF is set.
    @sa _Sn_MR_, Sn_MR_MMB
    @sa getSn_MR(), setSn_MR()
*/
#define Sn_MR_MMB6 (1 << 4)

/**
    @brief SOCKETn 닫힘(Closed)
    @details @ref Sn_MR_CLOSE는 아직 열리지 않았습니다.\n
            It is the default mode when @ref _WIZCHIP_ is reset.
    @sa _Sn_MR_, _Sn_CR_, _Sn_SR_
    @sa getSn_MR(), setSn_MR(), getSn_CR(), setSn_CR(), getSn_SR()
*/
#define Sn_MR_CLOSE (0x00)

/**
    @brief IPv4 TCP 모드
    @details @ref Sn_MR_TCP(= @ref Sn_MR_TCP4)는 SOCKETn을 TCP4 모드로
   설정합니다. \n It should be set before @ref Sn_CR_OPEN is performed.\n After
   @ref Sn_CR_OPEN, SOCKETn is opend as TCP4 mode and @ref _Sn_SR_ is changed
   from @ref SOCK_CLOSED to @ref SOCK_INIT.
    @note In oder to connect a peer, You should use @ref Sn_CR_CONNECT as
   command and @ref _Sn_DIPR_ as destination.
    @note In order to send data, You should use @ref Sn_CR_SEND.
    @sa _Sn_MR_, _Sn_CR_, _Sn_SR_, Sn_MR_TCP6, Sn_MR_TCPD
    @sa getSn_MR(), setSn_MR(), getSn_CR(), setSn_CR(), getSn_SR()
*/
#define Sn_MR_TCP (0x01)
#define Sn_MR_TCP4 (Sn_MR_TCP) ///< Refer to @ref Sn_MR_TCP.

/**
    @brief IPv4 UDP 모드
    @details @ref Sn_MR_UDP(= @ref Sn_MR_UDP4)는 SOCKETn을 UDP4 모드로
   설정합니다.\n It should be set before @ref Sn_CR_OPEN is performed. \n After
   @ref Sn_CR_OPEN, SOCKETn is opend as UDP4 mode and @ref _Sn_SR_ is changed
   from @ref SOCK_CLOSED to @ref SOCK_UDP.
    @note In order to send data, You should use @ref Sn_CR_SEND as command and
   @ref _Sn_DIPR_ as destination.
    @sa _Sn_MR_, _Sn_CR_, _Sn_SR_, Sn_MR_UDP6, Sn_MR_UDPD
    @sa getSn_MR(), setSn_MR(), getSn_CR(), setSn_CR(), getSn_SR()
*/
#define Sn_MR_UDP (0x02)
#define Sn_MR_UDP4 (Sn_MR_UDP) ///< Refer to @ref Sn_MR_UDP

/**
    @brief IPv4 RAW 모드
    @details @ref Sn_MR_IPRAW(= @ref Sn_MR_IPRAW4)는 SOCKETn을 IPRAW4 모드로
   설정합니다. \n It should be set before @ref Sn_CR_OPEN is performed.\n After
   @ref Sn_CR_OPEN, SOCKETn is opend as IPRAW4 mode and @ref _Sn_SR_ is changed
   from @ref SOCK_CLOSED to @ref SOCK_IPRAW(= @ref SOCK_IPRAW4).
    @note In order to send data, You should use @ref Sn_CR_SEND as command and
   @ref _Sn_DIPR_ as destination.
    @sa _Sn_MR_, _Sn_CR_, _Sn_SR_, Sn_MR_IPRAW6
    @sa getSn_MR(), setSn_MR(), getSn_CR(), setSn_CR(), getSn_SR()
*/
#define Sn_MR_IPRAW (0x03)
#define Sn_MR_IPRAW4 (Sn_MR_IPRAW) ///< Refer to @ref Sn_MR_IPRAW.

/**
    @brief MACRAW 모드
    @details @ref Sn_MR_MACRAW sets only SOCKET0 to MACRAW 모드.\n
            It should be set before @ref Sn_CR_OPEN is performed.\n
            After @ref Sn_CR_OPEN, SOCKET0 is opend as MACRAW 모드 and @ref
   _Sn_SR_ is changed from @ref SOCK_CLOSED to @ref SOCK_MACRAW.
    @note In order to send data, You should use @ref Sn_CR_SEND.
    @sa _Sn_MR_, _Sn_CR_, _Sn_SR_
    @sa getSn_MR(), setSn_MR(), getSn_CR(), setSn_CR(), getSn_SR()
*/
#define Sn_MR_MACRAW (0x07)

/**
    @brief IPv6 TCP 모드
    @details @ref Sn_MR_TCP6은 SOCKETn을 TCP6 모드로 설정합니다. \n
            It should be set before @ref Sn_CR_OPEN is performed.\n
            After @ref Sn_CR_OPEN, SOCKETn is opend as TCP6 mode and @ref
   _Sn_SR_ is changed from @ref SOCK_CLOSED to @ref SOCK_INIT.
    @note In oder to connect a peer, You should use @ref Sn_CR_CONNECT6 as
   command and @ref _Sn_DIP6R_ as destination.
    @note In order to send data, You should use @ref Sn_CR_SEND, not @ref
   Sn_CR_SEND6.
    @sa _Sn_MR_, _Sn_CR_, _Sn_SR_, Sn_MR_TCP4, Sn_MR_TCPD
    @sa getSn_MR(), setSn_MR(), getSn_CR(), setSn_CR(), getSn_SR()
*/
#define Sn_MR_TCP6 (0x09)

/**
    @brief IPv6 UDP 모드
    @details @ref Sn_MR_UDP6은 SOCKETn을 UDP6 모드로 설정합니다.\n
            It should be set before @ref Sn_CR_OPEN is performed.\n
            After @ref Sn_CR_OPEN, SOCKETn is opend as UDP6 mode and @ref
   _Sn_SR_ is changed from @ref SOCK_CLOSED to @ref SOCK_UDP.
    @note In order to send data, You should use @ref Sn_CR_SEND6 as command and
   @ref _Sn_DIP6R_ as destination.
    @sa _Sn_MR_, _Sn_CR_, _Sn_SR_, Sn_MR_UDP4, Sn_MR_UDPD
    @sa getSn_MR(), setSn_MR(), getSn_CR(), setSn_CR(), getSn_SR()
*/
#define Sn_MR_UDP6 (0x0A) // 0x1010

/**
    @brief IPv6 RAW 모드
    @details @ref Sn_MR_IPRAW6은 SOCKETn을 IPRAW6 모드로 설정합니다.\n
            It should be set before @ref Sn_CR_OPEN is performed.\n
            After @ref Sn_CR_OPEN, SOCKETn is opened as IPRAW6 mode and @ref
   _Sn_SR_ is changed from @ref SOCK_CLOSED to @ref SOCK_IPRAW6.
    @note In order to send data, You should use @ref Sn_CR_SEND6 as command and
   @ref _Sn_DIP6R_ as destination.
    @sa _Sn_MR_, _Sn_CR_, _Sn_SR_, Sn_MR_IPRAW4
    @sa getSn_MR(), setSn_MR(), getSn_CR(), setSn_CR(), getSn_SR()
*/
#define Sn_MR_IPRAW6 (0x0B) // 0x1011

/**
    @brief IPv4 및 IPv6 TCP 모드 모두 (TCP 듀얼 모드)
    @details @ref Sn_MR_TCPD는 SOCKETn을 TCP4 및 TCP6 모드로 모두 설정합니다. \n
            It should be set before @ref Sn_CR_OPEN is performed.\n
            After @ref Sn_CR_OPEN, SOCKETn is opened as TCP Dual mode and @ref
   _Sn_SR_ is changed from @ref SOCK_CLOSED to @ref SOCK_INIT.\n The real mode
   of TCP dual SOCKETn is decided when the connection with a peer is
   established.\n
     - In SOCKETn is operated as <b>TCP SERVER</b> mode
       If the connection request client have a IPv4 address, \n
       TCP dual SOCKETn is changed to TCP4 mode and a destination IP address can
   be checked thru @ref _Sn_DIPR_, \n else if the client have a IPv6 address, \n
       TCP dual SOCKETn is changed to IPv6 mode and destination IP address can
   be checked by thru @ref _Sn_DIP6R_.
     - In SOCKETn is operated as <b>TCP CLIENT</b> mode,
       If the IP address type of destination to connect is IPv4, \n
       the destination IP address should be set to @ref _Sn_DIPR_ and try to
   connect by @ref Sn_CR_CONNECT, \n else if the type is IPv6, \n the
   destination IP address should be set to @ref _Sn_DIP6R_ and try to connect by
   @ref Sn_CR_CONNECT6. \n

    @note In <b>TCP SERVER</b> mode, You can check the IP type of the client
   with @ref Sn_ESR_TCPM.
    @note If the connected client have a IPv6 address, You can check whether the
   address is LLA or GAU, thru @ref Sn_ESR_IP6T
    @sa _Sn_MR_, _Sn_CR_, _Sn_SR_, _Sn_ESR_TCPM_, Sn_MR_TCP4, Sn_MR_TCP6
    @sa getSn_MR(), setSn_MR(), getSn_CR(), setSn_CR(), getSn_SR(), getSn_ESR()
*/
#define Sn_MR_TCPD (0x0D)

/**
    @brief UDP 듀얼 모드
    @details @ref Sn_MR_UDPD는 SOCKETn을 UDP4 및 UDP6 모드로 모두 설정합니다. \n
            It should be set before @ref Sn_CR_OPEN is performed. \n
            After @ref Sn_CR_OPEN, SOCKETn is opened as UDP dual mode \n
            and @ref _Sn_SR_ is changed from @ref SOCK_CLOSED to @ref SOCK_UDP.
    @note In order to send data, \n
         You can use both @ref Sn_CR_SEND and @ref Sn_CR_SEND6 as command and
   both @ref _Sn_DIPR_ and @ref _Sn_DIP6R_ as destination.
    @note You can know the destination IP address type is whether IPv6 or IPv4
   thru @ref getsockopt() with @ref SO_PACKINFO.
    @sa _Sn_MR_, _Sn_CR_, _Sn_SR_, Sn_MR_UDP6, Sn_MR_UDP4
    @sa getSn_MR(), setSn_MR(), getSn_CR(), setSn_CR(), getSn_SR()
*/
#define Sn_MR_UDPD (0x0E)

/* SOCKETn  Command Register BIt Definition */
/**
    @brief SOCKETn 초기화 또는 열기(Open).
    @details SOCKETn은 @ref _Sn_MR_에 의해 선택된 프로토콜 모드와 @ref
   _Sn_PORTR_에 의해 설정된 소스 포트로 초기화 및 열립니다. \n The table shows
   @ref _Sn_SR_ is changed according to @ref _Sn_MR_.\n <table> <tr> <td>
   <b>@ref _Sn_MR_</b> (P[3:0])                      </td> <td><b>@ref
   _Sn_SR_</b> </td> </tr> <tr> <td> @ref Sn_MR_CLOSE </td> <td> @ref
   SOCK_CLOSED   </td> </tr> <tr> <td> @ref Sn_MR_TCP4, @ref Sn_MR_TCP6, @ref
   Sn_MR_TCPD </td> <td> @ref SOCK_INIT     </td> </tr> <tr> <td> @ref
   Sn_MR_UDP, @ref Sn_MR_UDP6, @ref Sn_MR_UDPD  </td> <td> @ref SOCK_UDP </td>
   </tr> <tr> <td> @ref Sn_MR_IPRAW4                                 </td> <td>
   @ref SOCK_IPRAW4   </td> </tr> <tr> <td> @ref Sn_MR_IPRAW6 </td> <td> @ref
   SOCK_IPRAW6   </td> </tr> <tr> <td> @ref Sn_MR_MACRAW </td> <td> @ref
   SOCK_MACRAW   </td> </tr>
    </table>

    @note If you want to use a SOCKETn option such as Sn_MR_MF, Sn_MR_ND,
   Sn_MR_MUTIL and etc, \n these options should be set before @ref Sn_CR_OPEN is
   performed.
    @note If you want to open a multicast UDP mode SOCKETn, \n
         You should set the multicast group with @ref _Sn_DIPR_ or @ref
   _Sn_DIP6R_ and @ref _Sn_DPORTR_ before @ref Sn_CR_OPEN is performed.
    @sa _Sn_CR_, _Sn_MR_, _Sn_SR_, _Sn_PORTR_, _Sn_DIPR_, _Sn_DIP6R_,
   _Sn_DPORTR_,
    @sa getSn_CR(), setSn_CR(), getSn_MR(), setSn_MR(), getSn_SR(),
   getSn_PORTR(), setSn_PORTR(), getSn_DIPR(), setSn_DIPR(), getSn_DIP6R(),
   setSn_DIP6R(), getSn_DPORTR(), setSn_DPORTR()
*/
#define Sn_CR_OPEN (0x01)

/**
    @brief <b>TCP 서버(SERVER)</b> 모드에서 연결 요청 대기
    @details SOCKETn은 <b>TCP 서버(SERVER)</b>로 동작하며 연결 요청(SYN 패킷)을
   대기합니다 \n with corresponding @ref _Sn_PORTR_ port number from any <b>TCP
   CLIENT</b> \n The @ref _Sn_SR_ is changed from @ref SOCK_INIT to @ref
   SOCK_LISTEN. \n When a <b>TCP CLIENT</b> connection request is successfully
   accepted,\n the @ref _Sn_SR_ is changed from @ref SOCK_LISTEN to @ref
   SOCK_ESTABLISHED \n and the @ref Sn_IR_CON is set.\n But when a <b>TCP
   CLIENT</b> connection request is failed, \n
            @ref Sn_IR_TIMEOUT is set and @ref _Sn_SR_ is changed to
   SOCK_CLOSED.
    @note This is valid only in TCP mode such as @ref Sn_MR_TCP4, @ref
   Sn_MR_TCP6, and @ref Sn_MR_TCPD.
    @sa _Sn_CR_, _Sn_MR_, _Sn_SR_, _Sn_PORTR_
    @sa getSn_CR(), setSn_CR(), getSn_MR(), setSn_MR(), getSn_SR(),
   getSn_PORTR(), setSn_PORTR()
*/
#define Sn_CR_LISTEN (0x02)

/**
    @brief <b>TCP 클라이언트(CLIENT)</b> 모드에서 연결 요청 전송
    @details 연결을 설정하기 위해 @ref _Sn_DIPR_ 및 @ref _Sn_DPORTR_에 의해
   설정된 <b>TCP 서버(SERVER)</b>로 연결 요청(SYN 패킷)을 전송합니다.\n If the
   connect-request is successful accepted by a <b>TCP SERVER</b>, \n the @ref
   _Sn_SR_ is changed to @ref SOCK_ESTABLISHED and the @ref Sn_IR_CON is set. \n
            The connect-request fails in the following three cases, \n
            and @ref _Sn_SR_ is changed to @ref SOCK_CLOSED.\n\n
    1. Until a ARP timeout is occurred (@ref Sn_IR_TIMEOUT = 1), a destination
   hardware address can not be acquired through the ARP-process.\n
    2. Until a TCP tmeout occurred (@ref Sn_IR_TIMEOUT = 1), a SYN/ACK packet is
   not received from the server\n
    3. When a RST packet is received instead of a SYN/ACK packet \n

    @note This is valid only in TCP mode such as @ref Sn_MR_TCP4 and @ref
   Sn_MR_TCPD.
    @sa _Sn_CR_, _Sn_MR_, _Sn_SR_, _Sn_DIPR_, _Sn_DPORTR_, Sn_CR_CONNECT6,
   _Sn_IR_, _Sn_IRCLR_
    @sa getSn_CR(), setSn_CR(), getSn_MR(), setSn_MR(), getSn_SR(),
   getSn_DIPR(), setSn_DIPR(), getSn_DPORTR(), setSn_DPORTR(), getSn_IR(),
   setSn_IRCLR()
*/
#define Sn_CR_CONNECT (0x04)

/**
    @brief <b>TCP 클라이언트(CLIENT)</b> 모드에서 연결 요청 전송
    @details 연결을 설정하기 위해 @ref _Sn_DIP6R_ 및 @ref _Sn_DPORTR_에 의해
   설정된 <b>TCP 서버(SERVER)</b>로 연결 요청(SYN 패킷)을 전송합니다.\n If the
   connect-request is successful accepted by a <b>TCP SERVER</b>, \n the @ref
   _Sn_SR_ is changed to @ref SOCK_ESTABLISHED and the @ref Sn_IR_CON is set. \n
            The connect-request fails in the following three cases, and @ref
   _Sn_SR_ is changed @ref SOCK_CLOSED.\n
     1. Until a ARP timeout is occurred (@ref Sn_IR_TIMEOUT = 1), a destination
   hardware address can not be acquired through the ARP-process.\n
     2. Until a TCP tmeout occurred (@ref Sn_IR_TIMEOUT = 1), a @b SYN/ACK
   packet is not received from the server\n
     3. When a RST packet is received instead of a @b SYN/ACK packet \n

    @note This is valid only in TCP mode such as @ref Sn_MR_TCP6 and @ref
   Sn_MR_TCPD.
    @sa _Sn_CR_, _Sn_MR_, _Sn_SR_, _Sn_DIP6R_, _Sn_DPORTR_, Sn_CR_CONNECT,
   _Sn_IR_, _Sn_IRCLR_
    @sa getSn_CR(), setSn_CR(), getSn_MR(), setSn_MR(), getSn_SR(),
   getSn_DIP6R(), setSn_DIP6R(), getSn_DPORTR(), setSn_DPORTR(), getSn_IR(),
   setSn_IRCLR()
*/
#define Sn_CR_CONNECT6 (0x84)

/**
    @brief TCP 모드에서 연결 해제 요청 전송
    @details <b>TCP 서버(SERVER)</b> 또는 <b>TCP 클라이언트(CLIENT)</b>에
   관계없이, \n
            @ref Sn_CR_DISCON processes the disconnect-process (Active or
   Passive close).\n When the disconnect-process is successful (that is, FIN/ACK
   packet is received successfully from/to each other),\n
            @ref _Sn_SR_ is changed to @ref SOCK_CLOSED.\n
            Otherwise, @ref Sn_IR_TIMEOUT is set and then @ref _Sn_SR_ is
   changed to @ref SOCK_CLOSED.
     - Active close
       It transmits first a disconnect-request(FIN packet) to the connected
   peer, and waits to receive two FIN/ACK and FIN packet from the peer. \n If
   two FIN/ACK and FIN packet is received successfully, @ref Sn_IR_DISCON is set
   and @ref _Sn_SR_ is changed @ref SOCK_CLOSED.
     - Passive close
       When a FIN packet is first received from the peer, the FIN/ACK packet is
   replied back to the peer and @ref Sn_IR_DISCON is set.\n And then, a FIN
   packet is sent by @ref Sn_CR_DISCON to the peer, and waits to receive the
   FIN/ACK packet from the peer. \n If the FIN/ACK packet is received
   successfully from the peer, @ref _Sn_SR_ is changed to @ref SOCK_CLOSED.

    @note It is valid only in TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6,
   and @ref Sn_MR_TCPD.
    @sa _Sn_CR_, _Sn_MR_, _Sn_SR_, _Sn_IR_, _Sn_IRCLR_, Sn_IR_DISCON,
   Sn_IR_TIMEOUT
    @sa getSn_CR(), setSn_CR(), getSn_MR(), setSn_MR(), getSn_SR(), getSn_IR(),
   setSn_IRCLR()
*/
#define Sn_CR_DISCON (0x08)

/**
    @brief SOCKETn 해제 또는 닫기(Close)
    @details TCP 모드에서 @ref Sn_CR_CLOSE는 연결 해제 절차 없이 SOCKETn을
   강제로 닫습니다.\n In other SOCKETn mode, @ref Sn_CR_CLOSE just closes a
   SOCKET.\n
    @note @ref _Sn_SR_ can be changed from any status to @ref SOCK_CLOSED by
   @ref Sn_CR_CLOSE.
    @sa _Sn_CR_, _Sn_MR_, _Sn_SR_, Sn_CR_DISCON
    @sa getSn_CR(), setSn_CR(), getSn_MR(), setSn_MR(), getSn_SR()
*/
#define Sn_CR_CLOSE (0x10)

/**
    @brief 데이터 송신
    @details @ref Sn_CR_SEND는 SOCKETn TX 버퍼 내의 @ref _Sn_TX_RD_부터 @ref
   _Sn_TX_WR_까지 저장된 데이터를 전송합니다 \n to the destination specified by
   @ref _Sn_DIPR_ or @ref _Sn_DIP6R_, and @ref _Sn_DPORTR_.\n
     - TCP mode(@ref Sn_MR_TCP4, @ref Sn_MR_TCP6,  @ref Sn_MR_TCPD)
       If it starts to be sent the data by @ref Sn_CR_SEND, @ref Sn_IR_SENDOK is
   set. \n And after sending the data, if the ACK to the sent data can not be
   received during @ref _Sn_RTR_, \n the sent data can be retransmitted as many
   as @ref _Sn_RCR_. \n During the retransmission, \n If the ACK is received,
   @ref _Sn_TX_FSR_ is increased as many as the sent data size, \n Otherwise,
   @ref Sn_IR_TIMEOUT is set and @ref _Sn_SR_ is changed to @ref SOCK_CLOSED.
     - UDP mode(@ref Sn_MR_UDP4, @ref Sn_MR_UDPD) & IPRAW mode(@ref
   Sn_MR_IPRAW4) It first sends a ARP-request to a destination specified with
   @ref _Sn_DIPR_ before it starts to be sent data by @ref Sn_CR_SEND. \n If the
   ARP-reply can not be received during @ref _Sn_RTR_, the ARP-request can be
   retransmitted as many as @ref _Sn_RCR_. \n During the retransmission, \n If
   the ARP-reply is received and @ref Sn_IR_SENDOK is set, it starts to send
   data and then @ref _Sn_TX_FSR_ is increased as many as the sent data size. \n
       Otherwise, @ref Sn_IR_TIMEOUT is set but @ref _Sn_SR_ is not changed.
     - MACRAW 모드(@ref Sn_MR_MACRAW)
       It just start to send data and @ref Sn_IR_SENDOK is set.

    @note Data size to be sent is calculated by the absolute difference between
   @ref _Sn_TX_WR_ and @ref _Sn_TX_RD_. \n In TCP or UDP mode, It can not be
   sent bigger data than @ref _Sn_TX_FSR_.\n In IPRAW or Macraw case, it can not
   be sent bigger data than MTU(Maximum Transmit Unit).
    @note In TCP or MACRAW 모드, It can send data to a destination address
   whether IPv4 or IPv6. \n In UDP or IPRAW mode, It can send data only to a
   destination IPv4 address. \n For Sending to IPv6 address, It can be used with
   @ref Sn_CR_SEND6.
    @sa _Sn_CR_, _Sn_MR_, _Sn_DIPR_, _Sn_DIP6R_, _Sn_DPORTR_, _Sn_IR_,
   _Sn_IRCLR_, _Sn_TX_FSR_, _Sn_TX_WR_, _Sn_TX_RD_
    @sa getSn_CR(), setSn_CR(), getSn_MR(), setSn_MR(), getSn_SR(),
   getSn_DIPR(), setSn_DIPR(), getSn_DIP6R(), setSn_DIP6R(), \n getSn_DPORTR(),
   setSn_DPORTR(), getSn_IR(), setSn_IRCLR(), getSn_TX_FSR(), getSn_TX_WR(),
   setSn_TX_WR(), getSn_TX_RD()
*/
#define Sn_CR_SEND (0x20)

/**
    @brief 데이터 송신
    @details @ref Sn_CR_SEND6는 SOCKETn TX 버퍼 내의 @ref _Sn_TX_RD_부터 @ref
   _Sn_TX_WR_까지 저장된 데이터를 전송합니다 \n to the destination specified by
   @ref _Sn_DIP6R_, and @ref _Sn_DPORTR_.\n
     - TCP mode(@ref Sn_MR_TCP4, @ref Sn_MR_TCP6,  @ref Sn_MR_TCPD) & MACRAW
   모드(@ref Sn_MR_MACRAW)
       @ref Sn_CR_SEND6 is not recommended. In this case, Use @ref Sn_CR_SEND.
     - UDP mode(@ref Sn_MR_UDP6, @ref Sn_MR_UDPD) & IPRAW mode(@ref
   Sn_MR_IPRAW6) It first send a neighbor solicitation NS) of ICMPv6 to a
   destination specified with @ref _Sn_DIP6R_ \n before it starts to be sent
   data by @ref Sn_CR_SEND. \n If the neighbor advertisement(NA) of ICMPv6 can
   not be received during @ref _Sn_RTR_, \n the NS can be retransmitted as many
   as @ref _Sn_RCR_. \n During the retransmission, \n If the NA is received and
   @ref Sn_IR_SENDOK is set, \n it starts to send data and then @ref _Sn_TX_FSR_
   is increased as many as the sent data size. \n Otherwise, @ref Sn_IR_TIMEOUT
   is set but @ref _Sn_SR_ is not changed.

    @note Data size to be sent is calculated by the absolute difference between
   @ref _Sn_TX_WR_ and @ref _Sn_TX_RD_. \n In TCP or UDP mode, It can not be
   sent bigger data than @ref _Sn_TX_FSR_.\n In IPRAW or Macraw case, it can not
   be sent bigger data than MTU(Maximum Transmit Unit).
    @note In UDP or IPRAW mode, It can send data only to a destination IPv6
   address. \n For Sending to IPv4 address, It can be sent by @ref Sn_CR_SEND.
    @sa _Sn_CR_, _Sn_MR_, _Sn_DIP6R_, _Sn_DPORTR_, _Sn_IR_, _Sn_IRCLR_,
   _Sn_TX_FSR_, _Sn_TX_WR_, _Sn_TX_RD_
    @sa getSn_CR(), setSn_CR(), getSn_MR(), setSn_MR(), getSn_SR(),
   getSn_DIP6R(), setSn_DIP6R(), getSn_DPORTR(), setSn_DPORTR(), \n getSn_IR(),
   setSn_IRCLR(), getSn_TX_FSR(), getSn_TX_WR(), setSn_TX_WR(), getSn_TX_RD()
*/
#define Sn_CR_SEND6 (0xA0)

/**
    @brief Keep Alive 메시지 전송
    @details @ref Sn_CR_SEND_KEEP는 1바이트 크기의 KA(Keep Alive) 패킷을
   전송하여 연결이 설정되었는지 확인합니다.\n If the destination can not respond
   to the KA packet during the time set by @ref _Sn_RTR_ and @ref _Sn_RCR_, \n
            the connection is terminated, @ref Sn_IR_TIMEOUT is set and then
   @ref _Sn_SR_ is changed @ref SOCK_CLOSED.
    @note It is valid only after sending data over 1 byte in TCP mode such as
   @ref Sn_MR_TCP4, @ref Sn_MR_TCP6, and @ref Sn_MR_TCPD.
    @sa _Sn_CR_, _Sn_MR_, _Sn_SR_, _Sn_IR_, _Sn_IRCLR_, _Sn_RTR_, _Sn_RCR_,
   _Sn_KPALVTR_
    @sa getSn_CR(), setSn_CR(), getSn_MR(), setSn_MR(), getSn_SR(), getSn_IR(),
   setSn_IRCLR(), \n getSn_RTR(), setSn_RTR(), getSn_RCR(), setSn_RCR(),
   getSn_KPALVTR(), getSn_KPALVTR()
*/
#define Sn_CR_SEND_KEEP (0x22)

/**
    @brief 데이터 수신
    @details @ref Sn_CR_RECV는 SOCKETn RX 버퍼 내의 @ref _Sn_RX_RD_부터 @ref
   _Sn_RX_WR_까지 저장된 데이터를 읽습니다.\n When a data is saved in the
   SOCKETn RX buffer, \n
            @ref Sn_IR_RECV is set and @ref _Sn_RX_RSR_ is increased as many as
   the saved data size.\n The total size of saved data is calculated by the
   absolute difference between @ref _Sn_RX_WR_ and @ref _Sn_RX_RD_,\n and it can
   be checked thru @ref _Sn_RX_RSR_.\n After reading data, @ref _Sn_RX_RD_
   should be increased as many as the read size before @ref Sn_CR_RECV is
   performed.\n After @ref Sn_CR_RECV, @ref _Sn_RX_RSR_ is decreased as many as
   the read size.\n If @ref _Sn_RX_RSR_ is remained still at none-zero, @ref
   Sn_IR_RECV is set again.
    @sa _Sn_CR_, _Sn_MR_, _Sn_IR_, _Sn_IRCLR_, _Sn_RX_RD_, _Sn_RX_WR_,
   _Sn_RX_RSR_
    @sa getSn_CR(), setSn_CR(), getSn_MR(), setSn_MR(), getSn_IR(),
   getSn_IRCLR(), \n getSn_RX_RD(), setSn_RX_RD(), getSn_RX_TX(), getSn_RX_RSR()
*/
#define Sn_CR_RECV (0x40)

/* Sn_IR values */
/**
    @brief SEND OK 인터럽트
    @details @ref Sn_IR_SENDOK는 @ref Sn_CR_SEND에 의해 데이터 전송이 시작될 때
   설정됩니다.
    @note Even though @ref Sn_IR_SENDOK is set, it does not means that the
   destination receives data successfully.\n
           - In TCP mode, The sent data maybe still transmitting or
   retransmitting. \n
           - In other modes, The sent data maybe lost by media collision or an
   other reason such as network traffic.
    @sa _Sn_IR_, _Sn_IRCLR_, Sn_CR_SEND
    @sa getSn_IR(), setSn_IRCLR(), getSn_CR(), setSn_CR()
*/
#define Sn_IR_SENDOK (0x10)

/**
    @brief TIMEOUT 인터럽트
    @details @ref Sn_IR_TIMEOUT은 ARP 및 ND 프로세스 또는 TCP 재전송에서 시간
   초과가 발생할 때 설정됩니다.
    @note In TCP mode, If it is set, @ref _Sn_SR_ is changed to @ref
   SOCK_CLOSED. \n In other modes, _Sn_SR_ is still remained at the previous
   status.
    @sa _Sn_IR_, _Sn_IRCLR_, Sn_CR_CONNECT, Sn_CR_CONNECT6, Sn_CR_SEND
    @sa getSn_IR(), setSn_IRCLR(), getSn_CR(), setSn_CR()
*/
#define Sn_IR_TIMEOUT (0x08)

/**
    @brief RECV 인터럽트
    @details @ref Sn_IR_RECV는 상대방으로부터 데이터를 수신할 때마다 설정됩니다,
   \n or if @ref _Sn_RX_RSR_ is still at none-zero whenever @ref Sn_CR_RECV is
   performed.
    @sa _Sn_IR_, _Sn_IRCLR_, Sn_CR_RECV
    @sa getSn_IR(), setSn_IRCLR(), getSn_CR(), setSn_CR()
*/
#define Sn_IR_RECV (0x04)

/**
    @brief DISCON 인터럽트
    @details @ref Sn_IR_DISCON은 연결된 상대방으로부터 FIN 또는 FIN/ACK 패킷을
   수신할 때 설정됩니다.
    @note When first a FIN packet is received from the connected peer and @ref
   _Sn_SR_ is changed to SOCK_CLOSE_WAIT, \n you should perform @ref
   Sn_CR_DISCON for a successful disconnect. \n If the disconnect-process is
   completed or failed, @ref _Sn_SR_ is changed to @ref SOCK_CLOSED.
    @note It is valild only in TCP mode such as @ref Sn_MR_TCP4, @ ref
   Sn_MR_TCP6 and @ref Sn_MR_TCPD.
    @sa _Sn_IR_, _Sn_IRCLR_, Sn_IR_DISCON, _Sn_SR_
    @sa getSn_IR(), setSn_IRCLR(), getSn_CR(), setSn_CR(), getSn_SR()
*/
#define Sn_IR_DISCON (0x02)

/**
    @brief CONNECT 인터럽트
    @details @ref Sn_IR_CON은 상대방과의 연결이 설정되고 @ref _Sn_SR_이 @ref
   SOCK_ESTABLISHED로 변경될 때 설정됩니다.
    @note It is valid only in TCP mode such as @ref Sn_MR_TCP4, @ ref Sn_MR_TCP6
   adn @ref Sn_MR_TCPD.
    @sa _Sn_IR_, _Sn_IRCLR_, _Sn_SR_
    @sa getSn_IR(), setSn_IRCLR(), getSn_SR()
*/
#define Sn_IR_CON (0x01)

/* Sn_SR values */
/**
    @brief SOCKETn 닫힘(Closed) status
    @details @ref SOCK_CLOSED는 SOCKETn이 닫히고 해제되었음을 나타냅니다.\n
            It is set when @ref Sn_CR_DISCON , @ref Sn_CR_CLOSE is performed, or
   when @ref Sn_IR_TIMEOUT is set.\n It can be changed to @ref SOCK_CLOSED
   regardless of previous status.
    @sa _Sn_SR_, _Sn_CR_, _Sn_IR_, _Sn_IRCLR_, Sn_IR_TIMEOUT
    @sa getSn_SR(), getSn_CR(), setSn_CR(), getSn_IR(), setSn_IRCLR()
*/
#define SOCK_CLOSED (0x00)

/**
    @brief TCP SOCKETn 초기화됨 상태
    @details @ref SOCK_INIT은 SOCKETn이 @ref Sn_MR_TCP4, @ref Sn_MR_TCP6, @ref
   Sn_MR_TCP6과 같은 TCP 모드로 열렸음을 나타냅니다.\n
            @ref _Sn_SR_ is changed from @ref SOCK_CLOSED to @ref SOCK_INIT when
   @ref Sn_CR_OPEN is performed in TCP mode.\n In @ref SOCK_INIT status, @ref
   Sn_CR_LISTEN for operating a <b>TCP SERVER</b> \n or @ref Sn_CR_CONNECT /
   @ref Sn_CR_CONNECT6 for operating a <b>TCP CLIENT</b> can be performed.
    @note It is valid only in TCP mode.
    @sa _Sn_SR_, _Sn_CR_, _Sn_MR_
    @sa getSn_SR, getSn_CR(), setSn_CR(), getSn_MR(), setSn_MR()
*/
#define SOCK_INIT (0x13)

/**
    @brief TCP SOCKETn 대기(Listen) 상태
    @details @ref SOCK_LISTEN은 SOCKETn이 <b>TCP 서버(SERVER)</b> 모드로
   동작하고 있음을 나타냅니다. \n and waiting for connection-request (SYN
   packet) from a peer (<b>TCP CLIENT</b>).\n
            @ref _Sn_SR_ is changed to @ref SOCK_SYNRECV when the
   connection-request(SYN packet) is successfully accepted \n and It is changed
   from @ref SOCK_SYNRECV to @ref SOCK_ESTABLISHED \n when the SYN/ACK packet is
   sent successfully to the peer and then the ACK packet of SYN/ACK is received
   successfully.\n Otherwise, it is changed to @ref SOCK_CLOSED and @ref
   Sn_IR_TIMEOUT is set.
    @note It is valid only in TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6,
   and @ref Sn_MR_TCP6..
    @sa _Sn_SR_, _Sn_CR_, _Sn_IR_, _Sn_IRCLR_, _Sn_MR_
    @sa getSn_SR, getSn_CR(), getSn_IR(), setSn_IRCLR(), setSn_CR(), getSn_MR(),
   setSn_MR()
*/
#define SOCK_LISTEN (0x14)

/**
    @brief TCP 연결 요청(Request) 상태
    @details @ref SOCK_SYNSENT는 TCP SOCKETn이 연결 요청 패킷(SYN 패킷)을
   전송했음을 나타냅니다.\n to the peer specified by @ref _Sn_DIPR_ / @ref
   _Sn_DIP6R_ and @ref _Sn_DPORTR_.\n It is temporarily shown when @ref _Sn_SR_
   is changing from @ref SOCK_INIT to @ref SOCK_ESTABLISHED by @ref
   Sn_CR_CONNECT or @ref Sn_CR_CONNECT6.\n When the connect-accept packet
   (SYN/ACK packet) is received from the peer at @ref SOCK_SYNSENT and the ACK
   packet of SYN/ACK is sent successfully, \n
            @ref _Sn_SR_ is changed to @ref SOCK_ESTABLISHED.\n
            Otherwise, it is changed to @ref SOCK_CLOSED and @ref Sn_IR_TIMEOUT
   is set.
    @note It is valid only in TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6,
   and @ref Sn_MR_TCP6..
    @sa _Sn_SR_, _Sn_CR_, _Sn_IR_, _Sn_IRCLR_, _Sn_MR_
    @sa getSn_SR, getSn_CR(), getSn_IR(), setSn_IRCLR(), setSn_CR(), getSn_MR(),
   setSn_MR()
*/
#define SOCK_SYNSENT (0x15)

/**
    @brief TCP 연결 수락(Accept) 상태
    @details @ref SOCK_SYNRECV는 TCP SOCKETn이 상대방으로부터 연결 요청 패킷(SYN
   패킷)을 성공적으로 수신했음을 나타냅니다.\n It is temporarily shown when @ref
   _Sn_SR_ is changing from @ref SOCK_LISTEN to @ref SOCK_ESTABLISHED by the SYN
   packet\n If SOCKETn sends the response (SYN/ACK  packet) to the peer
   successfully and the ACK packet of SYS/ACK is received successfully,\n
            @ref _Sn_SR_ is changed to @ref SOCK_ESTABLISHED. \n
            Otherwise, @ref _Sn_SR_ is changed to @ref SOCK_CLOSED and @ref
   Sn_IR_TIMEOUT is set.
    @note It is valid only in TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6,
   and @ref Sn_MR_TCP6..
    @sa _Sn_SR_, _Sn_CR_, _Sn_IR_, _Sn_IRCLR_, _Sn_MR_
    @sa getSn_SR, getSn_CR(), getSn_IR(), setSn_IRCLR(), setSn_CR(), getSn_MR(),
   setSn_MR()
*/
#define SOCK_SYNRECV (0x16)

/**
    @brief TCP SOCKETn 연결 설정됨(Established) 상태
    @details @ref SOCK_ESTABLISHED는 TCP SOCKETn이 상대방과 성공적으로
   연결되었음을 나타냅니다.\n when the <b>TCP SERVER</b> processes the SYN
   packet from the <b>TCP CLIENT</b> during @ref SOCK_LISTEN or \n when the
   <b>TCP CLIENT</b> performs successfully @ref Sn_CR_CONNECT or @ref
   Sn_CR_CONNECT6,\n
            @ref _Sn_SR_ is changed to @ref SOCK_ESTABLISHED and @ref Sn_IR_CON
   is set. \n During @ref SOCK_ESTABLISHED, a DATA packet can be sent to or
   received from the peer by @ref Sn_CR_SEND or @ref Sn_CR_RECV.  \n If the
   DATA/ACK packet is not received from the peer during data re-transmission,
   @ref Sn_IR_TIMEOUT is set and @ref _Sn_SR_ is changed to @ref SOCK_CLOSED.\n
            Otherwise, @ref _Sn_SR_ is still at @ref SOCK_ESTABLISHED.
    @note In <b>TCP SERVER</b>, \n
         You can check the IPv4/IPv6 address and port number of connected peer
   thru @ref _Sn_DIPR_, @ref _Sn_DIP6R_, and @ref _Sn_DPORTR_ respectively.
    @note It is valid only in TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6,
   and @ref Sn_MR_TCP6.
    @sa _Sn_SR_, _Sn_CR_, _Sn_IR_, _Sn_IRCLR_, _Sn_MR_, _Sn_DIPR_, _Sn_DIP6R_,
   _Sn_DPORTR_
    @sa getSn_SR, getSn_CR(), getSn_IR(), setSn_IRCLR(), setSn_CR(), getSn_MR(),
   setSn_MR(), getSn_DIPR(), setSn_DIPR(), getSn_DIP6R(), setSn_DIP6R(),
   getSn_DPORTR(), setSn_DPORTR().
*/
#define SOCK_ESTABLISHED (0x17)

/**
    @brief TCP SOCKETn 종료(Closing) 상태
    @details @ref SOCK_FIN_WAIT는 TCP 모드 SOCKETn이 연결 해제 절차가 완료될
   때까지 대기함을 나타냅니다. \n It is temporarily shown in disconnect-process
   such as active-close. \n When the disconnect-process is successfully
   completed or when @ref Sn_IR_TIMEOUT is set,\n
            @ref _Sn_SR_ is changed to @ref SOCK_CLOSED.
    @note It is valid only in TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6,
   and @ref Sn_MR_TCPD.
    @sa _Sn_SR_, _Sn_CR_, _Sn_IR_, _Sn_IRCLR_, _Sn_MR_, SOCK_TIME_WAIT,
   SOCK_LAST_ACK
    @sa getSn_SR, getSn_CR(), getSn_IR(), setSn_IRCLR(), setSn_CR(), getSn_MR(),
   setSn_MR()

*/
#define SOCK_FIN_WAIT (0x18)

/**
    @brief TCP SOCKETn 종료(Closing) 상태
    @details @ref SOCK_TIME_WAIT는 TCP SOCKETn이 연결 해제 절차가 완료될 때까지
   대기함을 나타냅니다.\n It is temporarily shown in disconnect-process such as
   active-close. \n When the disconnect-process is successfully completed or
   when @ref Sn_IR_TIMEOUT is set,\n
            @ref _Sn_SR_ is changed to @ref SOCK_CLOSED.
    @note It is valid only in TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6,
   and @ref Sn_MR_TCPD.
    @sa _Sn_SR_, _Sn_CR_, _Sn_IR_, _Sn_IRCLR_, _Sn_MR_, SOCK_FIN_WAIT,
   SOCK_LAST_ACK
    @sa getSn_SR, getSn_CR(), getSn_IR(), setSn_IRCLR(), setSn_CR(), getSn_MR(),
   setSn_MR()
*/
#define SOCK_TIME_WAIT (0x1B)

/**
    @brief TCP SOCKETn 반 종료(Half Closing) 상태
    @details @ref SOCK_CLOSE_WAIT는 TCP SOCKETn이 연결된 상대방으로부터 연결
   해제 요청(FIN 패킷)을 수신함을 나타냅니다.\n It is a half-closing status, and
   a DATA packet can be still sent or received by @ref Sn_CR_SEND or @ref
   Sn_CR_RECV.\n If you do not have any more need to send or received a DATA
   packet, You can perform @ref Sn_CR_DISCON for a full-closing.
    @note If you have no need the successful closing, You maybe perform @ref
   Sn_CR_CLOSE.
    @note It is valid only in TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6,
   and @ref Sn_MR_TCPD.
    @sa _Sn_SR_, _Sn_CR_, _Sn_IR_, _Sn_IRCLR_, _Sn_MR_
    @sa getSn_SR, getSn_CR(), getSn_IR(), setSn_IRCLR(), setSn_CR(), getSn_MR(),
   setSn_MR()
*/
#define SOCK_CLOSE_WAIT (0x1C)

/**
    @brief TCP SOCKETn 종료(Closing) 상태
    @details @ref SOCK_LAST_ACK는 TCP SOCKETn이 연결 해제 절차가 완료될 때까지
   대기함을 나타냅니다.\n It is temporarily shown in disconnect-process such as
   active-close and passive-close.\n When the disconnect-process is successfully
   completed or when @ref Sn_IR_TIMEOUT is set,\n
            @ref _Sn_SR_ is changed to @ref SOCK_CLOSED.
    @note It is valid only in TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6,
   and @ref Sn_MR_TCPD.
    @sa _Sn_SR_, _Sn_CR_, _Sn_IR_, _Sn_IRCLR_, _Sn_MR_, SOCK_FIN_WAIT,
   SOCK_TIME_WAIT
    @sa getSn_SR, getSn_CR(), getSn_IR(), setSn_IRCLR(), setSn_CR(), getSn_MR(),
   setSn_MR()
*/
#define SOCK_LAST_ACK (0x1D)

/**
    @brief UDP SOCKETn 상태
    @details @ref SOCK_UDP는 SOCKETn이 @ref Sn_MR_UDP4, @ref Sn_MR_UDP6, @ref
   Sn_MR_UDPD와 같은 UDP 모드로 열렸음을 나타냅니다.\n
            @ref _Sn_SR_ is changed from @ref SOCK_CLOSED to @ref SOCK_INIT when
   @ref Sn_CR_OPEN is performed in UDP mode.\n Unlike TCP mode, during @ref
   SOCK_UDP, \n a DATA packet can be sent to or received from a peer by @ref
   Sn_CR_SEND / @ref Sn_CR_SEND6 or @ref Sn_CR_RECV without a connect-process.\n
            Before a DATA packet is sent by @ref Sn_CR_SEND / @ref
   Sn_CR_SEND6,\n the ARP is requested to the peer specified by @ref _Sn_DIPR_ /
   @ref _Sn_DIP6R_.\n In ARP processing, @ref _Sn_SR_ is stll at @ref SOCK_UDP
   even if @ref Sn_IR_TIMEOUT is set.\n If you do not have any more need to send
   or received a DATA packet, \n You can perform @ref Sn_CR_CLOSE and @ref
   _Sn_SR_ is changed to @ref SOCK_CLOSED.
    @note It is valid only in UDP mode such as @ref Sn_MR_UDP4, @ref Sn_MR_UDP6,
   and @ref Sn_MR_UDPD.
    @sa _Sn_SR_, _Sn_CR_, _Sn_IR_, _Sn_IRCLR_, _Sn_MR_, _Sn_DIPR_, _Sn_DIP6R_
    @sa getSn_SR, getSn_CR(), getSn_IR(), setSn_IRCLR(), setSn_CR(), getSn_MR(),
   setSn_MR(), getSn_DIPR(), setSn_DIPR(), getSn_DIP6R(), setSn_DIP6R()
*/
#define SOCK_UDP (0x22)

/**
    @brief IPRAW4 SOCKETn 모드
    @details @ref SOCK_IPRAW4(= @ref SOCK_IPRAW)는 SOCKETn이 IPv4 RAW 모드로
   열렸음을 나타냅니다.\n
            @ref _Sn_SR_ is changed from @ref SOCK_CLOSED to @ref SOCK_IPRAW4
   when @ref Sn_CR_OPEN is performed with @ref Sn_MR_IPRAW4. \n A DATA packet
   can be send to or received from a peer without a connection like as @ref
   SOCK_UDP. \n Before a DATA packet is sent by @ref Sn_CR_SEND, \n the ARP is
   requested to the peer specified by @ref _Sn_DIPR_.\n In ARP processing, @ref
   _Sn_SR_ is still at @ref SOCK_IPRAW4 even if @ref Sn_IR_TIMEOUT is set.\n
            IPRAW4 SOCKETn can receive only the packet specified by @ref
   _Sn_PNR_, and it discards the others packets.\n If you do not have any more
   need to send or received a DATA packet, \n You can perform @ref Sn_CR_CLOSE
   and @ref _Sn_SR_ is changed to @ref SOCK_CLOSED.
    @note It is valid only in IPRAW4 mode such as @ref Sn_MR_IPRAW4.
    @sa _Sn_SR_, _Sn_CR_, _Sn_IR_, _Sn_IRCLR_, _Sn_MR_, _Sn_DIPR_, _Sn_PNR_
    @sa getSn_SR, getSn_CR(), getSn_IR(), setSn_IRCLR(), setSn_CR(), getSn_MR(),
   setSn_MR(), getSn_DIPR(), setSn_DIPR(), getSn_PNR(), setSn_PNR()
*/
#define SOCK_IPRAW4 (0x32)
#define SOCK_IPRAW (SOCK_IPRAW4) ///< Refer to @ref SOCK_IPRAW4.

/**
    @brief IPRAW6 SOCKETn 모드
    @details @ref SOCK_IPRAW6은 SOCKETn이 IPv6 RAW 모드로 열렸음을 나타냅니다.\n
            @ref _Sn_SR_ is changed from @ref SOCK_CLOSED to @ref SOCK_IPRAW6
   when @ref Sn_CR_OPEN is performed with @ref Sn_MR_IPRAW6. \n A DATA packet
   can be send to or received from a peer without a connection like as @ref
   SOCK_UDP.\n Before a DATA packet is sent by @ref Sn_CR_SEND6, \n the ICMPv6
   NS is requested to the peer specified by @ref _Sn_DIPR_ or @ref _Sn_DIP6R_.\n
            In ND(Neighbor Discovery) is processing,\n
            @ref _Sn_SR_ is still at @ref SOCK_IPRAW6 even if @ref Sn_IR_TIMEOUT
   is set.\n IPRAW6 SOCKETn can receive only the packet specified by @ref
   _Sn_PNR_, and it discards the others packets.\n If you do not have any more
   need to send or received a DATA packet, \n You can perform @ref Sn_CR_CLOSE
   and @ref _Sn_SR_ is changed to @ref SOCK_CLOSED.
    @note It is valid only in IPRAW6 mode such as @ref Sn_MR_IPRAW6.
    @sa _Sn_SR_, _Sn_CR_, _Sn_IR_, _Sn_IRCLR_, _Sn_MR_, _Sn_DIP6R_, _Sn_PNR_
    @sa getSn_SR, getSn_CR(), getSn_IR(), setSn_IRCLR(), setSn_CR(), getSn_MR(),
   setSn_MR(), getSn_DIP6R(), setSn_DIP6R(), getSn_PNR(), setSn_PNR()
*/
#define SOCK_IPRAW6 (0x33)

/**
    @brief MACRAW SOCKETn 모드
    @details @ref SOCK_MACRAW는 SOCKET0이 MACRAW 모드로 열렸음을 나타냅니다.\n
            @ref _Sn_SR_ is changed from @ref SOCK_CLOSED to @ref SOCK_MACRAW
   when @ref Sn_CR_OPEN command is ordered with @ref Sn_MR_MACRAW.\n MACRAW
   SOCKET0 can be sent or received a pure Ethernet frame packet to/from any
   peer.
    @note  It is valid only in SOCKET0.
    @sa _Sn_SR_, _Sn_CR_, _Sn_IR_, _Sn_IRCLR_, _Sn_MR_
    @sa getSn_SR, getSn_CR(), getSn_IR(), setSn_IRCLR(), setSn_CR(), getSn_MR(),
   setSn_MR(),
*/
#define SOCK_MACRAW (0x42)

/* Sn_ESR values */
/**
    @brief SOCKETn 확장 상태 : TCP 모드
    @details @ref Sn_ESR_TCPM은 @ref _Sn_ESR_의 TCPM 비트를 마스킹합니다. \n
            The masked bit values are as following. \n
            - @ref Sn_ESR_TCPM_IPV4
            - @ref Sn_ESR_TCPM_IPV6
    @note It is useful to know the destination IP version when TCPD(@ref
   Sn_MR_TCPD) mode SOCKETn is operated as <b>TCP SERVER</b>.
    @sa _Sn_ESR_
    @sa getSn_ESR()
*/
#define Sn_ESR_TCPM (1 << 2)

/**
    @brief TCP SOCKETn IP 버전 - IPv4
    @details @ref Sn_ESR_TCPM_IPV4는 TCP SOCKETn이 IPv4 상에서 동작함을
   나타냅니다.
    @sa _Sn_ESR_, Sn_ESR_TCPM_IPV6
    @sa getSn_ESR()
*/
#define Sn_ESR_TCPM_IPV4 (0 << 2)

/**
    @brief TCP SOCKETn IP 버전 - IPv6
    @details @ref Sn_ESR_TCPM_IPV6는 TCP SOCKETn이 IPv6 상에서 동작함을
   나타냅니다.
    @sa _Sn_ESR_, Sn_ESR_TCPM_IPV4
    @sa getSn_ESR()
*/
#define Sn_ESR_TCPM_IPV6 (1 << 2)

/**
    @brief SOCKETn 확장 상태 : TCP 동작 모드
    @details @ref Sn_ESR_TCPOP은 @ref _Sn_ESR_의 TCPOP 비트를 마스킹합니다.
   마스킹된 비트 값은 다음과 같습니다. \n
              - @ref Sn_ESR_TCPOP_SVR
              - @ref Sn_ESR_TCPOP_CLT
    @note It is useful to check TCP mode SOCKETn is operated as whether <b>TCP
   SERVER</b> or <b>TCP CLIENT</b>.
    @sa _Sn_ESR_
    @sa getSn_ESR()
*/
#define Sn_ESR_TCPOP (1 << 1)

/**
    @brief TCP SOCKETn 동작 모드 - <b>TCP 서버(SERVER)</b>
    @details @ref Sn_ESR_TCPOP_SVR은 TCP 모드 SOCKET n이 <b>TCP
   서버(SERVER)</b>로 동작함을 나타냅니다
    @note It is valid only in TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6,
   and @ref Sn_MR_TCPD.
    @sa _Sn_ESR_, Sn_ESR_TCPOP_CLT
    @sa getSn_ESR()
*/
#define Sn_ESR_TCPOP_SVR (0 << 1)

/**
    @brief TCP SOCKETn 동작 모드 - <b>TCP 클라이언트(CLIENT)</b>
    @details @ref Sn_ESR_TCPOP_SVR은 TCP 모드 SOCKET n이 <b>TCP
   클라이언트(CLIENT)</b>로 동작함을 나타냅니다
    @note It is valid only in TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6,
   and @ref Sn_MR_TCPD.
    @sa _Sn_ESR_, Sn_ESR_TCPOP_SVR
    @sa getSn_ESR()
*/
#define Sn_ESR_TCPOP_CLT (1 << 1)

/**
    @brief SOCKETn 확장 상태 : 소스 IPv6 주소 유형
    @details @ref Sn_ESR_IP6T는 @ref _Sn_ESR_의 IP6T 비트를 마스킹합니다. \n
            The masked bit values are as following. \n
              - @ref Sn_ESR_IP6T_LLA
              - @ref Sn_ESR_IP6T_GUA
    @note It is useful to check whether the connected peer IP address is LLA or
   GUA.
    @note It is valid only in TCP mode such as @ref Sn_MR_TCP6 and @ref
   Sn_MR_TCPD.
    @sa _Sn_ESR_
    @sa getSn_ESR()
*/
#define Sn_ESR_IP6T (1 << 0)

/**
    @brief 소스 IPv6 주소 유형 - LLA
    @details @ref Sn_ESR_IP6T_LLA는 소스 IPv6 주소가 @ref _LLAR_로 사용됨을
   나타냅니다.
    @note It is valid only in TCP mode such as @ref Sn_MR_TCP6, and @ref
   Sn_MR_TCPD.
    @sa _Sn_ESR_, Sn_ESR_IP6T_GUA, _LLAR_
    @sa getSn_ESR(), getLLAR(), setLLAR()
*/
#define Sn_ESR_IP6T_LLA (0 << 0)

/**
    @brief 소스 IPv6 주소 유형 - LLA
    @details @ref Sn_ESR_IP6T_GUA는 소스 IPv6 주소가 @ref _GUAR_로 사용됨을
   나타냅니다.
    @note It is valid only in TCP mode such as @ref Sn_MR_TCP6, and @ref
   Sn_MR_TCPD.
    @sa _Sn_ESR_, Sn_ESR_IP6T_LLA, _GUAR_
    @sa getSn_ESR(), getGUAR(), setGUAR()
*/
#define Sn_ESR_IP6T_GUA (1 << 0)

/* Sn_MR2 values */
/**
    @brief 목적지 하드웨어 주소 모드
    @details @ref Sn_MR2_DHAM은 @ref _Sn_MR2_의 DHAM 비트를 마스킹합니다.\n
            The masked bit values are as following.
              - @ref Sn_MR2_DHAM_AUTO
              - @ref Sn_MR2_DHAM_MANUAL
    @sa _Sn_MR2_
    @sa getSn_MR2(), setSn_MR2()
*/
#define Sn_MR2_DHAM (1 << 1)

/**
    @brief 목적지 하드웨어 주소 모드 - AUTO
    @details @ref Sn_MR2_DHAM_AUTO는 ARP 프로세스를 통해 획득한 주소를 목적지
   하드웨어 주소로 설정합니다.
    @sa _Sn_MR2_, Sn_MR_DHAM_MANUAL, NETMR_DHAS
    @sa getSn_MR2(), setSn_MR2(), getNETMR(), setNETMR()
*/
#define Sn_MR2_DHAM_AUTO (0 << 1)

/**
    @brief 목적지 하드웨어 주소 모드 - MANUAL
    @details @ref Sn_MR2_DHAM_MANUAL은 목적지 하드웨어 주소를 @ref _Sn_DHAR_로
   설정합니다.
    @sa _Sn_MR2_, Sn_MR_DHAM_MANUAL, NETMR_DHAS
    @sa getSn_MR2(), setSn_MR2(), getNETMR(), setNETMR()
*/
#define Sn_MR2_DHAM_MANUAL (1 << 1)

/**
    @brief 강제 ARP
    @details @ref Sn_MR2_FARP는 데이터 통신 전에 목적지 하드웨어 주소를 얻기
   위해 ARP 프로세스를 강제로 수행합니다.\n 0 : Normal \n 1 : 강제 ARP
     - In TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6, and @ref Sn_MR_TCPD
       If SOCKETn is operated as <b>TCP SERVER</b>, It sets the destination
   hardware address as the address acquired by the forced ARP-process before
   sending SYN/ACK packet.
     - In UDP mode such as @ref Sn_MR_UDP4, @ref Sn_MR_UDP6, and @ref Sn_MR_UDPD
       It sets the destination hardware address as the address acquired by the
   forced ARP-process whenever @ref Sn_CR_SEND or @ref Sn_CR_SEND6.
    @note When @ref Sn_MR2_DHAM_MANUAL and @ref Sn_MR2_FARP = '1', It sets the
   destination hardware address as @ref _Sn_DHAR_ even if the ARP-process is
   forced.
*/
#define Sn_MR2_FARP (1 << 0)

/*----------------------------For PHY Control-------------------------------*/

/**
    @ingroup Common_register_group_W6100
    @brief 이더넷 PHY의 기본 모드 제어(Control) 레지스터 [RW][0x3100]
    @details @ref PHYRAR_BMCR은 @ref _WIZCHIP_의 MDC/MDIO 컨트롤러에 의해 제어될
   수 있습니다. \n Each bit of @ref PHYRAR_BMCR is defined as the following.
    <table>
      <tr> <td>15</td>  <td>14</td> <td>13</td>  <td>12 </td>  <td>11</td>
   <td>10  </td> <td>9  </td> <td>8  </td> <td>7   </td>  <td>6 ~ 0  </td> </tr>
      <tr> <td>RST</td> <td>LB</td> <td>SPD</td> <td>ANE</td> <td>PWDN</td>
   <td>ISOL</td> <td>RAN</td> <td>DPX</td> <td>COLT</td> <td>Reserved</td> </tr>
    </table>
     - @ref BMCR_RST
     - @ref BMCR_LB
     - @ref BMCR_SPD
     - @ref BMCR_ANE
     - @ref BMCR_PWDN
     - @ref BMCR_ISOL : Not supported.
     - @ref BMCR_REAN
     - @ref BMCR_DPX
     - @ref BMCR_COLT

    @note Its some bits have the same function as @ref _PHYCR0_ and @ref
   _PHYCR1_.\n It can control the Ethernet PHY with software, while @ref
   _PHYCR0_ \n and @ref _PHYCR1_ can control the Ethernet PHY with hardware.

    @sa PHYRAR_BMSR, _PHYRAR_, _PHYDIR_, _PHYDOR_, _PHYACR_, _PHYCR0_, _PHYCR1_
    @sa getPHYRAR(), setPHYRAR(), wiz_mdio_read(), wiz_mdio_write()
*/
#define PHYRAR_BMCR (0x00)

// Basic mode status register, basic register
/**
    @ingroup Common_register_group_W6100
    @brief 이더넷 PHY의 기본 모드 상태(Status) 레지스터 [RO][0x7809]
    @details @ref PHYRAR_BMSR은 @ref _WIZCHIP_의 MDC/MDIO 컨트롤러를 통해 이더넷
   PHY의 상태를 가져옵니다. \n Each bit of @ref PHYRAR_BMSR is defined as the
   following. <table> <tr> <td>15</td> <td>14</td> <td>13</td> <td>12</td>
   <td>11</td> <td>10~7</td> <td>6 </td> <td>5 </td> <td>4 </td> <td>3 </td>
   <td>2 </td> <td>1   </td> <td>0</td> </tr> <tr> <td>100_T4  </td> <td>100_FDX
   </td> <td>100_HDX    </td> <td>10_FDX     </td> <td>10_HDX       </td>
   <td>Reserved</td> <td>MF_SUP</td> <td>ANG_COMP</td> <td>REMOTE_FAULT</td>
   <td>ANG_ABILITY</td> <td>LINK_STATUS</td> <td>JABBER_DETECT</td>
   <td>EXT_CAPA</td> </tr>
    </table>
     - @ref BMSR_100_T4        : Not supported. Always 0
     - @ref BMSR_100_FDX
     - @ref BMSR_100_HDX
     - @ref BMSR_10_FDX
     - @ref BMSR_10_HDX
     - @ref BMSR_MF_SUP        : Not supported. Always 0.
     - @ref BMSR_AN_COMP
     - @ref BMSR_REMOTE_FAULT  : Not supported. Always 0.
     - @ref BMSR_AN_ABILITY
     - @ref BMSR_LINK_STATUS
     - @ref BMSR_JABBER_DETECT
     - @ref BMSR_EXT_CAPA      : Always 1. If you need a extended register
   information, send e-mail to support@wiznet.io

    @note Its some bits have the same function as @ref _PHYSR_.
    @sa PHYRAR_BMCR, _PHYRAR_, _PHYDIR_, _PHYDOR_, _PHYACR_, _PHYCR0_, _PHYCR1_
    @sa getPHYRAR(), setPHYRAR(), wiz_mdio_read(), wiz_mdio_write()
*/
#define PHYRAR_BMSR (0x01)

/********************/
/* BMCR & BMSR Bit definitions  */
/********************/

/*For BMCR register*/
/**
    @brief 이더넷 PHY 소프트웨어(S/W) 리셋.
    @details 0 - 정상 동작 \n
            1 - Software reset
    @sa PHYRAR_BMCR, PHYCR1_RST
    @sa getPHYRAR_BMCR(), setPHYRAR_BMCR()
*/
#define BMCR_RST (1 << 15)

/**
    @brief 이더넷 PHY 루프백(Loopback).
    @details 0 - 정상 동작 \n
            1 - Loopback Enable
    @sa PHYRAR_BMCR
    @sa getPHYRAR_BMCR(), setPHYRAR_BMCR()
*/
#define BMCR_LB                                                                \
  (1 << 14) ///< 루프백(Loopback). 0 - 정상 동작, 1 - 루프백 활성화됨

/**
    @brief 이더넷 PHY 속도
    @details 0 - 10 Mbps \n
            1 - 100 Mbps
    @sa PHYCR_BMCR, PHYCR0_100F, PHYCR0_100H, PHYCR0_10F, PHYCR0_10H
    @sa getPHYRAR_BMCR(), setPHYRAR_BMCR(), setPHYCR0()
*/
#define BMCR_SPD (1 << 13)

/**
    @brief 이더넷 PHY 자동 협상(Auto-Negotiation)
    @details 0 - 비활성화 \n
            1 - Enable
    @note When it is set, @ref BMCR_SPD and @ref BMCR_DPX is ignored
    @sa PHYCR_BMCR, PHYCR0_AUTO
    @sa getPHYRAR_BMCR(), setPHYRAR_BMCR(), setPHYCR0()
*/
#define BMCR_ANE (1 << 12)

/**
    @brief 이더넷 PHY 전원 차단(Power Down) 모드
    @details 0 - 정상 동작 \n
            1 - Power Down mode
    @sa PHYCR_BMCR, PHYCR0_PWDN
    @sa getPHYRAR_BMCR(), setPHYRAR_BMCR(), setPHYCR0()
*/
#define BMCR_PWDN (1 << 11) ///< 전원 차단(Power-down) 모드

/**
    @brief 이더넷 PHY 격리(Isolation) 모드
    @details 0 - 정상 동작 \n
            1 - Isolation Mode
    @ Don't set it to '1'. It is not supported.
    @sa PHYCR_BMCR
    @sa getPHYRAR_BMCR(), setPHYRAR_BMCR()
*/
#define BMCR_ISOL (1 << 10)

/**
    @brief 이더넷 PHY 자동 협상(Auto-Negotiation) 재시작
    @details 0 - 정상 동작 \n
            1 - Restart Auto-Negotiation
    @sa PHYCR_BMCR
    @sa getPHYRAR_BMCR(), setPHYRAR_BMCR()
*/
#define BMCR_REAN (1 << 9)

/**
    @brief 이더넷 PHY 이중(Duplex) 모드
    @details 0 - 반이중(Half-Duplex) \n
            1 - Full-Duplex
    @sa PHYCR_BMCR, PHYCR0_100F, PHYCR0_100H, PHYCR0_10F, PHYCR0_10H
    @sa getPHYRAR_BMCR(), setPHYRAR_BMCR(), setPHYCR0()
*/
#define BMCR_DPX (1 << 8)

/**
    @brief 이더넷 PHY 충돌(Collision) 테스트
    @details 0 - 정상 동작 \n
            1 - Collision Test
    @sa PHYCR_BMCR
    @sa getPHYRAR_BMCR(), setPHYRAR_BMCR()
*/
#define BMCR_COLT (1 << 7)

/*For BMSR register*/

/**
    @brief 이더넷 PHY 100 Base-T4 지원
    @details @ref BMSR_100_T4는 항상 0입니다.
    @note It is not supported.
    @sa PHYCR_BMSR
    @sa getPHYRAR_BMSR()
*/
#define BMSR_100_T4 (1 << 15)

/**
    @brief 이더넷 PHY 100 Base-TX 전이중(Full-Duplex) 지원
    @details @ref BMSR_100_FDX는 항상 1입니다.
    @sa PHYCR_BMSR
    @sa getPHYRAR_BMSR()
*/
#define BMSR_100_FDX (1 << 14)

/**
    @brief 이더넷 PHY 100 Base-TX 반이중(Half-Duplex) 지원
    @details @ref BMSR_100_HDX는 항상 1입니다.
    @sa PHYCR_BMSR
    @sa getPHYRAR_BMSR()
*/
#define BMSR_100_HDX (1 << 13)

/**
    @brief 이더넷 PHY 10 Base-T 전이중(Full-Duplex) 지원
    @details @ref BMSR_10_FDX는 항상 1입니다.
    @sa PHYCR_BMSR
    @sa getPHYRAR_BMSR()
*/
#define BMSR_10_FDX (1 << 12)

/**
    @brief 이더넷 PHY 10 Base-T 반이중(Half-Duplex) 지원
    @details @ref BMSR_10_HDX는 항상 1입니다.
    @sa PHYCR_BMSR
    @sa getPHYRAR_BMSR()
*/
#define BMSR_10_HDX (1 << 11)

/**
    @brief 이더넷 PHY 관리 프레임 프리앰블(Preamble) 억제
    @details @ref BMSR_MF_SUP는 항상 0입니다.
    @note It is not supported
    @sa PHYCR_BMSR
    @sa getPHYRAR_BMSR()
*/
#define BMSR_MF_SUP (1 << 6)

/**
    @brief 이더넷 PHY 자동 협상(Auto-Negotiation) Complete
    @details @ref BMSR_MF_SUP는 자동 협상 상태를 나타냅니다. \n
            0 - Auto-negotiation process is not completed \n
            1 - Auto-negotiation process is completed
    @sa PHYCR_BMSR
    @sa getPHYRAR_BMSR()
*/
#define BMSR_AN_COMP (1 << 5)

/**
    @brief 이더넷 PHY 원격 결함(Remote Fault)
    @details @ref BMSR_REMOTE_FAULT는 항상 0입니다.
    @note It is not supported
    @sa PHYCR_BMSR
    @sa getPHYRAR_BMSR()
*/
#define BMSR_REMOTE_FAULT (1 << 4)

/**
    @brief 이더넷 PHY 자동 협상(Auto-Negotiation) Ability
    @details @ref BMSR_AN_ABILITY는 항상 1입니다.
    @sa PHYCR_BMSR
    @sa getPHYRAR_BMSR()
*/
#define BMSR_AN_ABILITY (1 << 3)

/**
    @brief 이더넷 PHY 링크 상태
    @details @ref BMSR_LINK_STATUS는 링크 상태를 나타냅니다. \n
            0 - Link is not established
            1 - Valid link is established
    @sa PHYCR_BMSR
    @sa getPHYRAR_BMSR()
*/
#define BMSR_LINK_STATUS (1 << 2)

/**
    @brief 이더넷 PHY 재버(Jabber) 감지
    @details @ref BMSR_JABBER_DETECT는 재버 감지 상태를 나타냅니다. \n
            0 - Jabber condition is not detected\n
            1 - Jabber condition is detected
    @sa PHYCR_BMSR
    @sa getPHYRAR_BMSR()
*/
#define BMSR_JABBER_DETECT (1 << 1)

/**
    @brief 이더넷 PHY 확장(Extended) 기능
    @details @ref BMSR_EXT_CAPA는 확장 레지스터 기능을 나타냅니다. \n
            0 - Only basic registers are capable\n
            1 - Extended registers are capable
    @sa PHYCR_BMSR
    @sa getPHYRAR_BMSR()
*/
#define BMSR_EXT_CAPA (1 << 0)

/**
    @brief 임계 영역(Critical section) 진입
    @details 간섭으로부터 공유 코드 및 하드웨어 자원을 보호하기 위해 제공됩니다.
   \n
     - Non-OS environment
       It can be just implemented by disabling whole interrupt.
     - OS environment
       You can replace it to critical section API supported by OS.

    @note It is callback function that can be replaced it with your code, by
   calling @ref reg_wizchip_cris_cbfunc().
    @sa WIZCHIP_READ(), WIZCHIP_WRITE(), WIZCHIP_READ_BUF(), WIZCHIP_WRITE_BUF()
    @sa WIZCHIP_CRITICAL_EXIT(), reg_wizchip_cris_cbfunc()
*/
#define WIZCHIP_CRITICAL_ENTER() WIZCHIP.CRIS._enter()

/**
    @brief 임계 영역(Critical section) 진입
    @details 간섭으로부터 코드 및 하드웨어 자원을 보호하는 영역을 빠져나옵니다.
   \n
     - Non-OS environment
       It can be just implemented by enabling whole interrupt.\n
     - OS environment
       You can replace it to critical section API supported by OS.

    @note It is callback function that can be replaced it with your code, by
   calling @ref reg_wizchip_cris_cbfunc().
    @sa WIZCHIP_READ(), WIZCHIP_WRITE(), WIZCHIP_READ_BUF(), WIZCHIP_WRITE_BUF()
    @sa WIZCHIP_CRITICAL_EXIT(), reg_wizchip_cris_cbfunc()
*/
#define WIZCHIP_CRITICAL_EXIT() WIZCHIP.CRIS._exit()

////////////////////////
// Basic I/O Function //
////////////////////////
//
//
/*
@function    uint8_t WIZCHIP_READ(uint32_t AddrSel)
@brief      레지스터에서 1바이트 값을 읽어옵니다.
@param      AddrSel: 레지스터 주소
@return     레지스터의 값
*/
uint8_t WIZCHIP_READ(uint32_t AddrSel);

/*
@function    void WIZCHIP_WRITE(uint32_t AddrSel, uint8_t wb)
@brief      레지스터에 1바이트 값을 씁니다.
@param      AddrSel: 레지스터 주소
@param      wb: 쓸 데이터
@return     void
*/
void WIZCHIP_WRITE(uint32_t AddrSel, uint8_t wb);

/*
@function    void WIZCHIP_READ_BUF(uint32_t AddrSel, uint8_t* pBuf, uint16_t
len)
@brief      레지스터에서 순차적인 데이터를 읽어옵니다.
@param      AddrSel: 레지스터 시작 주소
@param      pBuf: 데이터를 읽어올 버퍼 포인터
@param      len: 데이터 길이
@return     void
*/
void WIZCHIP_READ_BUF(uint32_t AddrSel, uint8_t *pBuf, uint16_t len);

/*
@function    void WIZCHIP_WRITE_BUF(uint32_t AddrSel, uint8_t* pBuf, uint16_t
len)
@brief      레지스터에 순차적인 데이터를 씁니다.
@param      AddrSel: 레지스터 시작 주소
@param      pBuf: 쓸 데이터가 담긴 버퍼 포인터
@param      len: 데이터 길이
@return     void
*/
void WIZCHIP_WRITE_BUF(uint32_t AddrSel, uint8_t *pBuf, uint16_t len);

/////////////////////////////////
// Common Register IO function //
/////////////////////////////////
/**
    @addtogroup Common_register_access_function_W6100
    @{
*/
#define getCIDR()                                                              \
  ((((uint16_t)WIZCHIP_READ(_CIDR_)) << 8) +                                   \
   WIZCHIP_READ(WIZCHIP_OFFSET_INC(_CIDR_, 1)))

/*
@function    getVER()
@brief      VER 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getVER()                                                               \
  ((((uint16_t)WIZCHIP_READ(_VER_)) << 8) +                                    \
   WIZCHIP_READ(WIZCHIP_OFFSET_INC(_VER_, 1)))

/*
@function    getSYSR()
@brief      SYSR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getSYSR() WIZCHIP_READ(_SYSR_)

/*
@function    getSYCR0()
@brief      SYCR0 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getSYCR0() WIZCHIP_READ(_SYCR0_)

/*
@function    setSYCR0(sycr0)
@brief      SYCR0 레지스터에 값을 설정합니다.
@param      sycr0: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSYCR0(sycr0) WIZCHIP_WRITE(_SYCR0_, (sycr0))

/*
@function    getSYCR1()
@brief      SYCR1 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getSYCR1() WIZCHIP_READ(_SYCR1_)

/*
@function    setSYCR1(sycr1)
@brief      SYCR1 레지스터에 값을 설정합니다.
@param      sycr1: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSYCR1(sycr1) WIZCHIP_WRITE(_SYCR1_, (sycr1))

/*
@function    getTCNTR()
@brief      TCNTR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getTCNTR()                                                             \
  ((((uint16_t)WIZCHIP_READ(_TCNTR_)) << 8) +                                  \
   WIZCHIP_READ(WIZCHIP_OFFSET_INC(_TCNTR_, 1)))

/*
@function    setTCNTRCLR(tcntrclr)
@brief      TCNTRCLR 레지스터에 값을 설정합니다.
@param      tcntrclr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setTCNTRCLR(tcntrclr) WIZCHIP_WRITE(_TCNTRCLR_, (tcntrclr))

/*
@function    getIR()
@brief      IR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getIR() WIZCHIP_READ(_IR_)

/*
@function    getSIR()
@brief      SIR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getSIR() WIZCHIP_READ(_SIR_)

/*
@function    getSLIR()
@brief      SLIR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getSLIR() WIZCHIP_READ(_SLIR_)

/*
@function    setIMR(imr)
@brief      IMR 레지스터에 값을 설정합니다.
@param      imr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setIMR(imr) WIZCHIP_WRITE(_IMR_, (imr))

/*
@function    getIMR()
@brief      IMR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getIMR() WIZCHIP_READ(_IMR_)

/*
@function    setIRCLR(irclr)
@brief      IRCLR 레지스터에 값을 설정합니다.
@param      irclr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setIRCLR(irclr) WIZCHIP_WRITE(_IRCLR_, (irclr))
/*
@function    setIR(ir)
@brief      IR 레지스터에 값을 설정합니다.
@param      ir: 설정할 값 또는 소켓 번호
@return     void
*/
#define setIR(ir) setIRCLR(ir)

/*
@function    setSIMR(simr)
@brief      SIMR 레지스터에 값을 설정합니다.
@param      simr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSIMR(simr) WIZCHIP_WRITE(_SIMR_, (simr))

/*
@function    getSIMR()
@brief      SIMR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getSIMR() WIZCHIP_READ(_SIMR_)

/*
@function    setSLIMR(slimr)
@brief      SLIMR 레지스터에 값을 설정합니다.
@param      slimr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSLIMR(slimr) WIZCHIP_WRITE(_SLIMR_, (slimr))

/*
@function    getSLIMR()
@brief      SLIMR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getSLIMR() WIZCHIP_READ(_SLIMR_)

/*
@function    setSLIRCLR(slirclr)
@brief      SLIRCLR 레지스터에 값을 설정합니다.
@param      slirclr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSLIRCLR(slirclr) WIZCHIP_WRITE(_SLIRCLR_, (slirclr))
/*
@function    setSLIR(slir)
@brief      SLIR 레지스터에 값을 설정합니다.
@param      slir: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSLIR(slir) setSLIRCLR(slir)

/*
@function    setSLPSR(slpsr)
@brief      SLPSR 레지스터에 값을 설정합니다.
@param      slpsr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSLPSR(slpsr) WIZCHIP_WRITE(_SLPSR_, (slpsr))

/*
@function    getSLPSR()
@brief      SLPSR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getSLPSR() WIZCHIP_READ(_SLPSR_)

/*
@function    setSLCR(slcr)
@brief      SLCR 레지스터에 값을 설정합니다.
@param      slcr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSLCR(slcr) WIZCHIP_WRITE(_SLCR_, (slcr))

/*
@function    getSLCR()
@brief      SLCR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getSLCR() WIZCHIP_READ(_SLCR_)

/*
@function    getPHYSR()
@brief      PHYSR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getPHYSR() WIZCHIP_READ(_PHYSR_)

/*
@function    setPHYRAR(phyrar)
@brief      PHYRAR 레지스터에 값을 설정합니다.
@param      phyrar: 설정할 값 또는 소켓 번호
@return     void
*/
#define setPHYRAR(phyrar) WIZCHIP_WRITE(_PHYRAR_, (phyrar))

/*
@function    getPHYRAR()
@brief      PHYRAR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getPHYRAR() WIZCHIP_READ(_PHYRAR_)

/*
@function    setPHYDIR(phydir)
@brief      PHYDIR 레지스터에 값을 설정합니다.
@param      phydir: 설정할 값 또는 소켓 번호
@return     void
*/
#define setPHYDIR(phydir)                                                      \
  do {                                                                         \
    WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(_PHYDIR_, 1), (uint8_t)((phydir) >> 8));  \
    WIZCHIP_WRITE(_PHYDIR_, (uint8_t)(phydir));                                \
  } while (0);

/*
@function    getPHYDOR()
@brief      PHYDOR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getPHYDOR()                                                            \
  ((((uint16_t)WIZCHIP_READ(WIZCHIP_OFFSET_INC(_PHYDOR_, 1))) << 8) +          \
   WIZCHIP_READ(_PHYDOR_))

/*
@function    setPHYACR(phyacr)
@brief      PHYACR 레지스터에 값을 설정합니다.
@param      phyacr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setPHYACR(phyacr) WIZCHIP_WRITE(_PHYACR_, (phyacr))

/*
@function    getPHYACR()
@brief      PHYACR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getPHYACR() WIZCHIP_READ(_PHYACR_)

/*
@function    setPHYDIVR(phydivr)
@brief      PHYDIVR 레지스터에 값을 설정합니다.
@param      phydivr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setPHYDIVR(phydivr) WIZCHIP_WRITE(_PHYDIVR_, (phydivr))

/*
@function    getPHYDIVR()
@brief      PHYDIVR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getPHYDIVR() WIZCHIP_READ(_PHYDIVR_)

/*
@function    setPHYCR0(phycr0)
@brief      PHYCR0 레지스터에 값을 설정합니다.
@param      phycr0: 설정할 값 또는 소켓 번호
@return     void
*/
#define setPHYCR0(phycr0) WIZCHIP_WRITE(_PHYCR0_, (phycr0))

/*
@function    setPHYCR1(phycr1)
@brief      PHYCR1 레지스터에 값을 설정합니다.
@param      phycr1: 설정할 값 또는 소켓 번호
@return     void
*/
#define setPHYCR1(phycr1) WIZCHIP_WRITE(_PHYCR1_, (phycr1))

/*
@function    getPHYCR1()
@brief      PHYCR1 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getPHYCR1() WIZCHIP_READ(_PHYCR1_)

/*
@function    setNET4MR(net4mr)
@brief      NET4MR 레지스터에 값을 설정합니다.
@param      net4mr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setNET4MR(net4mr) WIZCHIP_WRITE(_NET4MR_, (net4mr))

/*
@function    setNET6MR(net6mr)
@brief      NET6MR 레지스터에 값을 설정합니다.
@param      net6mr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setNET6MR(net6mr) WIZCHIP_WRITE(_NET6MR_, (net6mr))

/*
@function    setNETMR(netmr)
@brief      NETMR 레지스터에 값을 설정합니다.
@param      netmr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setNETMR(netmr) WIZCHIP_WRITE(_NETMR_, (netmr))

/*
@function    setNETMR2(netmr2)
@brief      NETMR2 레지스터에 값을 설정합니다.
@param      netmr2: 설정할 값 또는 소켓 번호
@return     void
*/
#define setNETMR2(netmr2) WIZCHIP_WRITE(_NETMR2_, (netmr2))

/*
@function    getNET4MR()
@brief      NET4MR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getNET4MR() WIZCHIP_READ(_NET4MR_)

/*
@function    getNET6MR()
@brief      NET6MR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getNET6MR() WIZCHIP_READ(_NET6MR_)

/*
@function    getNETMR()
@brief      NETMR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getNETMR() WIZCHIP_READ(_NETMR_)

/*
@function    getNETMR2()
@brief      NETMR2 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getNETMR2() WIZCHIP_READ(_NETMR2_)

/*
@function    setPTMR(ptmr)
@brief      PTMR 레지스터에 값을 설정합니다.
@param      ptmr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setPTMR(ptmr) WIZCHIP_WRITE(_PTMR_, (ptmr))

/*
@function    getPTMR()
@brief      PTMR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getPTMR() WIZCHIP_READ(_PTMR_)

/*
@function    setPMNR(pmnr)
@brief      PMNR 레지스터에 값을 설정합니다.
@param      pmnr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setPMNR(pmnr) WIZCHIP_WRITE(_PMNR_, (pmnr))

/*
@function    getPMNR()
@brief      PMNR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getPMNR() WIZCHIP_READ(_PMNR_)

/*
@function    setPHAR(phar)
@brief      PHAR 레지스터에 값을 설정합니다.
@param      phar: 설정할 값 또는 소켓 번호
@return     void
*/
#define setPHAR(phar) WIZCHIP_WRITE_BUF(_PHAR_, (phar), 6)

/*
@function    getPHAR(phar)
@brief      PHAR 레지스터의 값을 읽어옵니다.
@param      phar: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getPHAR(phar) WIZCHIP_READ_BUF(_PHAR_, (phar), 6)

/*
@function    setPSIDR(psidr)
@brief      PSIDR 레지스터에 값을 설정합니다.
@param      psidr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setPSIDR(psidr)                                                        \
  do {                                                                         \
    WIZCHIP_WRITE(_PSIDR_, (uint8_t)((psidr) >> 8));                           \
    WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(_PSIDR_, 1), (uint8_t)(psidr));           \
  } while (0);

/*
@function    getPSIDR()
@brief      PSIDR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getPSIDR()                                                             \
  ((((uint16_t)WIZCHIP_READ(_PSIDR_)) << 8) +                                  \
   WIZCHIP_READ(WIZCHIP_OFFSET_INC(_PSIDR_, 1)))

/*
@function    setPMRUR(pmrur)
@brief      PMRUR 레지스터에 값을 설정합니다.
@param      pmrur: 설정할 값 또는 소켓 번호
@return     void
*/
#define setPMRUR(pmrur)                                                        \
  do {                                                                         \
    WIZCHIP_WRITE(_PMRUR_, (uint8_t)((pmrur) >> 8));                           \
    WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(_PMRUR_, 1), (uint8_t)(pmrur));           \
  } while (0);

/*
@function    getPMRUR()
@brief      PMRUR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getPMRUR()                                                             \
  ((((uint16_t)WIZCHIP_READ(_PMRUR_)) << 8) +                                  \
   WIZCHIP_READ(WIZCHIP_OFFSET_INC(_PMRUR_, 1)))

/*
@function    setSHAR(shar)
@brief      SHAR 레지스터에 값을 설정합니다.
@param      shar: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSHAR(shar) WIZCHIP_WRITE_BUF(_SHAR_, (shar), 6)

/*
@function    getSHAR(shar)
@brief      SHAR 레지스터의 값을 읽어옵니다.
@param      shar: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSHAR(shar) WIZCHIP_READ_BUF(_SHAR_, (shar), 6)

/*
@function    setGAR(gar)
@brief      GAR 레지스터에 값을 설정합니다.
@param      gar: 설정할 값 또는 소켓 번호
@return     void
*/
#define setGAR(gar) WIZCHIP_WRITE_BUF(_GAR_, (gar), 4)

/*
@function    getGAR(gar)
@brief      GAR 레지스터의 값을 읽어옵니다.
@param      gar: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getGAR(gar) WIZCHIP_READ_BUF(_GAR_, (gar), 4)

/*
@function    setGA4R(ga4r)
@brief      GA4R 레지스터에 값을 설정합니다.
@param      ga4r: 설정할 값 또는 소켓 번호
@return     void
*/
#define setGA4R(ga4r) setGAR(ga4r)
/*
@function    getGA4R(ga4r)
@brief      GA4R 레지스터의 값을 읽어옵니다.
@param      ga4r: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getGA4R(ga4r) getGAR(ga4r)

/*
@function    setSUBR(subr)
@brief      SUBR 레지스터에 값을 설정합니다.
@param      subr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSUBR(subr) WIZCHIP_WRITE_BUF(_SUBR_, (subr), 4)

/*
@function    getSUBR(subr)
@brief      SUBR 레지스터의 값을 읽어옵니다.
@param      subr: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSUBR(subr) WIZCHIP_READ_BUF(_SUBR_, (subr), 4)

/*
@function    setSUB4R(sub4r)
@brief      SUB4R 레지스터에 값을 설정합니다.
@param      sub4r: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSUB4R(sub4r) setSUBR(sub4r)
/*
@function    getSUB4R(sub4r)
@brief      SUB4R 레지스터의 값을 읽어옵니다.
@param      sub4r: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSUB4R(sub4r) getSUBR(sub4r)

/*
@function    setSIPR(sipr)
@brief      SIPR 레지스터에 값을 설정합니다.
@param      sipr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSIPR(sipr) WIZCHIP_WRITE_BUF(_SIPR_, (sipr), 4)

/*
@function    getSIPR(sipr)
@brief      SIPR 레지스터의 값을 읽어옵니다.
@param      sipr: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSIPR(sipr) WIZCHIP_READ_BUF(_SIPR_, (sipr), 4)

/*
@function    setLLAR(llar)
@brief      LLAR 레지스터에 값을 설정합니다.
@param      llar: 설정할 값 또는 소켓 번호
@return     void
*/
#define setLLAR(llar) WIZCHIP_WRITE_BUF(_LLAR_, (llar), 16)

/*
@function    getLLAR(llar)
@brief      LLAR 레지스터의 값을 읽어옵니다.
@param      llar: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getLLAR(llar) WIZCHIP_READ_BUF(_LLAR_, (llar), 16)

/*
@function    setGUAR(guar)
@brief      GUAR 레지스터에 값을 설정합니다.
@param      guar: 설정할 값 또는 소켓 번호
@return     void
*/
#define setGUAR(guar) WIZCHIP_WRITE_BUF(_GUAR_, (guar), 16)

/*
@function    getGUAR(guar)
@brief      GUAR 레지스터의 값을 읽어옵니다.
@param      guar: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getGUAR(guar) WIZCHIP_READ_BUF(_GUAR_, (guar), 16)

/*
@function    setSUB6R(sub6r)
@brief      SUB6R 레지스터에 값을 설정합니다.
@param      sub6r: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSUB6R(sub6r) WIZCHIP_WRITE_BUF(_SUB6R_, (sub6r), 16)

/*
@function    getSUB6R(sub6r)
@brief      SUB6R 레지스터의 값을 읽어옵니다.
@param      sub6r: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSUB6R(sub6r) WIZCHIP_READ_BUF(_SUB6R_, (sub6r), 16)

/*
@function    setGA6R(ga6r)
@brief      GA6R 레지스터에 값을 설정합니다.
@param      ga6r: 설정할 값 또는 소켓 번호
@return     void
*/
#define setGA6R(ga6r) WIZCHIP_WRITE_BUF(_GA6R_, (ga6r), 16)

/*
@function    getGA6R(ga6r)
@brief      GA6R 레지스터의 값을 읽어옵니다.
@param      ga6r: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getGA6R(ga6r) WIZCHIP_READ_BUF(_GA6R_, (ga6r), 16)

/*
@function    setSLDIPR(sldipr)
@brief      SLDIPR 레지스터에 값을 설정합니다.
@param      sldipr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSLDIPR(sldipr) WIZCHIP_WRITE_BUF(_SLDIPR_, (sldipr), 4)
/*
@function    setSLDIP4R(sldip4r)
@brief      SLDIP4R 레지스터에 값을 설정합니다.
@param      sldip4r: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSLDIP4R(sldip4r) setSLDIPR((sldip4r))

/*
@function    getSLDIPR(sldipr)
@brief      SLDIPR 레지스터의 값을 읽어옵니다.
@param      sldipr: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSLDIPR(sldipr) WIZCHIP_READ_BUF(_SLDIPR_, (sldipr), 4)
/*
@function    getSLDIP4R(sldip4r)
@brief      SLDIP4R 레지스터의 값을 읽어옵니다.
@param      sldip4r: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSLDIP4R(sldip4r) getSLDIPR((sldip4r))

/*
@function    setSLDIP6R(sldip6r)
@brief      SLDIP6R 레지스터에 값을 설정합니다.
@param      sldip6r: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSLDIP6R(sldip6r) WIZCHIP_WRITE_BUF(_SLDIP6R_, (sldip6r), 16)

/*
@function    getSLDIP6R(sldip6r)
@brief      SLDIP6R 레지스터의 값을 읽어옵니다.
@param      sldip6r: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSLDIP6R(sldip6r) WIZCHIP_READ_BUF(_SLDIP6R_, (sldip6r), 16)

/*
@function    getSLDHAR(sldhar)
@brief      SLDHAR 레지스터의 값을 읽어옵니다.
@param      sldhar: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSLDHAR(sldhar) WIZCHIP_READ_BUF(_SLDHAR_, (sldhar), 6)

/*
@function    setPINGIDR(pingidr)
@brief      PINGIDR 레지스터에 값을 설정합니다.
@param      pingidr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setPINGIDR(pingidr)                                                    \
  do {                                                                         \
    WIZCHIP_WRITE(_PINGIDR_, (uint8_t)((pingidr) >> 8));                       \
    WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(_PINGIDR_, 1), (uint8_t)(pingidr));       \
  } while (0);

/*
@function    getPINGIDR()
@brief      PINGIDR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getPINGIDR()                                                           \
  (((int16_t)(WIZCHIP_READ(_PINGIDR_) << 8)) +                                 \
   WIZCHIP_READ(WIZCHIP_OFFSET_INC(_PINGIDR_, 1)))

/*
@function    setPINGSEQR(pingseqr)
@brief      PINGSEQR 레지스터에 값을 설정합니다.
@param      pingseqr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setPINGSEQR(pingseqr)                                                  \
  do {                                                                         \
    WIZCHIP_WRITE(_PINGSEQR_, (uint8_t)((pingseqr) >> 8));                     \
    WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(_PINGSEQR_, 1), (uint8_t)(pingseqr));     \
  } while (0);

/*
@function    getPINGSEQR()
@brief      PINGSEQR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getPINGSEQR()                                                          \
  (((int16_t)(WIZCHIP_READ(_PINGSEQR_) << 8)) +                                \
   WIZCHIP_READ(WIZCHIP_OFFSET_INC(_PINGSEQR_, 1)))

/*
@function    getUIPR(uipr)
@brief      UIPR 레지스터의 값을 읽어옵니다.
@param      uipr: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getUIPR(uipr) WIZCHIP_READ_BUF(_UIPR_, (uipr), 4)

/*
@function    getUIP4R(uip4r)
@brief      UIP4R 레지스터의 값을 읽어옵니다.
@param      uip4r: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getUIP4R(uip4r) getUIPR(uip4r)

/*
@function    getUPORTR()
@brief      UPORTR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getUPORTR()                                                            \
  ((((uint16_t)WIZCHIP_READ(_UPORTR_)) << 8) +                                 \
   WIZCHIP_READ(WIZCHIP_OFFSET_INC(_UPORTR_, 1)))

/*
@function    getUPORT4R()
@brief      UPORT4R 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getUPORT4R() getUPORTR()

/*
@function    getUIP6R(uip6r)
@brief      UIP6R 레지스터의 값을 읽어옵니다.
@param      uip6r: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getUIP6R(uip6r) WIZCHIP_READ_BUF(_UIP6R_, (uip6r), 16)

/*
@function    getUPORT6R(uport6r)
@brief      UPORT6R 레지스터의 값을 읽어옵니다.
@param      uport6r: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getUPORT6R(uport6r)                                                    \
  ((((uint16_t)WIZCHIP_READ(_UPORT6R_)) << 8) +                                \
   WIZCHIP_READ(WIZCHIP_OFFSET_INC(_UPORT6R_, 1)))

/*
@function    setINTPTMR(intptmr)
@brief      INTPTMR 레지스터에 값을 설정합니다.
@param      intptmr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setINTPTMR(intptmr)                                                    \
  do {                                                                         \
    WIZCHIP_WRITE(_INTPTMR_, (uint8_t)((intptmr) >> 8));                       \
    WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(_INTPTMR_, 1), (uint8_t)(intptmr));       \
  } while (0);

/*
@function    getINTPTMR()
@brief      INTPTMR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getINTPTMR()                                                           \
  ((((uint16_t)WIZCHIP_READ(_INTPTMR_)) << 8) +                                \
   WIZCHIP_READ(WIZCHIP_OFFSET_INC(_INTPTMR_, 1)))

/*
@function    getPLR()
@brief      PLR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getPLR() WIZCHIP_READ(_PLR_)

/*
@function    getPFR()
@brief      PFR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getPFR() WIZCHIP_READ(_PFR_)

/*
@function    getVLTR()
@brief      VLTR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getVLTR()                                                              \
  ((((uint32_t)WIZCHIP_READ(_VLTR_)) << 24) +                                  \
   (((uint32_t)WIZCHIP_READ(WIZCHIP_OFFSET_INC(_VLTR_, 1))) << 16) +           \
   (((uint32_t)WIZCHIP_READ(WIZCHIP_OFFSET_INC(_VLTR_, 2))) << 16) +           \
   (((uint32_t)WIZCHIP_READ(WIZCHIP_OFFSET_INC(_VLTR_, 3))) << 16))

/*
@function    getPLTR()
@brief      PLTR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getPLTR()                                                              \
  ((((uint32_t)WIZCHIP_READ(_PLTR_)) << 24) +                                  \
   (((uint32_t)WIZCHIP_READ(WIZCHIP_OFFSET_INC(_PLTR_, 1))) << 16) +           \
   (((uint32_t)WIZCHIP_READ(WIZCHIP_OFFSET_INC(_PLTR_, 2))) << 16) +           \
   (((uint32_t)WIZCHIP_READ(WIZCHIP_OFFSET_INC(_PLTR_, 3))) << 16))

/*
@function    getPAR(par)
@brief      PAR 레지스터의 값을 읽어옵니다.
@param      par: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getPAR(par) WIZCHIP_READ_BUF(_PAR_, (par), 16)

/*
@function    setICMP6BLKR(icmp6blkr)
@brief      ICMP6BLKR 레지스터에 값을 설정합니다.
@param      icmp6blkr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setICMP6BLKR(icmp6blkr) WIZCHIP_WRITE(_ICMP6BLKR_, (icmp6blkr))

/*
@function    getICMP6BLKR()
@brief      ICMP6BLKR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getICMP6BLKR() WIZCHIP_READ(_ICMP6BLKR_)

/*
@function    setCHPLCKR(chplckr)
@brief      CHPLCKR 레지스터에 값을 설정합니다.
@param      chplckr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setCHPLCKR(chplckr) WIZCHIP_WRITE(_CHPLCKR_, (chplckr))

/*
@function    getCHPLCKR()
@brief      CHPLCKR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getCHPLCKR() ((getSYSR() & SYSR_CHPL) >> 7)

#define CHIPLOCK() setCHPLCKR(0xFF)
#define CHIPUNLOCK() setCHPLCKR(0xCE)

/*
@function    setNETLCKR(netlckr)
@brief      NETLCKR 레지스터에 값을 설정합니다.
@param      netlckr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setNETLCKR(netlckr) WIZCHIP_WRITE(_NETLCKR_, (netlckr))

/*
@function    getNETLCKR()
@brief      NETLCKR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getNETLCKR() ((getSYSR() & SYSR_NETL) >> 6)

#define NETLOCK() setNETLCKR(0xC5)
#define NETUNLOCK() setNETLCKR(0x3A)

/*
@function    setPHYLCKR(phylckr)
@brief      PHYLCKR 레지스터에 값을 설정합니다.
@param      phylckr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setPHYLCKR(phylckr) WIZCHIP_WRITE(_PHYLCKR_, (phylckr))

/*
@function    getPHYLCKR()
@brief      PHYLCKR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getPHYLCKR() ((getSYSR() & SYSR_PHYL) >> 5)

#define PHYLOCK() setPHYLCKR(0xFF)
#define PHYUNLOCK() setPHYLCKR(0x53)

/*
@function    setRTR(rtr)
@brief      RTR 레지스터에 값을 설정합니다.
@param      rtr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setRTR(rtr)                                                            \
  do {                                                                         \
    WIZCHIP_WRITE(_RTR_, (uint8_t)((rtr) >> 8));                               \
    WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(_RTR_, 1), (uint8_t)(rtr));               \
  } while (0);

/*
@function    getRTR()
@brief      RTR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getRTR()                                                               \
  ((((uint16_t)WIZCHIP_READ(_RTR_)) << 8) +                                    \
   WIZCHIP_READ(WIZCHIP_OFFSET_INC(_RTR_, 1)))

/*
@function    setRCR(rcr)
@brief      RCR 레지스터에 값을 설정합니다.
@param      rcr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setRCR(rcr) WIZCHIP_WRITE(_RCR_, (rcr))

/*
@function    getRCR()
@brief      RCR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getRCR() WIZCHIP_READ(_RCR_)

/*
@function    setSLRTR(slrtr)
@brief      SLRTR 레지스터에 값을 설정합니다.
@param      slrtr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSLRTR(slrtr)                                                        \
  do {                                                                         \
    WIZCHIP_WRITE(_SLRTR_, (uint8_t)((slrtr) >> 8));                           \
    WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(_SLRTR_, 1), (uint8_t)(slrtr));           \
  } while (0);

/*
@function    getSLRTR()
@brief      SLRTR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getSLRTR()                                                             \
  ((((uint16_t)WIZCHIP_READ(_SLRTR_)) << 8) +                                  \
   WIZCHIP_READ(WIZCHIP_OFFSET_INC(_SLRTR_, 1)))

/*
@function    setSLRCR(slrcr)
@brief      SLRCR 레지스터에 값을 설정합니다.
@param      slrcr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSLRCR(slrcr) WIZCHIP_WRITE(_SLRCR_, (slrcr))

/*
@function    getSLRCR()
@brief      SLRCR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getSLRCR() WIZCHIP_READ(_SLRCR_)

/*
@function    setSLHOPR(slhopr)
@brief      SLHOPR 레지스터에 값을 설정합니다.
@param      slhopr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSLHOPR(slhopr) WIZCHIP_WRITE(_SLHOPR_, (slhopr))

/*
@function    getSLHOPR()
@brief      SLHOPR 레지스터의 값을 읽어옵니다.
@param      void
@return     읽어온 레지스터 값
*/
#define getSLHOPR() WIZCHIP_READ(_SLHOPR_)
/**
    @}
*/

////////////////////////////////////
// SOCKETn  register I/O function //
////////////////////////////////////
/**
    @addtogroup Socket_register_access_function_W6100
    @{
*/
#define setSn_MR(sn, mr) WIZCHIP_WRITE(_Sn_MR_(sn), (mr))
/*
@function    getSn_MR(sn)
@brief      Sn_MR 레지스터의 값을 읽어옵니다.
@param      sn: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSn_MR(sn) WIZCHIP_READ(_Sn_MR_(sn))

/*
@function    setSn_PSR(sn,psr)
@brief      Sn_PSR 에 값을 설정합니다.
@param      sn: 설정할 값 또는 소켓 번호
@param      psr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSn_PSR(sn, psr) WIZCHIP_WRITE(_Sn_PSR_(sn), (psr))
/*
@function    getSn_PSR(sn)
@brief      Sn_PSR 레지스터의 값을 읽어옵니다.
@param      sn: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSn_PSR(sn) WIZCHIP_READ(_Sn_PSR_(sn))

/*
@function    setSn_CR(sn,cr)
@brief      Sn_CR 에 값을 설정합니다.
@param      sn: 설정할 값 또는 소켓 번호
@param      cr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSn_CR(sn, cr) WIZCHIP_WRITE(_Sn_CR_(sn), (cr))
/*
@function    getSn_CR(sn)
@brief      Sn_CR 레지스터의 값을 읽어옵니다.
@param      sn: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSn_CR(sn) WIZCHIP_READ(_Sn_CR_(sn))

/*
@function    getSn_IR(sn)
@brief      Sn_IR 레지스터의 값을 읽어옵니다.
@param      sn: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSn_IR(sn) WIZCHIP_READ(_Sn_IR_(sn))

/*
@function    setSn_IMR(sn,imr)
@brief      Sn_IMR 에 값을 설정합니다.
@param      sn: 설정할 값 또는 소켓 번호
@param      imr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSn_IMR(sn, imr) WIZCHIP_WRITE(_Sn_IMR_(sn), (imr))
/*
@function    getSn_IMR(sn)
@brief      Sn_IMR 레지스터의 값을 읽어옵니다.
@param      sn: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSn_IMR(sn) WIZCHIP_READ(_Sn_IMR_(sn))

/*
@function    setSn_IRCLR(sn,irclr)
@brief      Sn_IRCLR 에 값을 설정합니다.
@param      sn: 설정할 값 또는 소켓 번호
@param      irclr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSn_IRCLR(sn, irclr) WIZCHIP_WRITE(_Sn_IRCLR_(sn), (irclr))
/*
@function    setSn_IR(sn,ir)
@brief      Sn_IR 에 값을 설정합니다.
@param      sn: 설정할 값 또는 소켓 번호
@param      ir: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSn_IR(sn, ir) setSn_IRCLR(sn, (ir))

/*
@function    getSn_SR(sn)
@brief      Sn_SR 레지스터의 값을 읽어옵니다.
@param      sn: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSn_SR(sn) WIZCHIP_READ(_Sn_SR_(sn))

/*
@function    getSn_ESR(sn)
@brief      Sn_ESR 레지스터의 값을 읽어옵니다.
@param      sn: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSn_ESR(sn) WIZCHIP_READ(_Sn_ESR_(sn))

/*
@function    setSn_PNR(sn,pnr)
@brief      Sn_PNR 에 값을 설정합니다.
@param      sn: 설정할 값 또는 소켓 번호
@param      pnr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSn_PNR(sn, pnr) WIZCHIP_WRITE(_Sn_PNR_(sn), (pnr))
/*
@function    setSn_NHR(sn,nhr)
@brief      Sn_NHR 에 값을 설정합니다.
@param      sn: 설정할 값 또는 소켓 번호
@param      nhr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSn_NHR(sn, nhr) setSn_PNR(_Sn_PNR_(sn), (nhr))

/*
@function    getSn_PNR(sn)
@brief      Sn_PNR 레지스터의 값을 읽어옵니다.
@param      sn: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSn_PNR(sn) WIZCHIP_READ(_Sn_PNR_(sn))
/*
@function    getSn_NHR(sn)
@brief      Sn_NHR 레지스터의 값을 읽어옵니다.
@param      sn: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSn_NHR(sn) getSn_PNR(sn)

/*
@function    setSn_TOSR(sn,tosr)
@brief      Sn_TOSR 에 값을 설정합니다.
@param      sn: 설정할 값 또는 소켓 번호
@param      tosr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSn_TOSR(sn, tosr) WIZCHIP_WRITE(_Sn_TOSR_(sn), (tosr))
/*
@function    getSn_TOSR(sn)
@brief      Sn_TOSR 레지스터의 값을 읽어옵니다.
@param      sn: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSn_TOSR(sn) WIZCHIP_READ(_Sn_TOSR_(sn))
/*
@function    getSn_TOS(sn)
@brief      Sn_TOS 레지스터의 값을 읽어옵니다.
@param      sn: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSn_TOS(sn) getSn_TOSR(sn) ///< ioLibrary 호환용
/*
@function    setSn_TOS(sn,tos)
@brief      Sn_TOS 에 값을 설정합니다.
@param      sn: 설정할 값 또는 소켓 번호
@param      tos: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSn_TOS(sn, tos) setSn_TOSR(sn, tos) ///< ioLibrary 호환용

/*
@function    setSn_TTLR(sn,ttlr)
@brief      Sn_TTLR 에 값을 설정합니다.
@param      sn: 설정할 값 또는 소켓 번호
@param      ttlr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSn_TTLR(sn, ttlr) WIZCHIP_WRITE(_Sn_TTLR_(sn), (ttlr))
/*
@function    setSn_TTL(sn,ttl)
@brief      Sn_TTL 에 값을 설정합니다.
@param      sn: 설정할 값 또는 소켓 번호
@param      ttl: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSn_TTL(sn, ttl) setSn_TTLR(sn, ttl) ///< ioLibrary 호환용y

/*
@function    getSn_TTLR(sn)
@brief      Sn_TTLR 레지스터의 값을 읽어옵니다.
@param      sn: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSn_TTLR(sn) WIZCHIP_READ(_Sn_TTLR_(sn))
/*
@function    getSn_TTL(sn)
@brief      Sn_TTL 레지스터의 값을 읽어옵니다.
@param      sn: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSn_TTL(sn) getSn_TTLR(sn) ///< ioLibrary 호환용y

/*
@function    setSn_HOPR(sn,hopr)
@brief      Sn_HOPR 에 값을 설정합니다.
@param      sn: 설정할 값 또는 소켓 번호
@param      hopr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSn_HOPR(sn, hopr)      setSn_TTLR(sn),(ttlr))
/*
@function    getSn_HOPR(sn)
@brief      Sn_HOPR 레지스터의 값을 읽어옵니다.
@param      sn: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSn_HOPR(sn) getSn_TTLR(sn)

/*
@function    setSn_FRGR(sn,frgr)
@brief      Sn_FRGR 에 값을 설정합니다.
@param      sn: 설정할 값 또는 소켓 번호
@param      frgr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSn_FRGR(sn, frgr)                                                   \
  do {                                                                         \
    WIZCHIP_WRITE(_Sn_FRGR_(sn), (uint8_t)((frgr) >> 8));                      \
    WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(_Sn_FRGR_(sn), 1), (uint8_t)(frgr));      \
  } while (0);
/*
@function    getSn_FRGR(sn,frgr)
@brief      Sn_FRGR 값을 읽어옵니다.
@param      sn: 설정할 값 또는 소켓 번호
@param      frgr: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSn_FRGR(sn, frgr)                                                   \
  ((((uint16_t)WIZCHIP_READ(_Sn_FRGR_(sn))) << 8) +                            \
   WIZCHIP_READ(WIZCHIP_OFFSET_INC(_Sn_FRGR_(sn), 1)))

/*
@function    setSn_MSSR(sn,mssr)
@brief      Sn_MSSR 에 값을 설정합니다.
@param      sn: 설정할 값 또는 소켓 번호
@param      mssr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSn_MSSR(sn, mssr)                                                   \
  do {                                                                         \
    WIZCHIP_WRITE(_Sn_MSSR_(sn), (uint8_t)((mssr) >> 8));                      \
    WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(_Sn_MSSR_(sn), 1), (uint8_t)(mssr));      \
  } while (0);
/*
@function    getSn_MSSR(sn)
@brief      Sn_MSSR 레지스터의 값을 읽어옵니다.
@param      sn: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSn_MSSR(sn)                                                         \
  ((((uint16_t)WIZCHIP_READ(_Sn_MSSR_(sn))) << 8) +                            \
   WIZCHIP_READ(WIZCHIP_OFFSET_INC(_Sn_MSSR_(sn), 1)))

/*
@function    setSn_PORTR(sn,portr)
@brief      Sn_PORTR 에 값을 설정합니다.
@param      sn: 설정할 값 또는 소켓 번호
@param      portr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSn_PORTR(sn, portr)                                                 \
  do {                                                                         \
    WIZCHIP_WRITE(_Sn_PORTR_(sn), (uint8_t)((portr) >> 8));                    \
    WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(_Sn_PORTR_(sn), 1), (uint8_t)(portr));    \
  } while (0);
/*
@function    getSn_PORTR(sn)
@brief      Sn_PORTR 레지스터의 값을 읽어옵니다.
@param      sn: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSn_PORTR(sn)                                                        \
  ((((uint16_t)WIZCHIP_READ(_Sn_PORTR_(sn))) << 8) +                           \
   WIZCHIP_READ(WIZCHIP_OFFSET_INC(_Sn_PORTR_(sn), 1)))

/*
@function    setSn_DHAR(sn,dhar)
@brief      Sn_DHAR 에 값을 설정합니다.
@param      sn: 설정할 값 또는 소켓 번호
@param      dhar: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSn_DHAR(sn, dhar) WIZCHIP_WRITE_BUF(_Sn_DHAR_(sn), (dhar), 6)
/*
@function    getSn_DHAR(sn,dhar)
@brief      Sn_DHAR 값을 읽어옵니다.
@param      sn: 설정할 값 또는 소켓 번호
@param      dhar: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSn_DHAR(sn, dhar) WIZCHIP_READ_BUF(_Sn_DHAR_(sn), (dhar), 6)

/*
@function    setSn_DIPR(sn,dipr)
@brief      Sn_DIPR 에 값을 설정합니다.
@param      sn: 설정할 값 또는 소켓 번호
@param      dipr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSn_DIPR(sn, dipr) WIZCHIP_WRITE_BUF(_Sn_DIPR_(sn), (dipr), 4)
/*
@function    getSn_DIPR(sn,dipr)
@brief      Sn_DIPR 값을 읽어옵니다.
@param      sn: 설정할 값 또는 소켓 번호
@param      dipr: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSn_DIPR(sn, dipr) WIZCHIP_READ_BUF(_Sn_DIPR_(sn), (dipr), 4)

/*
@function    setSn_DIP4R(sn,dipr)
@brief      Sn_DIP4R 에 값을 설정합니다.
@param      sn: 설정할 값 또는 소켓 번호
@param      dipr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSn_DIP4R(sn, dipr) setSn_DIPR(sn, (dipr))
/*
@function    getSn_DIP4R(sn,dipr)
@brief      Sn_DIP4R 값을 읽어옵니다.
@param      sn: 설정할 값 또는 소켓 번호
@param      dipr: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSn_DIP4R(sn, dipr) getSn_DIPR(sn, (dipr))

/*
@function    setSn_DIP6R(sn,dip6r)
@brief      Sn_DIP6R 에 값을 설정합니다.
@param      sn: 설정할 값 또는 소켓 번호
@param      dip6r: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSn_DIP6R(sn, dip6r) WIZCHIP_WRITE_BUF(_Sn_DIP6R_(sn), (dip6r), 16)
/*
@function    getSn_DIP6R(sn,dip6r)
@brief      Sn_DIP6R 값을 읽어옵니다.
@param      sn: 설정할 값 또는 소켓 번호
@param      dip6r: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSn_DIP6R(sn, dip6r) WIZCHIP_READ_BUF(_Sn_DIP6R_(sn), (dip6r), 16)

/*
@function    setSn_DPORTR(sn,dportr)
@brief      Sn_DPORTR 에 값을 설정합니다.
@param      sn: 설정할 값 또는 소켓 번호
@param      dportr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSn_DPORTR(sn, dportr)                                               \
  do {                                                                         \
    WIZCHIP_WRITE(_Sn_DPORTR_(sn), (uint8_t)((dportr) >> 8));                  \
    WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(_Sn_DPORTR_(sn), 1), (uint8_t)(dportr));  \
  } while (0);
/*
@function    getSn_DPORTR(sn)
@brief      Sn_DPORTR 레지스터의 값을 읽어옵니다.
@param      sn: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSn_DPORTR(sn)                                                       \
  ((((uint16_t)WIZCHIP_READ(_Sn_DPORTR_(sn))) << 8) +                          \
   WIZCHIP_READ(WIZCHIP_OFFSET_INC(_Sn_DPORTR_(sn), 1)))
/*
@function    getSn_DPORT(sn)
@brief      Sn_DPORT 레지스터의 값을 읽어옵니다.
@param      sn: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSn_DPORT(sn) getSn_DPORTR(sn)
/*
@function    setSn_DPORT(sn,dportr)
@brief      Sn_DPORT 에 값을 설정합니다.
@param      sn: 설정할 값 또는 소켓 번호
@param      dportr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSn_DPORT(sn, dportr) setSn_DPORTR(sn, dportr)

/*
@function    setSn_MR2(sn,mr2)
@brief      Sn_MR2 에 값을 설정합니다.
@param      sn: 설정할 값 또는 소켓 번호
@param      mr2: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSn_MR2(sn, mr2) WIZCHIP_WRITE(_Sn_MR2_(sn), (mr2))
/*
@function    getSn_MR2(sn)
@brief      Sn_MR2 레지스터의 값을 읽어옵니다.
@param      sn: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSn_MR2(sn) WIZCHIP_READ(_Sn_MR2_(sn))

/*
@function    setSn_RTR(sn,rtr)
@brief      Sn_RTR 에 값을 설정합니다.
@param      sn: 설정할 값 또는 소켓 번호
@param      rtr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSn_RTR(sn, rtr)                                                     \
  do {                                                                         \
    WIZCHIP_WRITE(_Sn_RTR_(sn), (uint8_t)((rtr) >> 8));                        \
    WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(_Sn_RTR_(sn), 1), (uint8_t)(rtr));        \
  } while (0);
/*
@function    getSn_RTR(sn)
@brief      Sn_RTR 레지스터의 값을 읽어옵니다.
@param      sn: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSn_RTR(sn)                                                          \
  ((((uint16_t)WIZCHIP_READ(_Sn_RTR_(sn))) << 8) +                             \
   WIZCHIP_READ(WIZCHIP_OFFSET_INC(_Sn_RTR_(sn), 1)))

/*
@function    setSn_RCR(sn,rcr)
@brief      Sn_RCR 에 값을 설정합니다.
@param      sn: 설정할 값 또는 소켓 번호
@param      rcr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSn_RCR(sn, rcr) WIZCHIP_WRITE(_Sn_RCR_(sn), (rcr))
/*
@function    getSn_RCR(sn)
@brief      Sn_RCR 레지스터의 값을 읽어옵니다.
@param      sn: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSn_RCR(sn) WIZCHIP_READ(_Sn_RCR_(sn))

/*
@function    setSn_KPALVTR(sn,kpalvtr)
@brief      Sn_KPALVTR 에 값을 설정합니다.
@param      sn: 설정할 값 또는 소켓 번호
@param      kpalvtr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSn_KPALVTR(sn, kpalvtr) WIZCHIP_WRITE(_Sn_KPALVTR_(sn), (kpalvtr))
/**
    @function    uint16_t getSn_TX_FSR(uint8_t sn)
    @brief      SOCKETn의 송신(TX) 버퍼에서 비어 있는(사용 가능한) 공간의 크기를
   읽어옵니다.
    @param      sn: 소켓 번호 (0 ~ _WIZCHIP_SOCK_NUM_)
    @return     송신 버퍼의 여유 공간 크기 (바이트 단위)
*/
uint16_t getSn_TX_FSR(uint8_t sn);
/*
@function    getSn_KPALVTR(sn)
@brief      Sn_KPALVTR 레지스터의 값을 읽어옵니다.
@param      sn: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSn_KPALVTR(sn) WIZCHIP_READ(_Sn_KPALVTR_(sn))

/*
@function    setSn_TX_BSR(sn, tmsr)
@brief      Sn_TX_BSR 에 값을 설정합니다.
@param      sn: 설정할 값 또는 소켓 번호
@param      tmsr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSn_TX_BSR(sn, tmsr) WIZCHIP_WRITE(_Sn_TX_BSR_(sn), (tmsr))
/*
@function    setSn_TXBUF_SIZE(sn, tmsr)
@brief      Sn_TXBUF_SIZE 에 값을 설정합니다.
@param      sn: 설정할 값 또는 소켓 번호
@param      tmsr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSn_TXBUF_SIZE(sn, tmsr) setSn_TX_BSR(sn, (tmsr))

/*
@function    getSn_TX_BSR(sn)
@brief      Sn_TX_BSR 레지스터의 값을 읽어옵니다.
@param      sn: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSn_TX_BSR(sn) WIZCHIP_READ(_Sn_TX_BSR_(sn))
/*
@function    getSn_TXBUF_SIZE(sn)
@brief      Sn_TXBUF_SIZE 레지스터의 값을 읽어옵니다.
@param      sn: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSn_TXBUF_SIZE(sn) getSn_TX_BSR(sn)

/*
@function    getSn_TxMAX(sn)
@brief      Sn_TxMAX 레지스터의 값을 읽어옵니다.
@param      sn: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSn_TxMAX(sn) (getSn_TX_BSR(sn) << 10)

/*
@function    getSn_TX_RD(sn)
@brief      Sn_TX_RD 레지스터의 값을 읽어옵니다.
@param      sn: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSn_TX_RD(sn)                                                        \
  ((((uint16_t)WIZCHIP_READ(_Sn_TX_RD_(sn))) << 8) +                           \
   WIZCHIP_READ(WIZCHIP_OFFSET_INC(_Sn_TX_RD_(sn), 1)))

/*
@function    setSn_TX_WR(sn,txwr)
@brief      Sn_TX_WR 에 값을 설정합니다.
@param      sn: 설정할 값 또는 소켓 번호
@param      txwr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSn_TX_WR(sn, txwr)                                                  \
  do {                                                                         \
    WIZCHIP_WRITE(_Sn_TX_WR_(sn), (uint8_t)((txwr) >> 8));                     \
    WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(_Sn_TX_WR_(sn), 1), (uint8_t)(txwr));     \
  } while (0);
/*
@function    getSn_TX_WR(sn)
@brief      Sn_TX_WR 레지스터의 값을 읽어옵니다.
@param      sn: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSn_TX_WR(sn)                                                        \
  (((uint16_t)WIZCHIP_READ(_Sn_TX_WR_(sn)) << 8) +                             \
   WIZCHIP_READ(WIZCHIP_OFFSET_INC(_Sn_TX_WR_(sn), 1)))
/**
    @function    uint16_t getSn_RX_RSR(uint8_t sn)
    @brief      SOCKETn의 수신(RX) 버퍼에 수신되어 있는 데이터의 크기를
   읽어옵니다.
    @param      sn: 소켓 번호 (0 ~ _WIZCHIP_SOCK_NUM_)
    @return     수신된 데이터의 크기 (바이트 단위)
*/
uint16_t getSn_RX_RSR(uint8_t sn);

/*
@function    setSn_RX_BSR(sn,rmsr)
@brief      Sn_RX_BSR 에 값을 설정합니다.
@param      sn: 설정할 값 또는 소켓 번호
@param      rmsr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSn_RX_BSR(sn, rmsr) WIZCHIP_WRITE(_Sn_RX_BSR_(sn), (rmsr))
/*
@function    setSn_RXBUF_SIZE(sn,rmsr)
@brief      Sn_RXBUF_SIZE 에 값을 설정합니다.
@param      sn: 설정할 값 또는 소켓 번호
@param      rmsr: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSn_RXBUF_SIZE(sn, rmsr) setSn_RX_BSR(sn, (rmsr))

/*
@function    getSn_RX_BSR(sn)
@brief      Sn_RX_BSR 레지스터의 값을 읽어옵니다.
@param      sn: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSn_RX_BSR(sn) WIZCHIP_READ(_Sn_RX_BSR_(sn))
/*
@function    getSn_RXBUF_SIZE(sn)
@brief      Sn_RXBUF_SIZE 레지스터의 값을 읽어옵니다.
@param      sn: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSn_RXBUF_SIZE(sn) getSn_RX_BSR(sn)

/*
@function    getSn_RxMAX(sn)
@brief      Sn_RxMAX 레지스터의 값을 읽어옵니다.
@param      sn: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSn_RxMAX(sn) (getSn_RX_BSR(sn) << 10)

/*
@function    setSn_RX_RD(sn,rxrd)
@brief      Sn_RX_RD 에 값을 설정합니다.
@param      sn: 설정할 값 또는 소켓 번호
@param      rxrd: 설정할 값 또는 소켓 번호
@return     void
*/
#define setSn_RX_RD(sn, rxrd)                                                  \
  do {                                                                         \
    WIZCHIP_WRITE(_Sn_RX_RD_(sn), (uint8_t)((rxrd) >> 8));                     \
    WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(_Sn_RX_RD_(sn), 1), (uint8_t)(rxrd));     \
  } while (0);

/*
@function    getSn_RX_RD(sn)
@brief      Sn_RX_RD 레지스터의 값을 읽어옵니다.
@param      sn: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSn_RX_RD(sn)                                                        \
  (((uint16_t)WIZCHIP_READ(_Sn_RX_RD_(sn)) << 8) +                             \
   WIZCHIP_READ(WIZCHIP_OFFSET_INC(_Sn_RX_RD_(sn), 1)))

/*
@function    getSn_RX_WR(sn)
@brief      Sn_RX_WR 레지스터의 값을 읽어옵니다.
@param      sn: 설정할 값 또는 소켓 번호
@return     읽어온 레지스터 값
*/
#define getSn_RX_WR(sn)                                                        \
  (((uint16_t)WIZCHIP_READ(_Sn_RX_WR_(sn)) << 8) +                             \
   WIZCHIP_READ(WIZCHIP_OFFSET_INC(_Sn_RX_WR_(sn), 1)))
/**
    @}
*/

/////////////////////////////////////
// Sn_TXBUF & Sn_RXBUF IO function //
/////////////////////////////////////
/**
@function    void wiz_send_data(uint8_t sn, uint8_t *wizdata, uint16_t len)
@brief      시스템 메모리에 있는 데이터를 SOCKETn TX 버퍼로 복사(기록)합니다.
@param      sn: 소켓 번호 (0 ~ _WIZCHIP_SOCK_NUM_ 사이의 값)
@param      wizdata: 쓸 데이터가 담긴 버퍼 포인터
@param      len: 데이터 길이
@return     void
*/
void wiz_send_data(uint8_t sn, uint8_t *wizdata, uint16_t len);

/*
@function    void wiz_recv_data(uint8_t sn, uint8_t *wizdata, uint16_t len)
@brief      SOCKETn RX 버퍼에 수신된 데이터를 시스템 메모리로 복사(읽기)합니다.
@param      sn: 소켓 번호 (0 ~ _WIZCHIP_SOCK_NUM_ 사이의 값)
@param      wizdata: 데이터를 읽어올 버퍼 포인터
@param      len: 데이터 길이
@return     void
*/
void wiz_recv_data(uint8_t sn, uint8_t *wizdata, uint16_t len);

/*
@function    void wiz_recv_ignore(uint8_t sn, uint16_t len)
@brief      SOCKETn RX 버퍼에 수신된 데이터를 복사하지 않고 무시(버림)합니다.
@param      sn: 소켓 번호 (0 ~ _WIZCHIP_SOCK_NUM_ 사이의 값)
@param      len: 무시할 데이터 길이
@return     void
*/
void wiz_recv_ignore(uint8_t sn, uint16_t len);

#if 1
// 20231019 taylor
/*
@function    void wiz_delay_ms(uint32_t ms)
@brief      W6100 내부 100us 타이머를 사용한 지연(Delay) 함수
@param      ms: 지연시킬 밀리초 단위 시간
@return     void
*/
void wiz_delay_ms(uint32_t ms);
#endif

/// @cond DOXY_APPLY_CODE
#if (_PHY_IO_MODE_ == _PHY_IO_MODE_MII_)
/// @endcond
/*
@function    void wiz_mdio_write(uint8_t phyregaddr, uint16_t var)
@brief      MDC/MDIO 인터페이스를 통해 PHY 레지스터에 데이터를 기록합니다.
@param      phyregaddr: PHY 레지스터 주소 (예: PHYRAR_BMCR, PHYRAR_BMSR 등)
@param      var: PHY 레지스터에 쓸 데이터
@return     void
*/
void wiz_mdio_write(uint8_t phyregaddr, uint16_t var);

/*
@function    uint16_t wiz_mdio_read(uint8_t phyregaddr)
@brief      MDC/MDIO 인터페이스를 통해 PHY 레지스터의 데이터를 읽어옵니다.
@param      phyregaddr: PHY 레지스터 주소 (예: PHYRAR_BMCR, PHYRAR_BMSR 등)
@return     PHY 레지스터의 값
*/
uint16_t wiz_mdio_read(uint8_t phyregaddr);
/// @cond DOXY_APPLY_CODE
#endif
/// @endcond

/// @cond DOXY_APPLY_CODE
/// @endcond

#ifdef __cplusplus
}
#endif

#endif //_W6100_H_
