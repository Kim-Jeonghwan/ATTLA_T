
추후 변경될 수 있는 TBD(To Be Determined: 추후 결정) 값들은 코드 상단에서 매크로(#define)나 설정 변수(Configuration Variables)로 쉽게 수정할 수 있도록 분리하여 배치했습니다.

요구사항 명세서 (Requirements Specification for AI Vibe Coding)

1. 설정 변수 정의 (Configuration Variables - TBD)
프롬프트 지시사항 (Prompt Instruction): > 아래 변수들은 임의의 초기값으로 설정하며, 추후 튜닝(Tuning)이 가능하도록 코드 상단에 상수로 선언(Declare)해 주세요.

LIMIT_OFFSET_COUNT = 1000: 리미트 스위치 감지 후 진입 허용 제한 거리 (Encoder 카운트)

LIMIT_CURRENT_RATIO = 0.25: 리미트 진입 시 구속 전류 제한 비율 (Max 대비 25%)

LIMIT_DEADZONE_COUNT = 100: 오프셋 재설정을 방지하기 위한 데드존(Dead-zone: 불감대) 최소 이동 거리

SENSOR_ERROR_TIME_MS = 50: 센서 이상 상태(동시 입력 등) 판단 유지 시간 (ms)

BRAKE_RELEASE_DELAY_MS = 150: 브레이크 해제(Brake OFF) 후 모터 기동까지의 대기 시간 (ms)

BRAKE_ENGAGE_DELAY_MS = 100: 모터 정지 판단 후 브레이크 체결(Brake ON)까지의 대기 시간 (ms)

2. 리미트 스위치 제어 로직 (Limit Switch Control Logic)
[기본 설정 (Basic Setup)]

양의 방향(Positive Direction) 리미트 스위치는 LS1, 음의 방향(Negative Direction) 리미트 스위치는 LS2로 정의합니다.

노이즈로 인한 오동작을 막기 위해 모든 리미트 스위치 입력에는 디바운싱(Debouncing: 노이즈 필터링) 처리를 적용합니다.

[동작 및 전류 제어 (Movement & Current Limit)]

스위치(LS1 또는 LS2) 값이 '1'(감지)이 되는 순간, 현재 위치를 기준으로 LIMIT_OFFSET_COUNT 만큼의 제한 거리(Limit Distance)를 설정합니다.

스위치가 감지된 방향으로 이동할 때는 모터 전류 제한(Current Limit)을 최대 출력의 LIMIT_CURRENT_RATIO (20~30%)로 즉시 낮춥니다.

제한 거리(LIMIT_OFFSET_COUNT)에 도달하면, 해당 리미트 방향으로의 이동 명령(Command)은 차단(Block)합니다.

단, 반대 방향으로 탈출하는 이동 명령은 항상 허용(Allow)하며, 반대 방향 이동 시 전류 제한은 정상(Normal, 100%) 상태로 복구합니다.

[오프셋 초기화 및 데드존 (Offset Reset & Dead-zone)]

스위치 값이 '0'(해제)이 되었다가 다시 '1'(감지)이 되면 원칙적으로 제한 거리를 다시 설정(Reset)합니다.

예외 조건 (Dead-zone): 모터가 LIMIT_DEADZONE_COUNT 거리 이상 반대 방향으로 충분히 빠져나오지 않은 상태에서 스위치가 '0'이 되는 경우는 기계적 진동으로 간주하여 오프셋을 초기화하지 않습니다.

[안전 및 에러 처리 (Safety & Error Handling)]

다음 중 하나의 조건이 SENSOR_ERROR_TIME_MS 시간 이상 유지되면 모터를 비상 정지(Emergency Stop)합니다.

LS1의 NO(Normal Open) 단자와 NC(Normal Close) 단자의 논리 값이 같을 때 (센서 단선/단락 고장)

양방향 리미트 스위치 LS1과 LS2가 동시에 '1'(감지)로 입력될 때

3. 전자식 브레이크 시퀀스 (Electronic Brake Sequence)
[모터 기동 시퀀스 (Motor Start Sequence)]

토크 인가 (Torque Pre-charge): 기동 명령이 떨어지면 가장 먼저 PID 제어기를 활성화(Servo On)하여 부하의 무게를 버틸 수 있는 최소한의 유지 토크(Holding Torque / 자화 전류)를 모터에 인가합니다.

브레이크 해제 (Brake Release): 토크 인가 직후 또는 동시에 브레이크 해제(Brake OFF) 신호를 출력합니다.

대기 및 기동: BRAKE_RELEASE_DELAY_MS (100~200ms) 대기 후, 위치/속도 지령을 발생시켜 실제 기동을 시작합니다. (충격 및 밀림 방지 목적)

[모터 정지 시퀀스 (Motor Stop Sequence)]

정지 판단: 모터의 이동이 완료되고 실제 속도가 0에 도달하여 정지 상태임을 확인합니다.

대기 및 브레이크 체결: 정지 후 BRAKE_ENGAGE_DELAY_MS (50~150ms) 대기한 뒤, 브레이크 체결(Brake ON) 신호를 출력합니다.

토크 해제: 브레이크가 완전히 물리적으로 체결된 후, 모터의 유지 토크(전류)를 차단(Servo Off)합니다.





4. Zero Set : 현재 엔코더 값을 기준 위치(0) 으로 설정(Zero Set)하기 위한 로직, 기준 위치는 FRAM 에 저장하여 항상 오프셋으로 사용
5. Zero Reset : 엔코더 초기 0 값을 기준 위치(0) 으로 설정(Zero Reset)