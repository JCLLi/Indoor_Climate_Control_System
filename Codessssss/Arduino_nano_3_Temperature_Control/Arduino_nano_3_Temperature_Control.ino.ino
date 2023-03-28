#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>
#include <SoftwareSerial.h>

float ptemp[45] = {20, 21, 20, 21, 20, 21, 20, 21, 22, 20, 21, 22, 20, 21, 22, 20, 21, 22, 20, 21, 22, 20, 21, 20, 19, 20, 19, 20, 19, 20, 19, 20, 19, 20, 19, 20, 19, 20, 19, 20, 19, 20, 19, 20, 19};
int value;
int cstate;
int corder = -2;
int lstate = 0;
int lorder = -1;
float stemp = 20;
int mode;
int lmode = -1;
float commandtemp = 0;
int commandsign = 0;
int repeatsign = 0;
int judgesign = 0;
int cmode = 0;
 
void setup()
{
  Serial.begin(9600);
  
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);
  pinMode(6, OUTPUT);
  digitalWrite(6, HIGH);
  
  Mirf.spi = &MirfHardwareSpi;
  Mirf.init();
 
  Mirf.setRADDR((byte *)"AIRCO"); 
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
    
    Serial.print("Recive:");Serial.println(value);
    
    if(value < 100)
    {
      corder = value;
      cstate = 0;
      commandsign = 0;
      judgesign = 0;
      repeatsign = 0;
    }
    else
    {
      if(value > 10000)
      {
        commandtemp = (static_cast<float>(value - 10000))/100;
        Serial.print("Command temp: ");Serial.println(commandtemp);
      }
      else
      {
        if(value == 200)
        {
          commandsign = 1;
          repeatsign = 1;
        }
        else
        {
          if(value >= 300)
          {
            if(repeatsign == 1)
            {
              lorder = -1;
              commandsign = 0;
              judgesign = 0;
              repeatsign = 0;
            }
            cstate = 1;     
          }
          else
          {
            corder = value - 100;
            cstate = 1;
          }
        }
      }
    }
  Serial.print("CS: ");Serial.println(commandsign);
    /*Serial.print("corder: ");Serial.println(corder);
    Serial.print("cstate: ");Serial.println(cstate);
    Serial.print("lorder: ");Serial.println(lorder);
    Serial.print("lstate: ");Serial.println(lstate); 
    Serial.print("mode: ");Serial.println(mode);
    Serial.print("lmode: ");Serial.println(lmode);
    Serial.println();*/
  if(cstate == 1 && commandsign ==0)
  {
      
    if(corder != lorder)
    {
      Serial.println();Serial.println("**********next temp***********");
      judge();
      Serial.println("set temp adjustment finished");
    }
      
    if(corder == lorder)
    {
      if(cstate != lstate)
      {
      mode = lmode;
      if(mode == 0)
      {
        digitalWrite(4, LOW);
        delay(100);
        digitalWrite(4, HIGH);
        delay(1000);
        Serial.print("mode: heating");
      }
      if(mode == 1)
      {
        digitalWrite(4, LOW);
        delay(100);
        digitalWrite(4, HIGH);
        delay(1000);
        Serial.print("mode: cooling");
       }
      }
     }
    }

    if(cstate == 0 && commandsign ==0)
    {
        /*  Serial.print("cstate = "); Serial.println(cstate);
        Serial.print("lstate = "); Serial.println(lstate);
        Serial.print("mode: ");Serial.println(mode);
        Serial.println();*/
      if(cstate != lstate)
      {
        // Serial.print("mode: ");Serial.println(mode);
        if(mode == 1)
        {
          mode = 3;
          digitalWrite(4, LOW);
          delay(100);
          digitalWrite(4, HIGH);
          delay(1000);
          Serial.print("off");
        }
        if(mode == 0)
        {
          Serial.println("off");
          digitalWrite(4, LOW);
          delay(100);
          digitalWrite(4, HIGH);
          delay(1000);
          digitalWrite(4, LOW);
          delay(100);
          digitalWrite(4, HIGH);
          delay(1000);
          mode = 3;
        }
      }
    }
  }

  if(commandsign == 1 && judgesign == 0)
  {
    Serial.println("**********New command************");
    Serial.print("CS1: ");Serial.println(commandsign);
    judge(); 
    judgesign = 1;
    Serial.println("Command temp adsjustment finished");
  }
  lstate = cstate;
  lorder = corder;
}

void judge()
{
  int i = 0;
  int y = 0;
  float catemp = 0;
  if(commandsign == 0)
  {
    catemp = ptemp[corder];
  }
  if(commandsign == 1)
  {
    catemp = commandtemp;
  }
  Serial.print("catemp");Serial.println(catemp);
  if(stemp < catemp)
  {
    if (mode == 3)
    {
      digitalWrite(4, LOW);
      delay(100);
      digitalWrite(4, HIGH);
      delay(1000);
       Serial.print("mode: heating"); 
       cmode = 1;
    }
    mode = 0;//increase
    while(stemp < catemp)
    {
      stemp = stemp + 0.5;
      Serial.print("stemp");Serial.println(stemp);
      i++;
      delay(1000);
    }
  Serial.print("Desired temp: ");Serial.println(catemp);
  Serial.print("result temp: ");Serial.println(stemp);
  Serial.print("push button times: ");Serial.println(i);delay(2000);
   push_button(i, mode);
   cmode = 0;
   lmode = mode;
  }
  else
  {
    if(stemp > catemp)
    {
      if (mode == 3)
    {
      digitalWrite(4, LOW);
      delay(100);
      digitalWrite(4, HIGH);
      delay(1000);
      digitalWrite(4, LOW);
      delay(100);
      digitalWrite(4, HIGH);
      delay(1000);
       Serial.print("mode: cooling");
       cmode = 1;
    }
      mode = 1;//decrease
      while(stemp > catemp)
      {
        stemp = stemp - 0.5;
        Serial.print("stemp");Serial.println(stemp);
        y++;
        delay(1000);
      }
        Serial.print("Desired temp: ");Serial.println(catemp);
    Serial.print("result temp: ");Serial.println(stemp);
    Serial.print("push button times: ");Serial.println(y);delay(2000);
      push_button(y, mode);
      cmode = 0;
      lmode = mode;
    }
  }
}

void push_button(int a, int b)
{
  if((lmode != -1) && (mode != lmode) && (cmode == 0))
  {
    if(mode == 0)
    {
      digitalWrite(4, LOW);
      delay(100);
      digitalWrite(4, HIGH);
      delay(1000);
      digitalWrite(4, LOW);
      delay(100);
      digitalWrite(4, HIGH);
      delay(1000); 
       Serial.print("mode: heating");
    }
    if(mode == 1)
    {
      digitalWrite(4, LOW);
      delay(100);
      digitalWrite(4, HIGH);
      delay(1000);
      Serial.print("mode: cooling");
    }
  }
  if(b == 0)
  { 
    while(a != 0)
    {
      digitalWrite(5, LOW);
      Serial.print("up");
      delay(100);
      digitalWrite(5, HIGH);
      delay(3000);
      
      a--;
    }
  }
  
  if(b == 1)
  {
    while(a != 0)
    {
      digitalWrite(6, LOW);
      Serial.print("down");
      delay(100);
      digitalWrite(6, HIGH);
      delay(3000);
      
      a--;
    }
  }
}
