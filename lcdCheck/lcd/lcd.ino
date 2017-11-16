#include <LiquidCrystal.h>
 
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

int rainValue = 240;

void setup() 
{
  Serial.begin(9600);
  
  lcd.begin(16, 2);   // set up the LCD's number of columns and rows:
}
 
void loop() 
{
    lcd.setCursor(0, 0);
    lcd.print("Rain ");
    lcd.setCursor(5, 0);
    lcd.print(rainValue);
    
    lcd.setCursor(0, 1);
    
    if(rainValue++ > 250)
    {
      lcd.print("Rain!!");
    }

  delay(1000);
  
  Serial.println("OK");
}
