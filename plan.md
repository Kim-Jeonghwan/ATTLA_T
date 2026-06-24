# ATTLA-T BIT 및 이더넷 연동통제안 반영 구현 계획서 (Plan)

본 계획서는 초장사정 자동선회잠금장치(ATTLA-T)의 PBIT, CBIT, IBIT 수행 절차와 이더넷 연동통제안 통신 프로토콜을 구현하기 위한 세부 작업 내역입니다.

## User Review Required
> [!IMPORTANT]
> **Priority 매크로 변경**: 첨부 이미지 규격에 따라 패킷 헤더의 Priority 필드는 기존 0에서 `0x02 (일반)` 및 `0x01 (비상정지)`로 변경됩니다. 기본 패킷 전송 시 Priority는 0x02로 세팅할 계획입니다.
> **상태정보(Heartbeat) 통신 방식 변경**: 기존에는 DSP가 자발적으로 100ms마다 상태정보를 전송했으나, 요구사항("PC는 메세지 받으면 ACK 보내고 100ms마다 상태정보 요청") 및 공통 규칙("상태정보 주기메시지의 미 요청 또는 미 응답이 연속 50회 이상 되면 통신 두절로 판단")에 따라 **PC가 100ms 주기로 `ETH_CODE_HEARTBEAT`를 요청(Req)하고, DSP가 이에 응답(Rep)** 하는 구조(Polling 방식)로 통신 주도권을 완전히 변경합니다.

## Open Questions
> [!WARNING]
> 1. **CBIT 시작 조건**: CBIT 결과 전송 Flag를 켜는 `CBIT 결과 요청` 버튼에 매핑할 메시지 코드(Code)가 현재 `ETH_CODE_CBIT_REQ (0x1C)` 등 명확히 지정되지 않았습니다. 임의로 코드를 정의하여 적용할까요? (예: `0x1C` = CBIT 결과 전송 시작, `0x1B` = 중지)
> 2. **IBIT 에러 누적 방식**: IBIT를 N초간 수행할 때, "1번이라도 오류 난 항목은 오류"로 판별하기 위해 IBIT 시작 시점에 CPU1의 기존 에러 플래그를 한 번 초기화(Clear)하고 누적해야 하는지, 아니면 현재 상태를 그대로 읽어 누적 상태로 간주할지 결정이 필요합니다. (본 계획은 시작 시 리셋을 수행하는 방향으로 작성되었습니다.)

---

## Proposed Changes

### 1. PC 프로그램 (C# UI 및 통신 로직)
- **UI 컨트롤 제거 및 수정 (`MainForm.cs`, `MainForm.resx`)**:
  - 기존 Seq Num, 수동 Seq Num 전송 컨트롤, CRC Error Count UI 제거.
  - PBIT, CBIT, IBIT 결과를 각각 독립적으로 표시하도록 GroupBox 3개로 분리 (현재 분리되어 있으나 로직 분리).
  - CBIT 제어: `전송주기 입력칸`, `주기 설정 전송` 버튼 추가. `CBIT 결과 요청 (전송 시작)`, `CBIT 결과 중지` 버튼 추가.
  - IBIT 제어: `수행 시간(N초) 입력칸`, `수행 요청` 버튼, `결과 요청` 버튼 추가.
- **통신 로직 (`UdpEthProtocol.cs`, `MainForm.cs`)**:
  - Priority 필드를 0x02로 설정하도록 패킷 빌더 수정.
  - PC 내부에 100ms 주기 `Timer`를 신설하여, DSP의 `Boot Done`에 대해 ACK를 전송한 후 타이머를 시작, 100ms 단위로 `ETH_CODE_HEARTBEAT` 전송.
  - 첫 Heartbeat 응답을 받은 시점에 1회 한정으로 `ETH_CODE_PBIT_REQ` 전송.
  - IBIT 요청 시 N초를 Payload로 싣고, `IBIT Done` 수신 시 ACK 회신.

---

### 2. DSP CM 코어 (`csu_Ethernet_cm.c / h`)
- **매크로 및 변수 추가**:
  - `PriorityNormal (0x02)`, `PriorityEmerg (0x01)` 매크로 적용.
  - `ETH_CODE_CBIT_REQ (0x1C)` 신규 코드 추가.
  - `stEthControl` 내부에 `CbitTxFlag` (CBIT 결과 전송 상태), `IbitDuration` (IBIT 수행 시간) 변수 추가.
- **망 가입 및 100ms 타이머 갱신**:
  - `STATE_WAIT_BOOT_ACK` 상태에서 500ms마다 Boot Done 전송. ACK 수신 시 `STATE_JOINED`로 이동.
  - `STATE_JOINED`에서 자발적 100ms 송신을 삭제하고, PC로부터 `ETH_CODE_HEARTBEAT` 수신 시에만 응답(응답 페이로드에 270V 상태 탑재)하도록 수정.
  - 100ms마다 실행되는 `Ethernet_StateMachine()` 내부에서는 통신 두절 감시(PC의 요청이 5초간 없을 시 롤백)만 수행.
- **CBIT 수행 절차**:
  - `ETH_CODE_CBIT_SET`: N초 주기를 수신 및 업데이트하고 ACK 회신.
  - `ETH_CODE_CBIT_REQ`: `CbitTxFlag = 1` 로 설정.
  - `ETH_CODE_CBIT_STOP`: `CbitTxFlag = 0` 으로 설정.
  - `Ethernet_StateMachine` 내에서 `CbitTxFlag == 1`이고 IBIT가 비활성 상태일 때만 주기(N초)마다 `ETH_CODE_CBIT_REP` 전송.
- **IBIT 수행 절차**:
  - `ETH_CODE_IBIT_REQ`: Payload의 N초를 읽어 `IbitTimer`에 세팅하고, `IbitInProgress = 1`로 변경 후 ACK 전송 (CBIT 일시정지).
  - N초 후 타이머 만료 시 `ETH_CODE_IBIT_DONE` 전송 (ReqAck=1) 및 `IbitInProgress = 2` (대기 상태).
  - `IBIT Done`에 대한 ACK 수신 시 `IbitInProgress = 0` (CBIT 재개).
  - `ETH_CODE_IBIT_RES_REQ`: IBIT 최종 누적 결과를 `ETH_CODE_IBIT_REP`로 응답.

---

### 3. DSP CPU1 코어 (`csu_Bit.c / h`)
- **결과 누적 로직**:
  - IBIT 요구사항("수행 중 정상/오류 누적하여 1번이라도 오류 난 항목은 오류")은 현재 CPU1의 `bitInformAll`이 Fault 발생 시 Latch(한 번 1이 되면 유지됨)되는 특성을 그대로 활용하여 충족.
  - (선택 사항) IBIT 요청 시점에 기존 에러를 Clear하기 위해, CM에서 CPU1으로 IBIT Start 신호를 IPC로 보내 `Bit_FaultReset(1)`을 트리거하도록 수정 여부 검토.

---

## Verification Plan

### 수동 검증 (Manual Verification)
1. **네트워크 가입 및 100ms 통신**: PC 프로그램 실행 후 이더넷 연결 시, PC에 `Boot Done`이 뜨고 100ms 단위로 RX/TX LED가 깜빡이는지(Heartbeat 교환) 확인.
2. **PBIT 자동 수신**: 망 가입 직후 PC UI에 PBIT 결과가 자동으로 갱신되는지 확인.
3. **CBIT 주기 설정 및 전송**:
   - 주기를 2초로 설정 후 `주기 설정 전송` 클릭. UI에 업데이트 완료 메시지 표시 확인.
   - `결과 요청` 클릭 후 2초마다 CBIT UI가 갱신되는지 확인.
   - `결과 중지` 클릭 시 수신 멈춤 확인.
4. **IBIT 수행 연동**:
   - 5초 설정 후 `IBIT 수행 요청`. UI에 "진행 중" 표시되고 CBIT 수신 멈춤 확인.
   - 5초 후 "IBIT 완료" 표시되며 CBIT 재개 확인.
   - `결과 요청` 클릭 시 IBIT UI 갱신 확인.
5. **통신 두절 및 롤백**: PC에서 UDP 연결을 끊었을 때 DSP가 5초 후 다시 Boot Done 상태로 롤백하는지 확인.
