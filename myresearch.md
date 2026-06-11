DSP_Initialization - DSP 만이 아니라서 함수명 변경 필요

Device_init

initW6100GpioPins - 여기 위치 맞는지 확인 필요

Initial_GPIO

Interrupt_initModule
Interrupt_initVectorTable

InitialPeripherals


 밖에 나와있는 함수들 Peripherals 에 포함 검토
Ethernet_Init - W6100
Fram_Init
Encoder_Init

개념적으로 초기화 후에 Interrupt 시작 가능한지 확인

EPWM1에 다 포함할 수 있는지 확인

초기화 - PWM 인터럽트 시작 - 전류센서 Offset 조정(동작 안할때 0으로 Fram 저장) - 초기점검(PBIT) - 이더넷 인터럽트 시작

EPWM1에 ADC 처리, DIO 처리, 엔코더 정보 처리, 모터드라이버 상태 처리, 주기점검(CBIT), 모터 구동 제어, 데이터 저장 순서

외부 인터럽트에 이더넷 통신 - 별도 인터럽트로 통신 되어있는지 확인 필요