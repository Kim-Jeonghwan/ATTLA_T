# 이더넷 연동통제안 (ICD) 코드 반영 계획 (Implementation Plan)

전달해주신 `초장사정유무인복합자주포_체계 연동통제안_r1.md` 문서 내용과 기존 펌웨어 코드를 비교 분석한 결과, 아래와 같은 핵심적인 반영이 필요함을 확인했습니다. 

## 1. 개요 및 분석 결과
1. **패킷 구조체 메모리 정렬 (Packing)**
   - 네트워크 통신의 무결성을 보장하기 위해 1바이트 단위 패킹 지시가 명시되어 있습니다.
   - 기존 `csu_Ethernet.h`의 구조체는 컴파일러에 따라 패딩(Padding) 바이트가 삽입될 위험이 있으므로, 명시적으로 `#pragma pack(push, 1)` 및 `#pragma pack(pop)` 매크로 추가가 필요합니다.
2. **상태 머신 상태명 동기화 및 롤백 로직 수정**
   - 현재 `ETH_STATE_INIT`, `ETH_STATE_BOOT_DONE`, `ETH_STATE_LINKED`로 되어 있는 상태를 ICD에 맞춰 `STATE_BOOTING`, `STATE_WAIT_BOOT_ACK`, `STATE_JOINED`, `STATE_COMM_LOSS` 로 변경합니다.
   - Boot Done을 500ms 마다 쏘는 로직의 위치를 ICD 가이드라인(Pseudo-code)에 맞게 `STATE_WAIT_BOOT_ACK` 모드 내부로 이동시킵니다.
3. **Background 태스크와 ISR 분리 확인**
   - 수신 인터럽트(XINT1)와 백그라운드 송신(100ms)은 현재 `main.c`에 완벽히 분리되어 동작 중이므로 아키텍처적 수정은 필요 없으나, 명확한 주석과 구조 명시를 추가합니다.

> 💡 **중요 참고**
> 상태 머신 Enum의 이름 변경과 `#pragma pack(push, 1)` 적용은 전체 네트워크 데이터 직렬화 방식에 변화를 주므로, 구현 후 구조체 사이즈가 정확히 12바이트, 18바이트 등으로 산출되는지 점검해야 합니다. 

## 2. 세부 작업 (Proposed Changes)

### CSU Layer (Ethernet)

#### [MODIFY] csu_Ethernet.h
- 패킷 헤더(`stEthHeader`), Payload(`stEthMessage`), ACK(`stEthAckMessage`) 구조체 선언부 위아래에 `#pragma pack(push, 1)` 및 `#pragma pack(pop)` 추가.
- `EthState_e` 열거형 내부의 명칭을 `STATE_BOOTING`, `STATE_WAIT_BOOT_ACK`, `STATE_JOINED`, `STATE_COMM_LOSS`로 변경.

#### [MODIFY] csu_Ethernet.c
- `Ethernet_ProtocolInit()` 내의 초기 상태를 `STATE_BOOTING`으로 변경.
- `Ethernet_StateMachine()` 내부 `switch-case` 블록을 새로운 상태 이름으로 교체.
- `STATE_BOOTING` 단계에서는 초기 통신망 진입 및 Boot Done 메시지를 전송하고 즉시 `STATE_WAIT_BOOT_ACK`로 천이.
- `STATE_WAIT_BOOT_ACK` 단계에서 500ms(5 Ticks) 주기 타이머를 체크하여 ACK가 올 때까지 반복해서 `BOOT_DONE` 메시지를 재전송하도록 로직 수정.
- `STATE_JOINED` (구 LINKED) 내에서 통신 두절 에러(50회 미수신) 발생 시 `STATE_WAIT_BOOT_ACK` 모드로 롤백하도록 수정 (ICD 기준).
- 수신 ISR에서 호출되는 `Ethernet_ParsePacket()` 내 ACK 판정 블록에서 `STATE_WAIT_BOOT_ACK` 일 때 `BOOT_DONE` 응답이 오면 `STATE_JOINED`로 천이하는 로직으로 이름 변경.

## 3. 검증 계획 (Verification Plan)
- 코드 수정 후 CCS 내 빌드를 통해 C2000 TI Clang 컴파일러 규격 오류가 없는지 확인해야 합니다.
- `#pragma pack(push, 1)` 지시자가 다른 모듈이나 변수에 엉뚱한 영향을 미치지 않도록 올바른 위치에 삽입되었는지 점검합니다.
