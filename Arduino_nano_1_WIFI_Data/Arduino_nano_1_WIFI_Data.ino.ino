#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>
#include <SoftwareSerial.h>
#include <U8x8lib.h>
int value = 0;
float ctemp;

U8X8_SSD1306_128X32_UNIVISION_HW_I2C u8x8(U8X8_PIN_NONE);
//新建一个softSerial对象，rx:6,tx:5
SoftwareSerial a(6,5);
 

void setup()
{
  Serial.begin(9600);
  a.begin(9600);
  a.listen();

 Mirf.spi = &MirfHardwareSpi;
  Mirf.init();
 
  Mirf.setRADDR((byte *)"WIFID"); 
  Mirf.payload = 2;   
  Mirf.channel = 11; 
  Mirf.config(); 
  Serial.println("Listening..."); 
    u8x8.begin();
  u8x8.setPowerSave(0);
}
 
void loop()
{  
  if(!Mirf.isSending() && Mirf.dataReady()) 
  {
    Mirf.getData((byte *) &value);  
    Serial.println(value);
    if(10000 < value && value< 20000)
    {
      ctemp = (static_cast<float>(value) - 10000)/100;
      Serial.print("ctemp");Serial.println(ctemp);
    }
    a.print(value);
  
  }
    /*u8x8.setFont(u8x8_font_px437wyse700a_2x2_r); //两行字体，粗
 
  
  String s = "Box temp" ;
  String s2;
  u8x8.drawString(0,0,s.c_str());

 
  s2 = ctemp;
 
  u8x8.drawString(0,2,s2.c_str());*/
  String command = a.readString();
  int b = (command.substring(3,7)).toInt() + 10000;
  int w;
  if(b != 10000)
  {  
    w = 10;
    while(w != 0)
    {
      Serial.println(b);
      Mirf.setTADDR((byte *)"MAINC");          
      Mirf.send((byte *)&b);                
      while(Mirf.isSending()) delay(1000);
      delay(1000);
      Mirf.setTADDR((byte *)"AIRCO");          
      Mirf.send((byte *)&b);                
      while(Mirf.isSending()) delay(1000);
      w--;   
    }
  }
}
