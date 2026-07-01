# 구현 완료 리포트: 매크로 상수의 디버그 튜닝용 구조체 변환 (Walkthrough)

## 1. 개요 (Overview)
사용자의 `plan.md` 승인 지시에 따라, 기존에 `#define`으로 하드코딩되어 있던 시스템 한계값 및 튜닝용 매크로 상수들을 CCS 디버거에서 실시간으로 변경하고 테스트할 수 있도록 전역 변수(구조체) 형태로 전환하는 작업을 완료하였습니다. 각 모듈의 독립성을 유지하기 위해 `Option 1` 방식을 채택하여 모듈별로 튜닝 구조체를 분리 적용했습니다.

## 2. 변경된 주요 파일 (Modified Files)

### 2.1 BIT 결함 진단 임계값 리팩토링
- **[csu_Bit.h](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/CSU/csu_Bit.h) / [csu_Bit.c](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/CSU/csu_Bit.c)**
  - 기존 과전류, 과열, 스톨, 과속 판단 등에 사용되던 매크로 상수들을 주석 처리하고 `stBitLimit` 구조체를 신규 정의했습니다.
  - 전역 변수 `xBitLimit`를 선언하고, `Bit_Init()` 함수 내부에서 기존 매크로와 동일한 값으로 멤버들을 초기화하도록 코드를 추가했습니다.
  - 전류/전압/속도 체크 함수들 내부의 판단 로직을 매크로 상수에서 `xBitLimit.xxx` 형태로 치환했습니다.

### 2.2 리미트 스위치 감지 설정 리팩토링
- **[csu_LimitSwitch.h](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/CSU/csu_LimitSwitch.h) / [csu_LimitSwitch.c](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/CSU/csu_LimitSwitch.c)**
  - 오프셋/데드존 거리 제한 및 센서 결함 지연시간 상수들을 주석 처리하고 `stLimitSwitchLimit` 구조체를 신규 정의했습니다.
  - 전역 변수 `xLimitSwitchLimit`를 선언하고, `LimitSwitch_Init()`에서 기본값 튜닝 세팅을 부여했습니다.
  - 제어 루프 내부에서 스위치 거리 및 지연 시간을 판단하는 로직을 구조체 멤버로 치환했습니다.

### 2.3 모터 제어 소프트 리미트 리팩토링
- **[csu_MotorCtrl.h](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/CSU/csu_MotorCtrl.h) / [csu_MotorCtrl.c](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/CSU/csu_MotorCtrl.c)**
  - 모터의 위치, 속도, 전류 상/하한선 및 브레이크 딜레이 시퀀스 타이머 매크로 상수들을 `stMotorCtrlLimit` 구조체로 변경했습니다.
  - 전역 변수 `xMotorCtrlLimit`를 선언하고 `MotorCtrl_Init()`에서 해당 값들을 초기화했습니다.
  - `PID_Init()` 시 적용되는 출력 리미트 및 런타임에 동적으로 변경되는 전류 구속 로직(`LIMIT_CURRENT_RATIO` 등)에 `xMotorCtrlLimit` 멤버 변수를 연동시켰습니다.

## 3. 유의 사항 (Important Notes)
> [!NOTE]
> PID 계수 제어를 위한 `xPidGain`과 목표 위치/속도 제어를 위한 `xMotorCtrl` 구조체는 기존에도 전역 변수 형태로 잘 구성되어 있었으므로, 별도 수정 없이 유지되었습니다.

## 4. 검증 항목 (Verification Checklist)
- [ ] **빌드 무결성 확인**: CCS 환경에서 Rebuild를 수행하여 문법 오류나 정의 누락 에러가 없는지 확인합니다.
- [ ] **실시간 디버그 튜닝 테스트**: 
  - 코드를 플래싱한 뒤 Debug 모드에 진입합니다.
  - CCS Expressions 뷰 창에 `xBitLimit`, `xLimitSwitchLimit`, `xMotorCtrlLimit`, `xPidGain`, `xMotorCtrl` 변수를 등록합니다.
  - 장비 구동 중 `xBitLimit.ovcMotMax`나 `xMotorCtrlLimit.posMax` 등의 값을 변경하여 실시간으로 리미트 동작이나 제어 반응이 바뀌는지 확인해 주십시오.
