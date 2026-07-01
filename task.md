# 작업 목록 (Task List)

- [ ] `csu_Bit.h` / `csu_Bit.c` 리팩토링
  - [ ] `stBitLimit` 구조체 정의 및 상세 주석 작성
  - [ ] 전역 변수 `xBitLimit` 선언
  - [ ] 기존 매크로 상수 제거
  - [ ] `Bit_Init()`에서 구조체 변수 초기화
  - [ ] `csu_Bit.c` 로직 내 매크로를 구조체 변수로 치환

- [ ] `csu_LimitSwitch.h` / `csu_LimitSwitch.c` 리팩토링
  - [ ] `stLimitSwitchLimit` 구조체 정의 및 상세 주석 작성
  - [ ] 전역 변수 `xLimitSwitchLimit` 선언
  - [ ] 기존 매크로 상수 제거 (틱 변환 매크로 포함)
  - [ ] `LimitSwitch_Init()`에서 구조체 변수 초기화
  - [ ] 관련 로직(`LimitSwitch_CheckFaults` 등) 내 매크로를 구조체 변수 및 직접 연산으로 치환

- [ ] `csu_MotorCtrl.h` / `csu_MotorCtrl.c` 리팩토링
  - [ ] `stMotorLimit` 구조체 정의 및 상세 주석 작성
  - [ ] 전역 변수 `xMotorLimit` 선언
  - [ ] 기존 매크로 상수 제거 (틱 변환 매크로 포함)
  - [ ] `MotorCtrl_Init()`에서 구조체 변수 초기화
  - [ ] `MotorCtrl_Run()` 및 `csu_Pid.c` 호출부의 매크로를 구조체 변수 참조로 치환

- [ ] `walkthrough.md` 작성 및 보고
