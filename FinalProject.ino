void setup() {

  blink_setup();      
  fading_setup();

}

void loop() {

  blink_loop();            // blink 처리
  fading_loop();          // fading 처리

}

int ledPin_blink = 7;

// ledState, previousMillis, blink_interval 변수는 전역변수로 
// 스케치 파일 내 모든 함수에서 참조 가능

int blink_ledState = LOW;
long blink_previousMillis = 0;
long blink_interval = 500;

void blink_setup() {            // blink setup

  pinMode(ledPin_blink, OUTPUT);

}

void blink_loop() {
  // millis 함수로 현재 시간 얻기
  unsigned long currentMillis = millis();

  if(currentMillis-blink_previousMillis >= blink_interval) {
    blink_previousMillis = currentMillis;

    if(blink_ledState == LOW) 
      blink_ledState = HIGH;
    else
      blink_ledState = LOW;

    digitalWrite(ledPin_blink, blink_ledState);      // blinking 적용
  }
}

//fadingValue, fadingDir, previousMillis, fading_interval 등은 전역변수로 스케치 파일 내 모든 함수에서 참조 가능

int ledPin_fading = 11;
int fadingValue = 0;
int fadingDir = 1;
long fading_previousMillis = 0;
long fading_interval = 30;

void fading_setup() {                // fading setup

  pinMode(ledPin_fading, OUTPUT);

}

void fading_loop() {
  unsigned long currentMillis = millis();

  if(currentMillis-fading_previousMillis >= fading_interval) {
    fading_previousMillis = currentMillis;

    if(fadingDir == 1) {
      if(fadingValue <= 255) {
        fadingValue +=3;
        if(fadingValue > 255) {
          fadingValue = 255;
          
          fadingDir = 0;
        }
      }
    } 
    else {
      if(fadingValue >= 0) {
        fadingValue -=3;
        if(fadingValue < 0) {
          fadingValue = 0;
          
          fadingDir = 1;
        }
      }
    }
    analogWrite(ledPin_fading, fadingValue);          // fading 적용
  }
}
