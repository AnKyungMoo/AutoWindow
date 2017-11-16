#include <EEPROM.h>
#define BTN_PIN 2

int power_state = 0; // 0이면 꺼짐, 1이면 켜짐
int addr = 0;  // 주소
bool win = false;

int currbtn;
int prevbtn = -1;

void setup()
{
  Serial.begin(9600);
  pinMode(BTN_PIN, INPUT);


  win = EEPROM.read(0);
  Serial.println(win);
}

void loop() 
{
  currbtn = digitalRead(BTN_PIN);   // on & off 처리
  
  if(currbtn == 1 && prevbtn == 0)  // on & off 확인
  {
    if(power_state == 0)
    {
      power_state = 1;
      Serial.println("켠다.");  
    }
    else
    {
      power_state = 0;
      if(win)
        win = false;
      else
        win = true;

      EEPROM.write(addr, win);

      Serial.println("끈다.");
    }
  }
  prevbtn = currbtn;
  delay(100);
}
