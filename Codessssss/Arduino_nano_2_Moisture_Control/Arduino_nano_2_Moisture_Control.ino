#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>

int value;
int dehumiswitch = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(6, OUTPUT);
  digitalWrite(6, HIGH);
  pinMode(5, OUTPUT);
  digitalWrite(5, LOW);
  
  Mirf.spi = &MirfHardwareSpi;
  Mirf.init();
 
  Mirf.setRADDR((byte *)"MOICO"); 
  Mirf.payload = 2;   
  Mirf.channel = 11; 
  Mirf.config(); 
  Serial.println("Listening..."); 
}

void loop() 
{
  if(!Mirf.isSending() && Mirf.dataReady()) 
  {
    Mirf.getData((byte *) &value);
    Serial.println(value);
    if(value == 0)
    {
      if(dehumiswitch != 0)
      {
        digitalWrite(6, LOW);
        delay(200);
        digitalWrite(6, HIGH);
        dehumiswitch = 0;
      }
      digitalWrite(5, LOW);
      Serial.println("OFF");
    }
    if(value == 1)
    {
      if(dehumiswitch != 0)
      {
        digitalWrite(6, LOW);
        delay(200);
        digitalWrite(6, HIGH);
        dehumiswitch = 0;
      }
      digitalWrite(5, HIGH);
      Serial.println("UP");
    }
    if(value == 2)
    {
      if(dehumiswitch == 0)
      {
        digitalWrite(6, LOW);
        delay(200);
        digitalWrite(6, HIGH);
        dehumiswitch = 1;
      }
      digitalWrite(5, LOW);
      Serial.println("DOWN");
    }
  }
}
