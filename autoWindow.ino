#include <DHT11.h>
#include <Stepper.h>
#include "SPI.h"
#include "WiFi.h"
#define MAXSIZE 5
#define BTN_PIN 6
#define SERVICE_KEY String("89b33429e954137534d5f996da975b2e")
#define LOCATION   String("Asan")
#define PORT      80

// WiFi 셋팅
char SSID[] = "ESL";
const char PASS[] = "dlaqpelemm608";
const char URL[] = "api.openweathermap.org";

WiFiClient client;

int establish_state = 0;

String currentLine = "";
String tempLine = "";
String dateLine = "";
String season;

boolean tempActive = false;
boolean dateActive = false;

int tempValue = 0;
int monthValue = 0;

// 수동_자동모드 셋팅
int mode_state = 0;
int prevbtn = -1;
int currbtn = -1;
int motion_state = 0;
int prev_motion = -1;
int curr_motion = -1;

// 모터 셋팅
const int stepsPerRevolution = 200; // 모터의 1회전당 스텝 수에 맞게 조정   
Stepper myStepper(stepsPerRevolution, 11,9,10,8); // Note 8 & 11 swapped

int pin=3;
DHT11 dht11(pin);      // 온습도
int motion_input = 4;   // 모션 센서 시그널핀
int pirState = LOW;     // PIR 초기상태
int motionValue = 0;    // 모션 Signal 입력값
int measurePin = 2;     // 미세먼지 A2
int ledPower = 5;       // 미세먼지 핀번호 5

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
int UVOUT = A0;             // 자외선
int REF_3V3 = A1;           // 빗물 3.3V power on the Arduino board

void setup()
{
  Serial.begin(9600);

  // WiFi setup
   Serial.println("Attempting to connect to WPA network...");
   Serial.print("SSID: ");
   Serial.println(SSID);

   Serial.print("Connecting to WiFi... ");
   
   if (WiFi.begin(SSID, PASS) != WL_CONNECTED)
   {
      Serial.println("Error.");
      while (true); /*DONT DO ANYTHING ELSE*/
   }

   Serial.println("Complete.");
   
   setup_client();
   get_from_client();

  // 센서 setup
  pinMode(BTN_PIN, INPUT);      // 모드 전환 버튼 input 설정
  pinMode(UVOUT, INPUT);        // 자외선 핀 설정
  pinMode(REF_3V3, INPUT);  
  pinMode(motion_input, INPUT); // 모션 센서 Input 설정
  pinMode(ledPower, OUTPUT);    // 미세먼지 아웃풋

  // 모터 스피드 설정
  myStepper.setSpeed(120);
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

  currbtn = digitalRead(BTN_PIN);   // 자동 & 수동모드 처리
  
  if(currbtn == 1 && prevbtn == 0)  // 자동 & 수동모드 확인
  {
    if(mode_state == 0)
    {
      mode_state = 1;
      Serial.println("수동모드로 변환");  
    }
    else
    {
      mode_state = 0;
      Serial.println("자동모드로 변환");
    }
  }
  prevbtn = currbtn;
  delay(100);
  
  /*****************************************************************************************/

  // 겨울일 때 /*기준 설정하자!*/
  if(mode_state == 0)     // 자동 모드
  {
    if (establish_state == 1) 
   {
      while (client.available())
      {
         char c = client.read();
         //Serial.print(c);

         currentLine += c;

         if (currentLine.endsWith("<temperature value="))
         {
            tempActive = true;
         }
         else if(currentLine.endsWith("<lastupdate value="))
         {
            dateActive = true;
         }
         
         if (tempActive)
         {
            if(c != 'm')
            {
               tempLine += c;
            }
            else
            {
               tempActive = false;
                
               tempValue = string_to_int(tempLine) - 273;  // 절대온도 계산
               Serial.println();
               Serial.print("tempActive : ");
               Serial.print(tempValue);
               Serial.println("°C");
            }
         }
         else if(dateActive)
         {
            if(c != 'T')
            {
                dateLine += c;
            } 
            else
            {
                dateActive = false;
                
                monthValue = month_substring(dateLine);
                Serial.println();
                Serial.print("dayActive : ");
                Serial.println(monthValue);

                if(monthValue >= 4 && monthValue <= 9)
                {
                   season = "Summer";
                }
                else
                {
                   season = "Winter";  
                }

                Serial.println(season);
            }
         }
      }

      if (!client.connected() &&
         !client.available())
      {
         Serial.println();
         client.stop();
         establish_state = 0;
         Serial.println("Disconnected.");         

         delay(10000);

         Serial.println("");
         
         // 메모리에 데이터 리셋
         currentLine = "";
         tempLine = "";
         dateLine = "";
         
         setup_client();
         get_from_client();
      }
    }
   
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
  
  
    //************************미세먼지*********************
    digitalWrite(ledPower, LOW);
    delayMicroseconds(samplingTime);
  
    voMeasured = analogRead(measurePin);
  
    delayMicroseconds(deltaTime);
    digitalWrite(ledPower, HIGH);
    delayMicroseconds(sleepTime);
  
    calcVoltage = voMeasured * (5.0 / 1024.0);
  
    dustDensity = 0.17 * calcVoltage - 0.1;
    
    if(dustDensity * 1000 <= 0)   // 음수 값 필터링(0으로 처리)
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
    
    if(season == "Winter")
    {
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
        /*
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
        */
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
        else if(temp >= tempValue)
        {
          Serial.println("집안의 온도가 높아 창문을 닫습니다.");
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
        /*
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
        */
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
        else if(activationFunc == 3 && temp < tempValue)
        {
          Serial.println("집안의 온도가 낮아 창문을 엽니다.");
          for (int x = 0; x < 19; ++x)
          {
            myStepper.step(-stepsPerRevolution);
          }
          
          windowCheck = false;
          activationFunc = -1;
        }
      }
    }
    // 여름일 때 /*기준 설정하자!*/
    else if(season == "Summer")
    {
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
        /*
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
        */
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
        else if(temp <= tempValue)
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
        /*
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
        */
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
        else if(activationFunc == 3 && temp > tempValue)
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
    }
  }
  else    // 수동 모드
  {
    curr_motion = digitalRead(motion_input); // 센서값 읽기

    if (prev_motion == LOW && curr_motion == HIGH)
    {
       if (motion_state == 0)         // 창문이 닫혀있을 때
       {
          for (int x = 0; x < 19; ++x)
          {
            myStepper.step(-stepsPerRevolution);  // 연다
          }

          motion_state = 1;
       }
       else                           // 창문이 열려있을 때
       {
          for (int x = 0; x < 19; ++x)
          {
            myStepper.step(stepsPerRevolution);  // 닫는다
          }

          motion_state = 0;
       }
    }
    
    prev_motion = curr_motion;
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

void setup_client()
{
   Serial.println("Starting to connect...");
   Serial.print("Connecting URL: ");
   Serial.println(URL);

   if (!client.connect(URL, PORT))
   {
      Serial.println("Error.");
      while (true);
   }

   Serial.println("Complete.");
}

void get_from_client()
{
   client.println("GET /data/2.5/weather?q=" + LOCATION + "&appid=" + SERVICE_KEY + "&mode=xml");
   client.print("HOST: api.openweathermap.org\n");
   client.println("User-Agent: launchpad-wifi");
   client.println("Connection: close");
   client.println();

   Serial.println();
   Serial.println("Weather information for " + LOCATION);
   
   establish_state = 1;
}

int month_substring(String input)
{
   char tmpMonth[3];
   int result;
   
   input = input.substring(7, 9);
   Serial.println();
   Serial.println(input);

   input.toCharArray(tmpMonth, sizeof(tmpMonth));
   

   result = atoi(tmpMonth);

   Serial.println();
   Serial.println(result);
   
   return result;
}

int string_to_int(String input)
{
   int i = 2;         // =" 두개를 제외하고 시작
   char changeVal[20];
   int result;

   while(input[i] != '"')
   {
      i++;
   }

   input = input.substring(2, i);

   input.toCharArray(changeVal, sizeof(changeVal));

   result = atoi(changeVal);
   return result;
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
