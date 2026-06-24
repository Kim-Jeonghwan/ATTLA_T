# BIT 및 연동통제안 반영을 위한 사전 조사 보고서 (Research)

## 0. 공통 및 UI 요구사항
- **요구사항**: BIT 결과를 UI에 표기 (PBIT, CBIT, IBIT 각각 구분). 명시된 곳에서만 ACK 진행. PC UI에 각 BIT 정보 독립 업데이트. Seq Num, CRC Error Count 관련 UI 및 로직 삭제.
- **수정 대상 파일**: 
  - `MainForm.cs`, `MainForm.Designer.cs`: 기존의 Seq Num, CRC Error 관련 Label 및 UI 컨트롤 제거.
  - `UdpEthProtocol.cs`, `SciPcProtocol.cs`: Seq Num 및 CRC 파싱 로직 제거.
  - `csu_Ethernet_cm.c/h`: 기존에는 모든 패킷 수신 시 `reqAck`가 1이면 무조건 ACK를 회신했으나, 요구사항에 맞게 ACK 회신 조건 최적화 필요 (Boot Done, IBIT Done 등 특정 메시지에 대해서만).

## 1. PBIT 수행 및 통신망 가입 (Network Join)
- **요구사항 (DSP)**: 부팅 후 500ms 주기로 Boot Done 전송. PC로부터 ACK 수신 시 상태정보 요청 대기.
- **요구사항 (PC)**: Boot Done 수신 시 ACK 전송. 이후 100ms 마다 상태정보 요청 전송. 첫 상태정보 응답 이후 1회에 한해 PBIT 결과 요청.
- **수정 대상 파일**:
  - `csu_Ethernet_cm.c`: 
    - `STATE_WAIT_BOOT_ACK`는 기존 유지 (500ms 주기 전송 로직 존재).
    - `STATE_JOINED`에서 기존에는 DSP가 100ms마다 자발적으로 Heartbeat를 보냈으나, PC의 상태정보 요청(`ETH_CODE_HEARTBEAT_REQ` 등 신규 코드 필요 여부 확인, 또는 Heartbeat 패킷 수신 시 응답하는 형태로 변경)에 대한 응답만 하도록 수정해야 함.
  - `UdpEthProtocol.cs`, `MainForm.cs`:
    - PC 쪽에 100ms 타이머를 두어 100ms마다 상태정보 요청을 전송하는 로직 추가.
    - 첫 번째 상태정보 응답을 받으면 `ETH_CODE_PBIT_REQ`를 전송하는 로직 구현. (현재 `MainForm.cs`의 `OnStatusReceived`에 유사 로직 존재하나 통신 주도권이 PC로 넘어가도록 수정 필요)

## 2. CBIT 수행 절차
- **요구사항 (PC)**: 전송주기(N초) 설정 버튼 -> 메시지 전송. 결과 요청 버튼 -> 메시지 전송 (전송 Flag On). 결과 요청 중지 버튼 -> 메시지 전송 (Flag Off).
- **요구사항 (DSP)**: 전송주기 수신 시 ACK 전송. Flag On 상태일 때 해당 주기마다 결과 전송. 
- **수정 대상 파일**:
  - `csu_Ethernet_cm.h`: `stEthControl` 구조체에 `CbitTxFlag` (CBIT 전송 플래그) 추가 필요.
  - `csu_Ethernet_cm.c`:
    - `ETH_CODE_CBIT_SET` 수신 시 ACK를 회신하도록 수정.
    - `ETH_CODE_CBIT_REQ` (결과 요청) 수신 시 `CbitTxFlag = 1` 설정.
    - `ETH_CODE_CBIT_STOP` 수신 시 `CbitTxFlag = 0` 설정.
    - `STATE_JOINED` 내에서 주기 타이머 만료 시 `CbitTxFlag == 1`일 때만 `ETH_CODE_CBIT_REP` 전송.
  - `MainForm.cs`: CBIT 관련 요청, 중지 버튼 이벤트 및 UI 추가.

## 3. IBIT 수행 절차
- **요구사항 (PC)**: IBIT 시간 설정 칸, 수행 요청 버튼, 결과 요청 버튼 추가.
- **요구사항 (DSP)**: 수행 요청 수신 시 ACK 전송 -> CBIT 일시 중지 -> N초간 IBIT 수행 (에러 누적) -> IBIT Done 전송 대기. PC에서 ACK 받으면 CBIT 재개.
- **수정 대상 파일**:
  - `csu_Ethernet_cm.h`: IBIT 수행 시간(N초) 저장용 변수 추가.
  - `csu_Ethernet_cm.c`:
    - `ETH_CODE_IBIT_REQ` 수신 시 N초 설정 및 ACK 전송.
    - 수행 시뮬레이션 타이머 로직을 N초에 맞게 수정.
    - N초 만료 시 `ETH_CODE_IBIT_DONE` 전송 및 PC로부터 ACK 대기. ACK 수신 전까지 대기하다가 수신 시 CBIT 재개.
  - `MainForm.cs`: IBIT 시간 설정 텍스트박스 및 전송 버튼 연동. `OnIbitDoneReceived` 이벤트 수신 시 ACK 회신 로직 추가 (현재 `UdpEthProtocol.cs`에서 자동 처리 중인지 확인 필요).

## 요약 및 결론
- **UI 측면**: 불필요한 기능(Seq Num, CRC) 제거 및 통신 주도권 변경(PC가 주도하여 100ms 상태정보 요청)에 따른 타이머 기반 Request 로직 추가 필요.
- **펌웨어 측면 (CM 코어)**: 기존 자발적 Heartbeat 송출 로직을 PC 요청에 대한 응답 로직으로 변경. CBIT 결과 전송 플래그 도입 및 IBIT 타이머 기반 흐름 제어 강화 필요.
