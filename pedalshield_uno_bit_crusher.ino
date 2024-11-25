//defining harware resources.
#define LED 13
#define FOOTSWITCH 12
#define TOGGLE 2

//defining the output PWM parameters
#define PWM_FREQ 0x00FF // pwm frequency - 31.3KHz
#define PWM_MODE 0 // Fast (1) or Phase Correct (0)
#define PWM_QTY 2 // 2 PWMs in parallel

//other variables
int input, bit_crush_variable=4;
unsigned int ADC_low, ADC_high;

void setup() {
  //setup IO
  pinMode(FOOTSWITCH, INPUT_PULLUP);
  pinMode(TOGGLE, INPUT_PULLUP); 
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
  //nothing here, all happens in the Timer 1 interruption.
}

ISR(TIMER1_CAPT_vect) {
    // Timer1 입력 캡처 인터럽트 서비스 루틴
    // Timer1이 입력 캡처 이벤트를 감지했을 때 호출됨

    // 1. ADC 데이터를 읽어들임
    ADC_low = ADCL; // ADC의 하위 바이트를 먼저 읽음 (8비트)
    ADC_high = ADCH; // ADC의 상위 바이트를 읽음 (8비트)
    // ADC 데이터를 16비트 값으로 결합
    input = ((ADC_high << 8) | ADC_low) + 0x8000; 
    // ADC 값을 signed 16비트 값으로 변환 (중간값 0을 기준으로 음수/양수 처리)

    //// 디지털 신호 처리 (DSP)가 이 부분에서 이루어짐 ////

    // 2. 신호 비트 크러싱 처리
    // bit_crush_variable의 값에 따라 신호를 '비트 크러싱' 처리.
    // 신호를 왼쪽으로 bit_crush_variable만큼 시프트하여 하위 비트를 제거.
    input = input << bit_crush_variable; 

    // 3. PWM 신호 출력
    OCR1AL = ((input + 0x8000) >> 8); 
    // 입력 값을 unsigned로 변환 후 상위 8비트를 출력
    OCR1BL = input; 
    // 하위 8비트를 출력
}

