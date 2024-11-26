// 하드웨어 리소스 정의
#define LED 13                // 효과가 활성화되었음을 표시하는 LED 핀
#define FOOTSWITCH 12         // 이펙트 활성화/비활성화를 위한 풋스위치 핀
#define TOGGLE 2              // 모드 전환 스위치 핀

// PWM 출력 파라미터 정의
#define PWM_FREQ 0x00FF       // PWM 주파수 설정 (31.3kHz)
#define PWM_MODE 0            // PWM 모드 (0 = Phase Correct, 1 = Fast PWM)
#define PWM_QTY 2             // 병렬로 사용할 PWM 출력의 수 (2개)

// 기타 변수
int input, vol_variable = 512;       // 입력 신호 및 볼륨 조절 변수
unsigned int ADC_low, ADC_high;      // ADC에서 읽은 데이터 저장

// 딜레이 버퍼 설정
#define MAX_DELAY 2000               // 딜레이 버퍼의 최대 크기
byte DelayBuffer[MAX_DELAY];         // 딜레이 데이터를 저장할 버퍼
unsigned int DelayCounter = 0;       // 현재 딜레이 버퍼의 위치를 나타내는 카운터
unsigned int Delay_Depth = MAX_DELAY;// 초기 딜레이 깊이 (최대 값)

void setup() {
  // IO 핀 설정
  pinMode(FOOTSWITCH, INPUT_PULLUP);  // 풋스위치를 풀업 저항으로 설정
  pinMode(TOGGLE, INPUT_PULLUP);      // 토글 스위치를 풀업 저항으로 설정
  pinMode(LED, OUTPUT);               // LED를 출력으로 설정
  
  // ADC 설정
  ADMUX = 0x60; // ADC0 채널 선택, 내부 전압 기준(Vcc), 좌측 정렬
  ADCSRA = 0xe5; // ADC 활성화, CK/32 분주, 자동 트리거 활성화
  ADCSRB = 0x07; // Timer1 캡처 이벤트를 트리거로 사용
  DIDR0 = 0x01;  // ADC0의 디지털 입력 비활성화
  
  // PWM 설정
  TCCR1A = (((PWM_QTY - 1) << 5) | 0x80 | (PWM_MODE << 1)); 
  // PWM 출력 수, 비반전 모드 설정
  TCCR1B = ((PWM_MODE << 3) | 0x11); 
  // Phase Correct PWM 모드, 분주율 1 설정
  TIMSK1 = 0x20; // 입력 캡처 인터럽트 활성화
  ICR1H = (PWM_FREQ >> 8); // PWM TOP 값 상위 바이트 설정
  ICR1L = (PWM_FREQ & 0xff); // PWM TOP 값 하위 바이트 설정
  DDRB |= ((PWM_QTY << 1) | 0x02); // PWM 출력 핀 활성화
  sei(); // 전역 인터럽트 활성화
}

void loop() 
{
  // 풋스위치 상태에 따라 LED 상태를 설정
  if (digitalRead(FOOTSWITCH)) 
    digitalWrite(LED, HIGH); // 이펙트 활성화 시 LED 켜기
  else  
    digitalWrite(LED, LOW);  // 이펙트 비활성화 시 LED 끄기
  // 루프에서 추가 동작은 없음, 모든 작업은 Timer 1 인터럽트에서 수행됨
}

ISR(TIMER1_CAPT_vect) 
{
  // ADC 데이터 가져오기
  ADC_low = 0; // 메모리 절약을 위해 ADC_low는 항상 0으로 설정
  ADC_high = ADCH; // ADC 상위 바이트 읽기
  
  // 딜레이 버퍼에 상위 바이트 저장
  DelayBuffer[DelayCounter] = ADC_high;

  // 딜레이 카운터를 증가시키거나, 딜레이 깊이를 초과하면 초기화
  DelayCounter++;
  if (DelayCounter >= Delay_Depth) 
    DelayCounter = 0; 

  // 입력 신호 생성
  input = 
    (((DelayBuffer[DelayCounter] << 8) | ADC_low) + 0x8000) + 
    (((ADC_high << 8) | ADC_low) + 0x8000); 
  // 딜레이 버퍼에서 데이터를 읽어 signed 16비트 값으로 변환하고 현재 ADC 값을 더함

  // PWM 신호 출력
  OCR1AL = ((input + 0x8000) >> 8); // unsigned 값으로 변환 후 상위 바이트 출력
  OCR1BL = input; // 하위 바이트 출력
}
