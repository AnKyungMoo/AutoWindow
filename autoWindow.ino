#include <DHT11.h>
#include <Stepper.h>
#define MAXSIZE 5

// 모터 셋팅
const int stepsPerRevolution = 200; // 모터의 1회전당 스텝 수에 맞게 조정   
Stepper myStepper(stepsPerRevolution, 11,9,10,8); // Note 8 & 11 swapped

int pin=3;
DHT11 dht11(pin);
int inputPin = 4; // 센서 시그널핀
int pirState = LOW; // PIR 초기상태
int val = 0; // Signal 입력값
int measurePin = 2;   //A2
int ledPower = 5;     //핀번호 5

int samplingTime = 280;
int deltaTime = 40;
int sleepTime = 9680;

float voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;
bool windowFunction[MAXSIZE] = {false, };     // 각 기능별 bool 체크
int activationFunc = -1;
bool windowCheck = false;   // 창문 열닫
bool curtainCheck = false;  // 커튼 열닫

//Hardware pin definitions
int UVOUT = A0; //Output from the sensor
int REF_3V3 = A1; //3.3V power on the Arduino board

void setup()
{
  Serial.begin(9600);

  pinMode(UVOUT, INPUT);
  pinMode(REF_3V3, INPUT);
  pinMode(inputPin, INPUT); // 센서 Input 설정
  pinMode(ledPower, OUTPUT);

  // 모터 스피드 설정
  myStepper.setSpeed(120);

  Serial.println("uv센서 예제");
}

void loop()
{
  int uvLevel = averageAnalogRead(UVOUT);
  int refLevel = averageAnalogRead(REF_3V3);
  int err;
  float temp, humi;

  //Use the 3.3V power pin as a reference to get a very accurate output value from sensor
  float outputVoltage = 3.3 / refLevel * uvLevel;

  float uvIntensity = mapfloat(outputVoltage, 0.99, 2.8, 0.0, 15.0); //Convert the voltage to a UV intensity level

  if((err=dht11.read(humi, temp))==0) //온도, 습도 읽어와서 표시
  {
    Serial.print("온도:");
    Serial.print(temp);
    Serial.print("습도:");
    Serial.print(humi);
    Serial.println();
  }
  else                                //에러일 경우 처리
  {
    Serial.println();
    Serial.print("Error No :");
    Serial.print(err);
    Serial.println();    
  }

  Serial.print("output: ");
  Serial.print(refLevel);

  Serial.print(" 센서 출력: ");
  Serial.print(uvLevel);

  Serial.print(" 출력전압값: ");
  Serial.print(outputVoltage);

  Serial.print(" 자외선 강도: ");
  Serial.print(uvIntensity);

  Serial.println();

  int rainValue = analogRead(A1); // ADC  값 읽기
 
 
  // 시리얼 통신으로 전송
  Serial.print("빗물 수치 : ");   Serial.print(rainValue);
  Serial.println();

  //************************모션인식*********************
  val = digitalRead(inputPin); // 센서값 읽기

  if(val == HIGH)
    Serial.println("모션이 감지되었습니다");

  //************************미세먼지*********************
  digitalWrite(ledPower, LOW);
  delayMicroseconds(samplingTime);

  voMeasured = analogRead(measurePin);

  delayMicroseconds(deltaTime);
  digitalWrite(ledPower, HIGH);
  delayMicroseconds(sleepTime);

  calcVoltage = voMeasured * (5.0 / 1024.0);

  dustDensity = 0.17 * calcVoltage - 0.1;
  if(dustDensity * 1000 <= 0)
  {
     dustDensity = 0;
  }

  Serial.print("Raw Signal Value (0-1023): ");
  Serial.print(voMeasured);

  Serial.print(" - Voltage: ");
  Serial.print(calcVoltage);

  Serial.print(" - Dust Density: ");

  Serial.print(dustDensity * 1000);
  Serial.println(" ug/m3 ");
  
  // 창문이 열려있을 때
  if(windowCheck == false && activationFunc == -1)
  {
    // 빗물
    if(rainValue < 100)
    {
      Serial.println("비가 내려서 창문을 닫습니다.");
      for (int x = 0; x < 19; ++x)
      {
        myStepper.step(stepsPerRevolution);
      }
      
      windowCheck = true;
      activationFunc = 0;
    }
    // 습도
    else if(humi <= 22)
    {
      Serial.println("내부 습도가 낮아져 창문을 닫습니다.");
      for (int x = 0; x < 19; ++x)
      {
        myStepper.step(stepsPerRevolution);
      }

      windowCheck = true;
      activationFunc = 1;
    }
    // 먼지
    else if(dustDensity * 1000 >= 80)
    {
      Serial.println("미세먼지가 많아 창문을 닫습니다.");   // 미세먼지 나쁜 기준
      for (int x = 0; x < 19; ++x)
      {
        myStepper.step(stepsPerRevolution);
      }
      
      windowCheck = true;
      activationFunc = 2;   
    }
    // 온도
    else if(temp <= 20)
    {
      Serial.println("온도가 낮아 창문을 닫습니다.");
      for (int x = 0; x < 19; ++x)
      {
        myStepper.step(stepsPerRevolution);
      }
      
      windowCheck = true;
      activationFunc = 3;
    }
  }
  // 창문이 열려있을 때
  else if (windowCheck == true)
  {
    // 빗물
    if(activationFunc == 0 && rainValue >= 300)
    {
      Serial.println("비가 그쳐서 창문을 엽니다.");
      for (int x = 0; x < 19; ++x)
      {
        myStepper.step(-stepsPerRevolution);
      }
      
      windowCheck = false;
      activationFunc = -1;
    }
    // 습도
    else if(activationFunc == 1 && humi >= 27)
    {
      Serial.println("내부 습도가 높아져 창문을 엽니다.");

      for (int x = 0; x < 19; ++x)
      {
        myStepper.step(-stepsPerRevolution);
      }
      
      windowCheck = false;
      activationFunc = -1;
    }
    //먼지
    else if(activationFunc == 2 && dustDensity * 1000 <= 30)  // 미세 먼지 좋은 기준
    {
      Serial.println("미세먼지가 적어져 창문을 엽니다.");
      for (int x = 0; x < 19; ++x)
      {
        myStepper.step(-stepsPerRevolution);
      }
      
      windowCheck = false;
      activationFunc = -1;
    }
    // 온도
    else if(activationFunc == 3 && temp >= 24)
    {
      Serial.println("온도가 높아 창문을 엽니다.");
      for (int x = 0; x < 19; ++x)
      {
        myStepper.step(-stepsPerRevolution);
      }
      
      windowCheck = false;
      activationFunc = -1;
    }
  }

  // 커튼 열닫 조건
  if(curtainCheck == false)
  {
    if(uvIntensity >= 70)
    {
      Serial.println("자외선이 높아 커튼을 내립니다");
      curtainCheck = true;  
    }
  }
  else if(curtainCheck == true)
  {
    if(uvIntensity <= 50)
    {
      Serial.println("자외선이 낮아져 커튼을 올립니다");
      curtainCheck = false;  
    }
  }
  
  

  delay(1000);
}

//Takes an average of readings on a given pin
//Returns the average
int averageAnalogRead(int pinToRead)
{
  byte numberOfReadings = 8;
  unsigned int runningValue = 0; 

  for(int x = 0 ; x < numberOfReadings ; x++)
    runningValue += analogRead(pinToRead);
  runningValue /= numberOfReadings;

  return(runningValue);  
}


float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
