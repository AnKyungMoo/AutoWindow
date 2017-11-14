#include "SPI.h"
#include "WiFi.h"

#define SERVICE_KEY String("89b33429e954137534d5f996da975b2e")
#define LOCATION   String("Asan")
#define PORT      80

char SSID[] = "m620_24g";
const char PASS[] = "digitallogic";
const char URL[] = "api.openweathermap.org";

WiFiClient client;

int establish_state = 0;

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
