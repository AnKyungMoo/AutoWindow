#include "SPI.h"
#include "WiFi.h"

#define SERVICE_KEY String("89b33429e954137534d5f996da975b2e")
#define LOCATION   String("Asan")
#define PORT      80

char SSID[] = "will_204";
const char PASS[] = "16111711";
const char URL[] = "api.openweathermap.org";

WiFiClient client;

int establish_state = 0;

String currentLine = "";
String tempLine = "";
String dateLine = "";

boolean tempActive = false;
boolean dateActive = false;

int tempValue = 0;
int monthValue = 0;

void setup()
{
   Serial.begin(9600);
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
}

void loop()
{
   if (establish_state == 1) 
   {
      while (client.available())
      {
         char c = client.read();
         Serial.print(c);

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
