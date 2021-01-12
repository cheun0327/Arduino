#include <DHT.h>
#include <DHT_U.h>
#include <Adafruit_Sensor.h>
#include <pitches.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>

#define DHTTYPE DHT11 
#define DHTPin 6

LiquidCrystal_I2C lcd(0x27, 16, 2); 
Servo servo;
DHT dht(DHTPin, DHTTYPE);
bool servoFlag = false;
const int button = 2;     //주차장을 사용할 수 없을 때, 강제로 차단기 여는 버튼
const int R1 = 3;         //LED
const int G2 = 4;         //LED
const int R2 = 5;         //LED
//const int DHTPin = 6;   //DHT
const int G3 = 7;         //LED
const int R3 = 8;         //LED
const int G1 = 9;         //LED
const int piezo = 10;     //piezo buzzer
//servo = 11;
const int echoPin = 12;   //distance sensor
const int trigPin = 13;   //distance sensor
const int potentio = A0;  //potentio

int ledStatus[3] = {0, 0, 0}; //check parking space available
int cnt = 0;              //count left parking space
int angle = 0;            //servo motor angle
byte char_sad[8] = {0x00, 0x1B, 0x1B, 0x00, 0x00, 0x0E, 0x11, 0x11};    //:(
byte char_smile[8] = {0x00, 0x1B, 0x1B, 0x00, 0x00, 0x11, 0x11, 0x0E};  //:)
byte heart[8] = {0x00, 0x0A, 0x1F, 0x1F, 0x1F, 0x0E, 0x04, 0x00};       //하트
int openSong[4] = {NOTE_C5, NOTE_E5, NOTE_G5, NOTE_C6};             //gate bar open song
int closeSong[4] = {NOTE_C6, NOTE_G5, NOTE_E5, NOTE_C5};            //gate bar close song
int songTime[4] = {200, 200, 200, 300};
int endSong[29] = {NOTE_B4, NOTE_C5, NOTE_D5, 0, NOTE_D5, NOTE_C5, NOTE_B4, 0,
NOTE_A4, 0, NOTE_A4, 0, NOTE_A4, NOTE_B4, NOTE_C5, NOTE_B4, NOTE_A4, 0,
NOTE_A4, 0, NOTE_A4, NOTE_B4, NOTE_C5, 0, NOTE_B4, NOTE_A4, NOTE_G4, 0, 0};
int endTime[29] = {250, 250, 250, 250, 250, 250, 250, 250,
250, 20, 250, 20, 250, 250, 250, 250, 250, 250,
250, 20, 250, 250, 250, 250, 250, 250, 250, 250, 250};
float tem;                //temperature
float hum;                //humid
bool canUse = true;       //if tem or hum is too high, cannot use parking lot

void setup()
{
  Init_setup();
  Led_setup();
  Lcd_setup();
  Servo_setup();
  Distance_setup(); 
}
void loop() 
{
  potentio_loop();    //가변저항 level 3이상이면 주차장 종료(노래로 알림)
  Dht_loop();         //온습도가 너무 높으면 주차장 운영 중지(차단기 안열림)
  Jodo_loop();        //주차여부 판단
  Distance_loop();    //입구에 차량 도착 감지 후, 차단기 동작
  LCD_loop();         //lcd에 상황 표시
} 

void Init_setup()
{
  Serial.begin(9600);
  pinMode(piezo, OUTPUT);
  pinMode(potentio, INPUT);
  dht.begin();
  attachInterrupt(0, forceOpen, RISING);
}

void Led_setup()
{
  pinMode(G1, OUTPUT);
  pinMode(R1, OUTPUT);
  pinMode(G2, OUTPUT);
  pinMode(R2, OUTPUT);
  pinMode(G3, OUTPUT);
  pinMode(R3, OUTPUT);
}

void Lcd_setup()
{
  lcd.init(); // I2C LCD의 초기화
  lcd.backlight(); // I2C LCD의 백라이트 켜기
  lcd.clear();
  lcd.createChar(0, char_sad);  //슬픈 이모티콘 저장
  lcd.createChar(1, char_smile);//웃는 이모티콘 저장
  lcd.createChar(2, heart);      //하트 이모티콘 저장
}

void potentio_loop()
{
  int adc = analogRead(potentio); //가변저항 값 읽기
  int level = adc / 205;    // level (0~4) 설정
  if (level >= 3)
  {
    for(int i=0; i<29; i++) //level이 3이상이면 주자창 종료 노래가 나온다(알림)
    {
      tone(piezo, endSong[i], endTime[i]);
      delay(endTime[i]);
    }
  }
}

void Jodo_loop()
{
  int reading1 = analogRead(A1); // 조도 센서 읽기
  int reading2 = analogRead(A2);
  int reading3 = analogRead(A3);
 
  Serial.print("1 : ");
  Serial.print(reading1);
  Serial.print(" / 2 : ");
  Serial.print(reading2);
  Serial.print(" / 3 : ");
  Serial.println(reading3);

  if (reading1 < 20)        //밝기가 충분히 어두워지면 주차 된것으로 판단
    setLED(G1, R1, 10);
  else
    setLED(R1, G1, 11);
  if (reading2 < 20)
    setLED(G2, R2, 20);
  else
    setLED(R2, G2, 21);
  if (reading3 < 20)
    setLED(G3, R3, 30);
  else
    setLED(R3, G3, 31);
  
  delay(1000);
}

void Distance_setup()   //초음파 거리측정 센서 설정
{
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void Distance_loop()
{
  long duration, inches, cm;

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2); 
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10); 
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH); 

  cm = microsecondsToCentimeters(duration);
  Serial.print(cm);
  Serial.println(" cm"); 

  if (!servoFlag && canUse && cm <=5)   //차단기가 닫힌 상태 & 주차장 사용 가능 & 차량 진입
    servoUp();                          //차단기 올림
  else if (servoFlag && cm > 5)         //차단기가 열린 상태 & 진입 원하는 차량 없음
    servoDown();                        //차단기 내림

  delay(100); 
}

long microsecondsToCentimeters(long microseconds)
{ 
    return microseconds / 29 / 2;
}

void Servo_setup()    //서보 모터 설정
{
  servo.attach(11);
  servo.write(0);
}

void LCD_loop() 
{
  if (canUse)
  {
    if (cnt > 0)    //주차장에 주차 공간이 있을 때 가능한 주차 공간 표시
    {
      lcd.clear();
      lcd.setCursor(0, 0); // 커서를 0번 행(윗줄)의 0번 열(가장 왼쪽 열)에 위치
      lcd.print(cnt + String(" Available "));
      lcd.write(byte(1));
      lcd.write(byte(2));
    }
    else          //주차장에 여유 주차 공간 없을 때 자리 없음 표시
    {
      lcd.clear();
      lcd.setCursor(0, 0); // 커서를 0번 행(윗줄)의 0번 열(가장 왼쪽 열)에 위치
      lcd.print("NO Parking Space");
      lcd.setCursor(4, 1);
      lcd.write(byte(0));
      lcd.print(" ");
      lcd.write(byte(0));
      lcd.print(" ");
      lcd.write(byte(0));
      lcd.print(" ");
      lcd.write(byte(0));
    }
  } 
  else      //주차장 이용이 어려울 때(온습도 영향)
  {
    lcd.clear();
    lcd.print("  UNAVAILABLE  ");
    lcd.setCursor(4, 1);
    lcd.write(byte(0));
    lcd.print(" ");
    lcd.write(byte(0));
    lcd.print(" ");
    lcd.write(byte(0));
    lcd.print(" ");
    lcd.write(byte(0));
  }
} 

void Dht_loop()
{
  tem = dht.readTemperature();
  hum = dht.readHumidity();
  if (isnan(tem) || isnan(hum))
    Serial.println(F("Failed to read from DHT sensor!"));
  else
  {
    Serial.print("Temp : "); Serial.print(tem);
    Serial.print(" "); 
    Serial.print("Humid : "); Serial.print(hum); 
    Serial.println("%");
    if (tem > 50.0 || hum > 80)   //온습도가 높으면 사용 불가능 표시
      canUse = false;
    else
      canUse = true;
  }
}

void setLED(int low, int high, int state)   //꺼지는 led, 켜지는 led를 구별해서 씀
{
  digitalWrite(low, LOW);
  digitalWrite(high, HIGH);
  if (state == 10)      //parking 1 low //상태 저장
    ledStatus[0] = 0;
  else if (state == 11)  //parking 1 high
    ledStatus[0] = 1;
  else if (state == 20) //parking 2 low
    ledStatus[1] = 0;
  else if (state == 21)  //parking 2 high
    ledStatus[1] = 1;
  else if (state == 30) //parking 3 low
    ledStatus[2] = 0;
  else if (state == 31)  //parking 3 high
    ledStatus[2] = 1;
  cnt = 0;
  for(int i = 0; i<3; i++)  //count available seat
  {
    if(ledStatus[i])
      cnt++;
  }
  Serial.println(String("--- ") + cnt);
}

void servoUp()
{
  servoFlag = true;
  for(int i=0; i<4; i++)  //차단기 열리기 전 효과음 내기
  {
    tone(piezo, openSong[i], songTime[i]);
    delay(songTime[i]);
  }
  while (angle <= 100)    //차단기 올림
  {
    servo.write(angle++);
    delay(15); 
   }
}
void servoDown()
{
  servoFlag = false;
  for(int i=0; i<4; i++)  //차단기 내리기 전 효과음 내기
  {
    tone(piezo, closeSong[i], songTime[i]);
    delay(songTime[i]);
  }
  while (angle >= 0)      //차단기 내림
  {
    servo.write(angle--);
    delay(15); 
  }
}

void forceOpen()        //인터럽트 핸들러
{//주차장을 사용할 수 없을 때, 강제로 차단기 여는 동작
  canUse = true;
}
