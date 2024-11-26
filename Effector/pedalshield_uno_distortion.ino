//defining harware resources. 
#define LED 13               // 이펙트가 활성화되었음을 표시하는 LED 핀
#define FOOTSWITCH 12        // 이펙트 활성화/비활성화를 위한 풋스위치 핀
#define TOGGLE 2             // 모드 전환 스위치 핀

//PWM 출력 파라미터 정의
#define PWM_FREQ 0x00FF // PWM 주파수 설정 - 31.3KHz
/* 
0x00FF는 16진수 값으로, 타이머1의 TOP 값을 설정합니다. 이 값이 낮을수록 주파수가 높아집니다.
주파수는 Arduino 클럭 속도(16MHz)를 기반으로 계산됩니다.
주파수 = 클럭 속도 / (2 * N * TOP), 여기서 N은 분주율(1)이며, TOP이 0x00FF일 때 약 31.3kHz가 됨
가청 주파수 20Hz~20kHz 를 벗어나는 값으로 설정해야 노이즈가 발생하지 않음.
주파수가 너무 낮으면 음질에 영향을 줄 수도 있음. 31.3kHz는 모든 문제를 절충한 값으로, 
음향쪽에서 자주 사용되는 값
*/
#define PWM_MODE 0 // PWM 모드 설정 (0 = Phase Correct, 1 = Fast PWM)
/* 
Fast PWM 모드가 아닌 Phase Correct 모드를 사용하는 이유는 신호의 대칭성과 품질을 유지하며, 
오디오 신호 처리의 특성을 최적화하기 위함임. 신호 왜곡이 필연적인 이번 프로젝트에서 클리핑 외의 노이즈를
피하기 위해서 선택되었음.
*/
#define PWM_QTY 2 // 2 PWMs in parallel  // 병렬로 사용할 PWM 출력의 수 (2개의 핀 사용)

//other variables
int input, distortion_threshold=6000; // 초기 왜곡 임계값
unsigned int ADC_low, ADC_high;       // ADC로 읽은 데이터 저장

void setup() {

  //setup IO
  pinMode(FOOTSWITCH, INPUT_PULLUP);
  pinMode(TOGGLE, INPUT_PULLUP);
// 풋스위치, 토글 스위치를 모두 풀업저항으로 설정
  pinMode(LED, OUTPUT);                  // LED를 출력으로 설정
  
  // setup ADC
  ADMUX = 0x60; // ADC0 채널 선택, 내부 기준 전압(VCC), 좌측 정렬
  /* 
  0x60 = 0b01100000
  01 => 기준 전압을 내부 전압으로 설정
  1  => 좌측 정렬 (결과가 상위 8비트에 저장됨) (데이터 처리 속도 향상)
  0000 => ADC0 채널 선택 (아날로그 핀 A0)
  */
  ADCSRA = 0xe5; //ADC 활성화, CK/32, 자동 트리거
  /*
  0xe5 = 0b11100101
  1 => ADC 활성화
  1 => 변환 시작 (자동 트리거 모드에서는 무시됨)
  1 => 자동 트리거 활성화
  101 => 프리스케일러 값 32 설정
  */
  ADCSRB = 0x07; // T1 캡쳐 이벤트에서 자동 트리거
  DIDR0 = 0x01; // ADC0의 디지털 입력 비활성화 (불필요한 디지털 신호 간섭 방지)

  // setup PWM
  TCCR1A = (((PWM_QTY - 1) << 5) | 0x80 | (PWM_MODE << 1)); //TCCR1A 설정: PWM 출력 수, 비반전 모드 설정
  TCCR1B = ((PWM_MODE << 3) | 0x11); // TCCR1B 설정: PWM 모드와 분주율 (ck/1)
  TIMSK1 = 0x20; // 입력 캡처 인터럽스 활성화
  ICR1H = (PWM_FREQ >> 8);  // TOP 상위 바이트
  ICR1L = (PWM_FREQ & 0xff);  // TOP 하위 바이트
  DDRB |= ((PWM_QTY << 1) | 0x02); // PWM 출력 핀 활성화
  sei(); // 전역 인터럽트 활성화 
  // PWM 부분은 신호 샘플링과 왜곡된 신호를 출력하는데 최적화된 방식으로 설정되어 있음
  }

void loop() 
{
  if (digitalRead(FOOTSWITCH)) digitalWrite(LED, HIGH); //풋스위치 상태에 따라 LED 상태를 설정
     else  digitalWrite(LED, LOW);
  //이펙트 활성화 시 LED 를 키고, 비활성화시 LED를 끈다.
}

//타이머1 입력 캡처 인터럽트 서비스 루틴
ISR(TIMER1_CAPT_vect) 
{
  ADC_low = ADCL; // ADC 하위 바이트 읽기
  ADC_high = ADCH; // ADC 상위 바이트 읽기

  //ADC 데이터를 16비트 signed 값으로 변환
  input = ((ADC_high << 8) | ADC_low) + 0x8000; 

  // ADC로 읽은 입력 신호는 -32768에서 +32768 범위의 16비트 signed 값
  // 입력 신호 값이 distortion_threshold 값을 초과하면 클리핑 수행
    if(input>distortion_threshold) input=distortion_threshold; //클리핑 : 최대값을 distortion_threshold로 제한

  //PWM 신호 출력
  OCR1AL = ((input + 0x8000) >> 8); 
  // signed 16비트 값을 unsigned로 변환 후 상위 8비트를 OCR1AL에 저장
  // input + 0x8000 → -32768 ~ +32768 범위를 0 ~ 65535 범위로 변환
  // (input + 0x8000) >> 8 → 상위 8비트 추출
  
  OCR1BL = input; 
  // 하위 8비트를 OCR1BL에 저장하여 PWM 출력 설정
}
