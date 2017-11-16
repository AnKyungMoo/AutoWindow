#include <LiquidCrystal.h>
#include <DHT11.h>

int pin=8;    
DHT11 dht11(pin);     
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
 
void setup() 
{
  Serial.begin(9600);
  
  lcd.begin(16, 2);   // set up the LCD's number of columns and rows:
}
 
void loop() 
{
  int err;
  float temp, humi;
  
  if((err=dht11.read(humi, temp))==0) //온도, 습도 읽어와서 표시
  {
    lcd.setCursor(0, 0);
    lcd.print("Temp ");
    lcd.setCursor(5, 0);
    lcd.print(temp);
    lcd.print(" T");
    lcd.setCursor(0, 1);
    lcd.print("humi ");
    lcd.setCursor(5, 1);
    lcd.print(humi);
    lcd.print(" H");
  } 
  else                                //  err변수에 0이 담기지 않을 경우
  {
    lcd.clear();                      // LCD 화면 비우기
    lcd.setCursor(0, 0);              // LCD 출력위치
    lcd.println();                    //줄 바꿈 (엔터)
    lcd.print("Error No :");          // "Error No :" 텍스트 출력
    lcd.print(err);                   // err 출력
    lcd.println();                    //줄 바꿈 (엔터)
  }
  delay(1000);
  
  Serial.println("OK");
}
