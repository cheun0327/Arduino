#include <Servo.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); 
Servo servo;
const int G1 = 2;
const int R1 = 3;
const int G2 = 4;
const int R2 = 5;
const int G3 = 7;
const int R3 = 8;
const int Buzzer = 10;
const int echoPin = 12;
const int trigPin = 13;
int ledStatus[3] = {0, 0, 0};
int cnt = 0;
int angle = 0;
bool canPark = true;

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
  Jodo_loop();
  Distance_loop();
} 

void Init_setup()
{
  Serial.begin(9600);
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
  lcd.setCursor(0, 0); // 커서를 0번 행(윗줄)의 0번 열(가장 왼쪽 열)에 위치
  lcd.print("LCD ON");
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

  if (reading1 < 20)
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

void Distance_setup()
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

  if (cm <=5)
    servoUp();
  else
    servoDown();

  delay(100); 
}

long microsecondsToCentimeters(long microseconds)
{ 
    return microseconds / 29 / 2;
}

void Servo_setup()
{
  servo.attach(11);
  servo.write(0);
}

void LCD() 
{
  
} 

void setLED(int low, int high, int state)
{
  digitalWrite(low, LOW);
  digitalWrite(high, HIGH);
  if (state == 10)      //parking 1 low
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
  for(int i = 0; i<3; i++)
  {
    if(ledStatus[i])
      cnt++;
  }
  Serial.println(String("--- ") + cnt);
}

void servoUp()
{
 while (angle <= 100)
 {
  servo.write(angle++);
  delay(15); 
 }
}
void servoDown()
{
 while (angle >= 0)
 {
  servo.write(angle--);
  delay(15); 
 }
}
