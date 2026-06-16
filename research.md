# ATTLA_T W6100 이더넷 드라이버 불필요 코드 조사 보고서 (Research Report)

## 1. 개요 및 분석 기준
사용자의 요청에 따라 `socket.c`, `socket.h`, `w6100.c`, `w6100.h`, `wizchip_conf.c`, `wizchip_conf.h` 파일을 조사하였습니다. 
판단 기준은 **ATTLA_T 시스템 아키텍처 명세서(Architecture.md)**의 "4. 체계 통신 및 외부 인터페이스" 내용입니다.
- **적용 통신망**: 체계 통신 (W6100 이더넷)
- **적용 프로토콜**: **UDP 통신 단일 사용** (`SOCK_UDP_COM`: 0)
- **지원하지 않는 규격**: TCP, IPv6, IPRAW, MACRAW, PPPoE 등

아키텍처 상 UDP 통신 외의 부가적인 프로토콜은 전부 불필요하므로, 소스코드 및 헤더 내에 잔존하는 해당 기능들을 걷어내면 코드 사이즈와 정적 복잡도를 크게 낮출 수 있습니다.

---

## 2. 모듈별 불필요 코드 및 주석 상세 내역

### 2.1 `socket.c` 및 `socket.h`
**UDP 전용 아키텍처에 맞지 않는 불필요한 분기문 및 상수들입니다.**
- **TCP 관련 코드 전면 제거 가능**
  - `socket.h`: `SF_TCP_FPSH`, `SF_TCP_NODELAY`, `TCPSOCK_MODE`, `TCPSOCK_OP`, `TCPSOCK_SIP`, `SO_EXTSTATUS`, `SO_REMAINSIZE`, `SO_MSS` 등 TCP 전용 소켓 플래그와 옵션 타입.
  - `socket.c`: `socket()` 함수의 `case Sn_MR_TCP:` 블록, `getsockopt()` / `setsockopt()` 의 `SO_MSS` 처리 구문, `SO_PACKINFO` 에서의 TCP 에러 리턴 처리 로직.
- **MACRAW 및 IPRAW 관련 분기 제거 가능**
  - `socket.h`: `SF_ETHER_OWN`, `SF_ETHER_MULTI4B`, `SF_ETHER_MULIT6B` 등 MAC 필터 및 IPv6 멀티캐스트 플래그.
  - `socket.c`: `socket()`, `sendto_W6x00()`, `recvfrom_W6x00()` 내부의 `case Sn_MR_MACRAW:`, `case Sn_MR_IPRAW:` 분기 블록. 이를 제거하면 UDP 통신 경로 단일화로 속도가 개선되고 복잡도(Cyclomatic Complexity)가 크게 낮아집니다.

### 2.2 `wizchip_conf.c` 및 `wizchip_conf.h`
**미사용 네트워크 서비스(IPv6, PPPoE, Socket-less) 잔재입니다.**
- **IPv6 잔재 제거**
  - `wizchip_conf.h`: `CNS_DAD`, `CNS_SLAAC`, `CNS_UNSOL_NA`, `CNS_GET_PREFIX` (IPv6 전용 서비스 제어 명령).
- **PPPoE 및 특수 모드 잔재 제거**
  - `wizchip_conf.h`: `IK_PPPOE_TERMINATED`(PPPoE 인터럽트), `NM_PPPoE`(PPPoE 모드 설정).
- **직접 호출하지 않는 Socket-less 및 부가 기능**
  - 체계통신은 상태 머신을 통해 UDP 페이로드를 주고받으므로, 에이전트 측에서의 명시적인 PING이나 ARP 호출 구조는 불필요합니다.
  - `wiz_ARP`, `wiz_PING` 구조체 및 관련 함수 (`wizchip_arp()`, `wizchip_ping()`).
  - WOL (Wake On Lan) 관련 인터럽트 및 매크로 (`IK_WOL`, `NM_WOL`).

### 2.3 `w6100.c` 및 `w6100.h`
**단일 통신 구성에 불필요한 레지스터 정의들입니다.**
- **IPv6 주소 및 제어 레지스터 정의 (`w6100.h`)**
  - `_LLAR_`, `_GUAR_`, `_SUB6R_`, `_GA6R_`, `_SLDIP6R_`, `_UIP6R_`, `_UPORT6R_`, `_ICMP6BLKR_` 등.
  - `Sn_MR_UDP6`, `Sn_MR_UDPD`, `Sn_MR_IPRAW4`, `Sn_MR_IPRAW6`, `Sn_MR_MACRAW` 등 지원하지 않는 프로토콜 모드 정의.
- **PPPoE 및 TCP/TOS 레지스터 정의 (`w6100.h`)**
  - PPPoE 제어용: `_PTMR_`, `_PMNR_`, `_PHAR_`, `_PSIDR_`, `_PMRUR_` 등.
  - TCP 혼잡제어/MSS: `_Sn_KPALVTR_`, `_Sn_MSSR_`, `_Sn_TOSR_` 등.
- **`w6100.c` 정리 대상**
  - FDM (Fixed Data Mode) 버스트 매크로 잔재 (`_WIZCHIP_SPI_FDM_LEN1_` 등) : ATTLA_T는 VDM 통신만을 사용하므로 불필요.

---

## 3. 요약 및 권장 조치사항 (Next Step)

현재 드라이버 코드는 일반적인 범용(Generic) 통신 환경을 커버하도록 작성되어 있어, **정적 시험 메트릭(복잡도 등)**을 악화시키는 거대한 `switch-case` 블록과 수백 줄의 미사용 레지스터 매크로를 포함하고 있습니다.

ATTLA_T 시스템은 **"UDP 단일 프로토콜"**만을 명시적으로 사용하므로, 위에서 조사된 **TCP, IPv6, MACRAW, IPRAW, PPPoE 관련 헤더 선언과 소스 분기문을 완전히 삭제(Hard-coding or Cleanup)하는 리팩토링**을 진행하는 것이 좋습니다. 이를 통해 `socket.c`의 `recvfrom_W6x00`이나 `sendto_W6x00` 함수의 사이클로매틱 복잡도(Cyclomatic Complexity)를 낮추고 실행 속도를 더욱 최적화할 수 있습니다.

사용자님께서 명시적으로 **"계획(Plan) 및 리팩토링 구현을 시작하라"**고 지시해주시면, 해당 코드들을 안전하게 제거하는 작업 계획서를 작성하고 구현을 진행하겠습니다.
