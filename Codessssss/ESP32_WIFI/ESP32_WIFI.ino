#include <WiFi.h>
#include <HardwareSerial.h>
#define RXD2 16
#define TXD2 17

/*char ssid[] = "DeRuyter2.4GHz"; 
char pass[] = "student2017";*/

char ssid[] = "OnePlus7Pro"; 
char pass[] = "da991026";

WiFiClient  client;
char myTalkBackID[] = "42666";
char myTalkBackKey[] = "547GS0R76W5DUDYY";
String thingspeak_key = "VNKYOOFQ43GV1OGL"; 

float ctemp = 0;
float chumi = 0;
float wtemp =0;

void setup() 
{
  Serial.begin(115200); 
  Serial2.begin(9600,SERIAL_8N1,16,17);
  WiFi.mode(WIFI_STA);
}

void loop() 
{

  
  if(WiFi.status() != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(String(ssid));
    while(WiFi.status() != WL_CONNECTED)
    {
      WiFi.begin(ssid, pass);  
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected.");
  }

   readdata();
     if (client.connect("api.thingspeak.com", 80)) 
    {
      upload();
      delay(1000);
    } 
  client.stop();
  
  String tbURI = String("/talkbacks/") + String(myTalkBackID) + String("/commands/execute");
  String postMessage =  String("api_key=") + String(myTalkBackKey);                      
  String newCommand = String();

  int x = httpPOST(tbURI, postMessage, newCommand);
  client.stop();
  
  if(x == 200)
  {
    Serial.println("checking queue...");  
    if(newCommand.length() != 0)
    {
      Serial.print("  Latest command from queue: ");
      Serial.println(newCommand);
      Serial2.print(newCommand);
    }
    else
    {
      Serial.println("  Nothing new.");  
    }
  }
  else
  {
    Serial.println("Problem checking queue. HTTP error code " + String(x));
  }
 
 
}



int httpPOST(String uri, String postMessage, String &response){

  bool connectSuccess = false;
  connectSuccess = client.connect("api.thingspeak.com",80);

  if(!connectSuccess)
  {
      return -301;   
  }
  
  postMessage += "&headers=false";
  
  String Headers =  String("POST ") + uri + String(" HTTP/1.1\r\n") +
                    String("Host: api.thingspeak.com\r\n") +
                    String("Content-Type: application/x-www-form-urlencoded\r\n") +
                    String("Connection: close\r\n") +
                    String("Content-Length: ") + String(postMessage.length()) +
                    String("\r\n\r\n");

  client.print(Headers);
  client.print(postMessage);
  long startWaitForResponseAt = millis();
  while(client.available() == 0 && millis() - startWaitForResponseAt < 5000){
      delay(100);
  }

  if(client.available() == 0){       
    return -304; // Didn't get server response in time
  }

  if(!client.find(const_cast<char *>("HTTP/1.1"))){
      return -303; // Couldn't parse response (didn't find HTTP/1.1)
  }
  
  int status = client.parseInt();
  if(status != 200){
    return status;
  }

  if(!client.find(const_cast<char *>("\n\r\n"))){
    return -303;
  }

  String tempString = String(client.readString());
  response = tempString;
  Serial.print("tempString=");Serial.println(tempString);

  return status;
}

void upload()
{
  String url = "/update?key=";
   url += thingspeak_key;
   url += "&field1=";
   url += String(ctemp);
   url += "&field2=";
   url += String(wtemp);
   client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: api.thingspeak.com\r\n" + 
               "Connection: close\r\n\r\n");

   Serial.println("done");
}
void readdata()
{
  String a =Serial2.readString();
  Serial.print("a:");Serial.println(a);
  int b = (a.substring(0,5)).toInt();
  datadecode(b);
  int c = (a.substring(5,5)).toInt();
  datadecode(c);
  int e = (a.substring(10,5)).toInt();
  datadecode(e);
}

void datadecode(int thdata)
{
  if(10000 < thdata && thdata< 20000)
  {
    ctemp = (static_cast<float>(thdata) - 10000)/100;
    Serial.print("ctemp");Serial.println(ctemp);
  }
  if(20000 < thdata && thdata< 30000)
  {
    wtemp = (static_cast<float>(thdata) - 20000)/100;
    Serial.print("wtemp");Serial.println(wtemp);
  }
  /*if(20000 < thdata && thdata< 30000)
  {
    chumi = (static_cast<float>(thdata) - 20000)/100;
    Serial.print("chumi");Serial.println(chumi);
  }
  if(30000 < thdata && thdata< 40000)
  {
    wtemp = (static_cast<float>(thdata) - 30000)/100;
    Serial.print("wtemp");Serial.println(wtemp);
  }*/
}
