# 이더넷 드라이버(WIZnet) 정적시험 최적화 계획 (W6100 / UDP / IPv4 전용)

본 계획은 정적시험 통과를 위한 불필요 코드(타기종 호환, TCP 프로토콜, IPv6 기능)를 완전히 제거하고, `w6100`, `wizchip_conf`, `socket` 라이브러리의 파일 크기 및 함수 복잡도(Cyclomatic Complexity)를 대폭 낮추는 것을 목표로 합니다.

## User Review Required
> [!IMPORTANT]
> - 본 계획에 따라 삭제되는 코드는 향후 W5500 등 타 칩셋으로 변경하거나, TCP 프로토콜 또는 IPv6 기능 추가가 필요할 경우 원본을 참고하여 복구해야 할 수 있습니다. 
> - **공통 코드(Common Code)** 와 하드웨어 레지스터 접근 매크로 등은 안전하게 유지됩니다.
> - 수정 후 반드시 IDE 컴파일 및 동작 확인(UDP 패킷 정상 송수신 여부)을 수행해야 합니다.
> - 아래의 **계획(Plan)을 검토하시고 승인(또는 추가 의견)** 해 주시면, 즉시 실제 소스코드 파일들의 다이어트(삭제/수정) 작업을 진행하겠습니다.

---

## Proposed Changes (변경 예정 사항)

### HAL 계층: WIZnet 라이브러리 최적화 (`ATTLA_T_CPU1/HAL/`)

#### [COMPLETED] [wizchip_conf.h](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/HAL/wizchip_conf.h)
- **타기종 호환 코드 삭제:** `#if (_WIZCHIP_ == W5100)` 부터 시작되는 W5100S, W5200, W5300, W5500, W6300 관련 `#elif` 블록 및 해당 블록 내부의 기종별 선언(자료형, 매크로 등) 전면 삭제. (오직 `W6100` 분기만 유지)
- **IPv6 기능 선언 삭제:** `IPV6_ADDR_LLA`, `CNS_DAD`, `CNS_SLAAC`, `IK_SOCKL_ARP6` 등 IPv6 전용 매크로, 인터럽트 종류(Enum), `netinfo6` 등의 구조체 부분 삭제. (IPv4와 공용되는 Enum은 유지)

#### [COMPLETED] [wizchip_conf.c](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/HAL/wizchip_conf.c)
- **타기종 본문 삭제:** `wizchip_init()`, `wizchip_setnetinfo()`, `ctlwizchip()` 등 주요 함수 내부에 존재하는 `#if (_WIZCHIP_ == W5100)` 등 타 칩셋 전용 로직 및 함수 호출부 제거.
- **IPv6 전용 함수 삭제:** `wizchip_setnetinfo6()`, `wizchip_getnetinfo6()` 함수 본문 전체 삭제 및 `ctlnetservice()` 내에 존재하는 IPv6 주소 기반 분기 로직(DAD, SLAAC 등) 제거.

#### [COMPLETED] [socket.h](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/HAL/socket.h)
- **미사용 TCP API 선언 삭제:** UDP 환경에서 사용되지 않는 `listen()`, `connect()`, `send()`, `recv()`, `disconnect()` 및 이와 관련된 타기종(`_W5x00`) 함수 선언 제거.
- **IPv6 및 MACRAW 선언 삭제:** MACRAW 전용 API, 또는 IPv6 주소 할당을 위한 별도의 16바이트 구조체/함수 선언 제거. 
- **공용 코드 유지 (DHCP/DNS 포함):** 공용 소켓 에러 코드, UDP 소켓 송수신 함수(`socket`, `sendto`, `recvfrom`), 그리고 **DHCP와 DNS 구현에 필수적인 구조체 및 함수들은 모두 유지**합니다.

#### [COMPLETED] [socket.c](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/HAL/socket.c)
- **미사용 TCP API 구현부 삭제:** `listen()`, `connect()`, `send()`, `recv()`, `disconnect()` 등 TCP 소켓 연결 및 송수신 함수 본문 전체 삭제.
- **타기종 함수 삭제:** `sendto_W5x00()`, `recvfrom_W5x00()` 등 타기종 지원용 함수들 삭제. 오로지 W6100(혹은 공통) 하드웨어에 맞춰진 `sendto_W6x00()` 및 `recvfrom_W6x00()` 로직만 유지.
- **IPv6 패킷 처리 로직 분기 삭제:** `sendto()`와 `recvfrom()` 내부에 존재하는 `AF_INET6` 프로토콜 분기문 및 IPv6 주소(16바이트) 복사 처리 로직 삭제. 오직 `AF_INET` (IPv4, 4바이트) 기준의 헤더/페이로드 조립 로직만 유지.

#### [COMPLETED] [w6100.h](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/HAL/w6100.h) 및 [w6100.c](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/HAL/w6100.c)
- **타기종 코드 제로화:** 이 파일들은 기본적으로 W6100 전용이나, 헤더 인클루드 부분에 잔존할 수 있는 `#if` 타기종 예외 코드 확인 후 정리.
- **IPv6 전용 레지스터 접근 함수 삭제:** IPv6 Destination/Source 주소 설정용 레지스터(`Sn_DEST_IP6`, `Sn_SRC_IP6` 등)를 제어하는 `getSn_DEST_IP6()`, `setSn_DEST_IP6()` 류의 매크로/인라인 헬퍼 함수들과 본문들을 삭제하여 전체 함수 개수(Fan-out 등) 감소 처리.

---

## Verification Plan (검증 및 테스트 계획)
1. **코드 다이어트 후 컴파일 검증:** 삭제된 매크로나 함수로 인해 기존의 `hal_Ethernet.c` (UDP 사용부)에서 컴파일 에러나 누락된 종속성이 발생하는지 1차 확인. (에이전트가 알려드리면, 사용자님께서 직접 CCS에서 Build 수행)
2. **정적시험 기준 (DAPA SCR-G) 충족성 검토:** 삭제 작업 완료 후, `socket.c` 및 `wizchip_conf.c` 파일의 크기(Line Count)와 Cyclomatic Complexity(분기문 개수)가 눈에 띄게 줄어들었는지 확인.
3. **실 장비 UDP 통신 테스트 (사용자 수동 테스트):** 빌드 성공 이후, 화포통제컴퓨터 등 타겟 장비와의 UDP 패킷(Tx/Rx) 송수신 로직 및 핑(Ping) 응답이 정상 작동하는지 기존과 동일하게 수동으로 점검 요망.
