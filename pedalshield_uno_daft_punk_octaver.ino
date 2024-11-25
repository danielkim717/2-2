//defining harware resources.
#define LED 13
#define FOOTSWITCH 12
#define TOGGLE 2

//defining the output PWM parameters
#define PWM_FREQ 0x00FF // pwm frequency - 31.3KHz
#define PWM_MODE 0 // Fast (1) or Phase Correct (0)
#define PWM_QTY 2 // 2 PWMs in parallel

//other variables
int counter = 0;
int input;
int dist_variable=10;
byte ADC_low, ADC_high;


void setup() {
//setup IO
  pinMode(FOOTSWITCH, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  
  // setup ADC
  ADMUX = 0x60; // left adjust, adc0, internal vcc
  ADCSRA = 0xe5; // turn on adc, ck/32, auto trigger
  ADCSRB = 0x07; // t1 capture for trigger
  DIDR0 = 0x01; // turn off digital inputs for adc0

  // setup PWM
  TCCR1A = (((PWM_QTY - 1) << 5) | 0x80 | (PWM_MODE << 1)); //
  TCCR1B = ((PWM_MODE << 3) | 0x11); // ck/1
  TIMSK1 = 0x20; // interrupt on capture interrupt
  ICR1H = (PWM_FREQ >> 8);
  ICR1L = (PWM_FREQ & 0xff);
  DDRB |= ((PWM_QTY << 1) | 0x02); // turn on outputs
  sei(); // turn on interrupts - not really necessary with arduino
}

void loop() 
{
  //Turn on the LED if the effect is ON.
  if (digitalRead(FOOTSWITCH)) digitalWrite(LED, HIGH); 
    else  digitalWrite(LED, LOW);
  
  //nothing else here, all happens in the Timer 1 interruption.
}

ISR(TIMER1_CAPT_vect) {
    // Timer1 입력 캡처 인터럽트 서비스 루틴; 상승 에지에서 동작함
    // Timer1이 입력 캡처 이벤트를 감지했을 때 실행됨

    counter++; // counter 증가. 딜레이 주기를 제어.
    if (counter >= dist_variable) { 
        // counter가 현재 설정된 딜레이 값(dist_variable)에 도달하면 실행
        counter = 0; // counter를 초기화하여 새로운 딜레이 주기 준비

        // ADC 데이터 읽기
        ADC_low = ADCL; // ADC의 하위 바이트를 먼저 읽음
        ADC_high = ADCH; // ADC의 상위 바이트를 읽음

        // ADC 데이터로 16비트 signed 값을 생성
        input = ((ADC_high << 8) | ADC_low) + 0x8000; 
        // ADC 값을 상위 바이트와 하위 바이트로 결합 후 offset 추가

        // PWM 출력
        OCR1AL = ((input + 0x8000) >> 8); 
        // 입력 값을 unsigned로 변환 후 상위 바이트를 출력
        OCR1BL = input; 
        // 하위 바이트를 출력
    }
}

