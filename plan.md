# 🌐 ATTLA_T 프로젝트 설정 잔재 명칭 일괄 변경 계획서 (plan.md)

본 계획서는 **초장사정 자동선회잠금장치(ATTLA_T)** 프로젝트의 CM 코어 및 CPU1 코어 설정 파일과 PC 모니터링 프로그램에 남아 있는 이전 테스터 프로젝트(`TMDSCNCD28388D_T`) 관련 잔재들을 일괄 정밀 변경하기 위한 구현 계획입니다.

---

## 1. 핵심 제안 사항 및 사용자의 확인 필요 사항

1. **프로젝트명 대소문자 매칭**:
   사용자 요청에 `ATTLA_T_Cm`과 `ATTLA_T_CM`이 혼용되어 있습니다. 폴더명(`ATTLA_T_CM`) 및 기존 `ATTLA_T_CPU1`과의 정합성을 위해 프로젝트 명칭은 대문자인 **`ATTLA_T_CM`**으로 변경하겠습니다.

2. **CPU1 코어 및 PC 프로그램 확장 제안**:
   리서치 결과 CPU1 코어 및 PC 모니터링 프로그램(`ATTLA_T_PC`) 설정 및 소스에도 이전 프로젝트 명칭(`TMDSCNCD28388D_T`)이 남아 있습니다. 이 부분도 함께 변경하는 것을 권장드립니다.

---

## 2. 세부 작업 항목 (Proposed Changes)

### 2.1 CM 코어 프로젝트 설정 변경 (필수)

#### [MODIFY] [ATTLA_T_CM/.project](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CM/.project)
*   Line 3: 프로젝트 이름 `<name>TMDSCNCD28388D_T_CM copy</name>` ➡️ `<name>ATTLA_T_CM</name>`
*   Line 34: 매크로 경로 `<value>file:/D:/Nexcom/Firmware/01_Project/02_Tester/TMDSCNCD28388D_T/TMDSCNCD28388D_T/TMDSCNCD28388D_T_CM</value>` ➡️ `<value>file:/D:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CM</value>`

#### [MODIFY] [ATTLA_T_CM/.cproject](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CM/.cproject)
*   Line 8: 매크로 경로 `value="D:/Nexcom/Firmware/01_Project/02_Tester/TMDSCNCD28388D_T/TMDSCNCD28388D_T/TMDSCNCD28388D_T_CM"` ➡️ `value="D:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CM"`

---

### 2.2 CPU1 코어 프로젝트 설정 변경 (선택 권장)

#### [MODIFY] [ATTLA_T_CPU1/.cproject](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/.cproject)
*   Line 233: `TMDSCNCD28388D_T2_CPU1` ➡️ `ATTLA_T_CPU1`

#### [MODIFY] [ATTLA_T_CPU1/.ccsproject](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/.ccsproject)
*   Line 9: `TMDSCNCD28388D_T2_CPU1` ➡️ `ATTLA_T_CPU1`

---

### 2.3 PC 모니터링 프로그램 설정 및 소스코드 변경 (선택 권장)

#### [MODIFY] PC 프로그램 C# 소스 파일들 ([ATTLA_T_PC](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_PC) 폴더 하위)
*   대상 파일들: `UdpEthProtocol.cs`, `SciPcProtocol.cs`, `Program.cs`, `MainForm.cs`, `LogForm.cs`, `IProtocol.cs`, `CanProtocol.cs` 등
*   이전 네임스페이스 `TMDSCNCD28388D_T_PC` ➡️ `ATTLA_T_PC` 치환
*   UI 메인 윈도우 타이틀 등 이전 프로젝트 텍스트 수정

---

## 3. 검증 및 빌드 계획 (Verification Plan)

*   변경 사항을 적용한 뒤, **CCS Theia**를 실행해 CM 프로젝트명이 `ATTLA_T_CM`으로 갱신되었는지 확인합니다.
*   빌드 패스 등의 프로젝트 속성이 로컬의 올바른 경로(`04_ATTLA/ATTLA_T/ATTLA_T_CM`)를 가리키고 있는지 점검합니다.
*   사용자가 직접 빌드하여 문제없이 기동되는지 검증합니다.
