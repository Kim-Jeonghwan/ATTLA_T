# CBIT 및 IBIT 제어 명령 이력 추가 계획 (Plan)

## 1. 개요
PC 모니터링 프로그램(`MainForm.cs`)의 "REAL-TIME LOG MONITOR" 패널(하단 로그 창) 영역에 사용자가 요청한 **CBIT, IBIT 제어 명령(설정, 시작, 중지, 요청, 결과 갱신 등)의 수행 이력**을 시간별로 기록하여 보여주는 리스트박스(ListBox)를 추가합니다.

## 2. 작업 대상 파일
- `d:\Nexcom\Firmware\01_Project\04_ATTLA\ATTLA_T\ATTLA_T\ATTLA_T_PC\MainForm.cs`

## 3. 상세 구현 계획

### [ ] 1단계: UI 컴포넌트 변수 추가
- `MainForm` 클래스의 전역 변수 선언부에 제어 명령 이력을 표시할 리스트박스 변수를 추가합니다.
  ```csharp
  private ListBox lstCommandHistory;
  ```

### [ ] 2단계: 이력 추가용 헬퍼 함수 구현
- 시간 정보와 함께 명령 이름 및 상태를 `lstCommandHistory`에 기록하는 함수를 `MainForm` 내부에 추가합니다. 메모리 누수를 방지하기 위해 최대 항목 수(예: 1000개)를 유지하도록 제한합니다.
  ```csharp
  private void AddCommandHistory(string command, string status)
  {
      if (lstCommandHistory == null || lstCommandHistory.IsDisposed) return;
      
      // UI 스레드에서 안전하게 업데이트하도록 Invoke 적용 보장
      if (this.InvokeRequired)
      {
          this.Invoke(new Action(() => AddCommandHistory(command, status)));
          return;
      }

      string timeStr = DateTime.Now.ToString("HH:mm:ss.fff");
      string logMsg = $"[{timeStr}] [{command}] {status}";
      lstCommandHistory.Items.Add(logMsg);

      // 항목 수 제한 (최대 1000개 유지)
      if (lstCommandHistory.Items.Count > 1000)
      {
          lstCommandHistory.Items.RemoveAt(0);
      }

      // 항상 최신 항목(맨 아래)으로 자동 스크롤
      lstCommandHistory.TopIndex = lstCommandHistory.Items.Count - 1;
  }
  ```

### [ ] 3단계: `BuildUI()` 내부에 UI 요소 배치
- `BuildUI()` 함수의 "4. Real-Time Log Panel" 영역(`pnlLog`) 구성 시, 아래쪽에 `lstCommandHistory`를 추가합니다.
  ```csharp
  Label lblHistoryTitle = new Label { 
      Text = "■ 제어 명령 요청 이력", 
      Location = new Point(30, 150), 
      AutoSize = true, 
      Font = new Font("맑은 고딕", 11, FontStyle.Bold), 
      ForeColor = Color.Cyan 
  };
  
  lstCommandHistory = new ListBox {
      Location = new Point(30, 180),
      Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right,
      Width = 1450, // 패널 가로 크기에 맞춰 넓게
      Height = 200, // 패널 세로 크기에 맞춤
      BackColor = Color.FromArgb(30, 30, 30),
      ForeColor = Color.LightGray,
      Font = new Font("Consolas", 11),
      BorderStyle = BorderStyle.FixedSingle
  };
  
  pnlLog.Controls.Add(lblHistoryTitle);
  pnlLog.Controls.Add(lstCommandHistory);
  ```

### [ ] 4단계: 각 버튼 클릭 이벤트 핸들러에 이력 기록 연동
- 각 제어 버튼(`btnCbitSet`, `btnCbitReq`, `btnCbitStop`, `btnIbitReq`, `btnIbitResReq`)의 이벤트 핸들러 안에 `AddCommandHistory()` 호출 로직을 끼워 넣습니다.
- 실패/차단 케이스(예: UDP 모드가 아닐 때, 올바르지 않은 값 입력 시, IBIT 수행 중 차단 시)에도 적절한 상태 메시지를 남기도록 처리합니다.
  - 예시 (`btnIbitResReq.Click`):
    ```csharp
    if (lblIbitReqStatus.Text == "IBIT 수행 중..." || lblIbitReqStatus.Text == "요청 전송 중...")
    {
        AddCommandHistory("IBIT 결과 갱신", "차단됨 (IBIT 수행 중)"); // 이력 남기기
        MessageBox.Show("IBIT 수행 중에는 결과를 갱신할 수 없습니다.", "안내", MessageBoxButtons.OK, MessageBoxIcon.Warning);
        return;
    }
    _protocol.SendEthCommand(0x1A, null);
    AddCommandHistory("IBIT 결과 갱신", "요청 전송 완료"); // 이력 남기기
    ```
  - 나머지 버튼들도 이와 동일한 패턴으로 성공/에러 케이스별로 이력을 기록하도록 수정합니다.

## 4. 확인 사항
위 계획에 동의하시면 **"구현 시작"** 등 긍정의 메시지를 알려주시기 바랍니다. 승인 즉시 코드를 수정하고 이력을 남길 수 있도록 하겠습니다.
