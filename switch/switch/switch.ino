#define BTN_PIN 2

int mode_state = 0;
int prevbtn = -1;
int currbtn = -1;

void setup() 
{
  Serial.begin(9600);
  pinMode(BTN_PIN, INPUT);
}

void loop() 
{
  currbtn = digitalRead(BTN_PIN);
  if(currbtn == 1 && prevbtn == 0)
  {
    if(mode_state == 0)
    {
      mode_state = 1;
      Serial.println("A");  
    }
    else
    {
      mode_state = 0;
      Serial.println("B");
    }
  }
  prevbtn = currbtn;
  delay(100);
}
