1. PBIT 수행 및 통신망 가입
가. (Update)부팅 후 Boot Done 메세지(통신망 가입 요청) PC로 전송, PC는 메세지 받으면 ACK 보내고 100ms 마다 상태정보 요청, UI에서 Boot Done 확인(통신망 가입 완료), DSP에서는 이 ACK 받을때 까지 500 ms 마다 Boot Done 메세지 전송, 받으면 상태정보 요청 대기
나. (Request) 통신망 가입 완료 후 PC는 DSP 에게 상태 정보 요청, DSP는 요청 받으면 상태정보 응답, PC는 응답 받으면 통신 상태 표시
다. (Request) 처음 상태정보 요청 응답 1회 이후 PC에서 PBIT 결과 요청, 요청 메세지 받으면 DSP에서 PBIT 수행 후 결과 메세지 전송, PC에서는 UI상 결과 표시(각 항목 정상/오류)

2. CBIT 수행 절차
가. (Update)PC상에 CBIT 전송주기를 적고 설정 전송 버튼이 있고, 누르면 CBIT 전송주기 메세지 전송(N초), DSP에서는 확인 후 주기 업데이트 및 ACK 전송, PC 에서는 ACK 확인 후 CBIT 주기 업데이트 완료 표시 (CBIT 자체 진행은 모터 제어 주기마다 기존대로 진행)
나. (Reflect)CBIT은 결과 요청 버튼이 있고 누르면 메세지 전송, DSP 상 결과 전송 Flag 켬
다. (Reflect)결과 전송 Flag 가 켜져있으면, 해당 주기마다 CBIT 결과 전송
나. (Reflect)CBIT 결과 요청 중지 버튼이 있고, 누르면 메세지 전송, CBIT 결과 전송 Flag 끔

3. IBIT 수행 절차
가. (Update)IBIT 은 시간 설정 칸(N초), 수행 요청 버튼, 결과 요청 버튼이 있어야함 수행 요청 버튼 누르면 PC가 수행요청 메세지 전송, DSP는 ACK 전송 후 CBIT 전송 일시 중지, N초 동안 IBIT 수행, 수행 중 정상/오류 누적하여 1번이라도 오류 난 항목은 오류, 아니면 정상으로 IBIT 결과 획득
나. (Update)IBIT 결과 획득 후 IBIT Done 메세지를 DSP가 PC로 보내고, PC가 ACK 보내주고  UI에 IBIT 완료 표시, DSP는 ACK 받으면 CBIT 전송 재개
다. (Request) IBIT 결과 요청 버튼 누르면 요청 메세지 전송, 메세지 받으면 DSP는 PC로 결과 메세지 전송하여야 함, 결과 메세지 받으면 IBIT 결과 UI 표시

4. PBIT, CBIT, IBIT 결과 값은 별도로 표시하여 PBIT 인지, CBIT 인지, IBIT 의 값인지 UI 상 알 수 있어야 함