#include <Arduino.h>
#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>
#include "SdFat.h"
#include "Adafruit_SHT31.h"
#include "Seeed_MCP9600.h"
#include "Wire.h"
extern "C" 
{ 
  #include "utility/twi.h"  // from Wire library, so we can do bus scanning
}
 
#define TCAADDR 0x70

#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
    #define SERIAL SerialUSB
#else
    #define SERIAL Serial
#endif


const uint8_t SD_CS_PIN     = 2;
const uint8_t SOFT_MOSI_PIN = 3;
const uint8_t SOFT_MISO_PIN = 4;
const uint8_t SOFT_SCK_PIN  = 5;

SoftSpiDriver<SOFT_MISO_PIN, SOFT_MOSI_PIN, SOFT_SCK_PIN> softSpi;
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(0), &softSpi)

MCP9600 sensor;
Adafruit_SHT31 sht31 = Adafruit_SHT31();
SdFat sd;
File file;

//float ptemp[2] = {31, 29.5};
float ptemp[45] = {20, 21, 20, 21, 20, 21, 20, 21, 22, 20, 21, 22, 20, 21, 22, 20, 21, 22, 20, 21, 22, 20, 21, 20, 19, 20, 19, 20, 19, 20, 19, 20, 19, 20, 19, 20, 19, 20, 19, 20, 19, 20, 19, 20, 19};
float ctemp = 0;
float ctemperror = 0.5;
float chumi = 0;
float wtemp = 0;
int acswitch = 1;
int moswitch = 0;
int order = 0;
int timer = 0;
int value = 0;
int command = 0;
int commandsign = 0;
float commandtemp = 0;

int season = 0;
int day = 1;
int minute = 0;
int lorder = 0;

void setup() 
{
  Serial.begin(9600);
  
  Serial.print("Connecting to SD card... ");
  if (!sd.begin(SD_CONFIG)) 
  {
    sd.initErrorHalt();
  }
  
  Mirf.spi = &MirfHardwareSpi;
  Mirf.init();
  Mirf.setRADDR((byte *)"MAINC"); 
  Mirf.payload = 2;
  Mirf.channel = 11;            
  Mirf.config();
  
  tcaselect(1);
  if (sensor.init(THER_TYPE_K)) 
  {
    Serial.println("sensor init failed!!");
  }
  thermocouple_config();
  
  tcaselect(0); 
  if (! sht31.begin(0x44))
  {
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }
}

void loop() 
{
  minute ++;
  Serial.println(commandsign);
  if(!Mirf.isSending() && Mirf.dataReady()) 
  {
    Mirf.getData((byte *) &command);
    Serial.print("value: ");Serial.println(command);
    if(command != 0)
    {
      Serial.print("command");Serial.println(command);
      commandtemp = (static_cast<float>(command - 10000))/100;
      Serial.print("Command temp: ");Serial.println(commandtemp);
      commandsign = 1;
    }
  }
  tcaselect(0);
  ambient_temp();
  if(lorder != order)
  {
    commandsign = 0;
    command = 0;
    Serial.println("**********next temp**********");
    acswitch = 1;
  }
  acorder_judge(); 
  acswitch_judge();
  moswitch_judge();
   //Serial.print("order: ");Serial.println(order);

  
  delay(2000);  
  lorder = order;
  tcaselect(1);
  wtemp = wall_temp();
  SDstorage();
  delay(2000);  
  send2WIFI();
  delay(2000);
}





/**************************************************************************************************/

void acorder_judge()
{
  if(season == 0)
  {
  
    if(day == 1 || day == 2 || day == 3 || day == 9)
    {
      if(minute == 481)
      {
        order ++;
        Serial.print("order: ");Serial.println(order);
        Serial.print("minute: ");Serial.println(minute);
        Serial.print("day: ");Serial.println(day);
        Serial.print("Season: ");Serial.println(season);
        Serial.print("Temperature: ");Serial.println(ptemp[order]);
        Serial.println();
      }
      if(minute == 1441)
      {
        order ++;
        minute = 0;
        day++;
        Serial.print("order: ");Serial.println(order);
        Serial.print("minute: ");Serial.println(minute);
        Serial.print("day: ");Serial.println(day);
        Serial.print("Season: ");Serial.println(season);
        Serial.print("Temperature: ");Serial.println(ptemp[order]);
        Serial.println();
      }
    }
    else
    {
      if(minute == 481 || minute == 721 || minute == 1201)
      {
        order ++;
        Serial.print("order: ");Serial.println(order);
        Serial.print("minute: ");Serial.println(minute);
        Serial.print("day: ");Serial.println(day);
        Serial.print("Season: ");Serial.println(season);
        Serial.print("Temperature: ");Serial.println(ptemp[order]);
        Serial.println();
      }
      if(minute == 1441)
      {
        minute = 0;
        day++;
      }
    }
    
    if(day == 10)
    {
      season = 1;
    }  
  }
  else
  {
    if(minute == 121 || minute == 541)
    {
      order ++;
      if(order == 45)
      {
        order = 0;
      }
      Serial.print("order: ");Serial.println(order);
      Serial.print("minute: ");Serial.println(minute);
      Serial.print("day: ");Serial.println(day);
      Serial.print("Season: ");Serial.println(season);
      Serial.print("Temperature: ");Serial.println(ptemp[order]);
      Serial.println();
  
    }
    if(minute == 1441)
    {
      minute = 0;
      day++;
    }
  
    if(day == 21)
    {
      day = 1;
      season = 0;
    }
  }
}

void acswitch_judge()
{
  if(commandsign == 1)
  {
    if(ctemp > commandtemp - 0.2 && ctemp < commandtemp + 0.2) commandsign = 3;
  }

  if(commandsign == 0)
  {
    if (order != 0)
    {
      if(ptemp[order - 1] < ptemp[order])
      {
        if(ctemp  > (ptemp[order] - 0.25))
        {
          acswitch = 0;
         // Serial.print("acswitch: ");Serial.println(acswitch);
         Serial.println("a");
        }
        else
        {
          acswitch = 1;
          //Serial.print("acswitch: ");Serial.println(acswitch);
        } 
      }
      else
      {
         if(ctemp  < (ptemp[order] + 0.25))
        {
          acswitch = 0;
         // Serial.print("acswitch: ");Serial.println(acswitch);
        }
        else
        {
          acswitch = 1;
          //Serial.print("acswitch: ");Serial.println(acswitch);
        }  
      }
    }
    else
    {
      if(ctemp  > (ptemp[order] - 0.25))
      {
        acswitch = 0;
       // Serial.print("acswitch: ");Serial.println(acswitch);
      }
      else
      {
        acswitch = 1;
        //Serial.print("acswitch: ");Serial.println(acswitch);
      } 
    }
  }

   if(commandsign == 3)
  {
    if(ctemp > ptemp[order] - 0.2 && ctemp < ptemp[order] + 0.2) 
    {
      commandsign = 0;
      command = 0;
    }
  }
}

void moswitch_judge()
{
  if (chumi > 51)
  {
    moswitch = 2;
  }
  else
  {
    if(chumi < 49) 
    {
      moswitch = 1;
    }
    else
    {
      moswitch = 0;
    }
  }
}

void tcaselect(uint8_t i) 
{
  if (i > 7) return;
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();  
}

void send2WIFI()
{
 
  int ambient_temp = ctemp * 100 + 10000;
  int wall_temp = wtemp * 100 + 20000;
  Mirf.setTADDR((byte *)"WIFID");          
  Mirf.send((byte *)&ambient_temp);                
  while(Mirf.isSending()) delay(100);
  Mirf.send((byte *)&wall_temp);
  while(Mirf.isSending()) delay(100);
  Serial.print("temp: ");Serial.println(ambient_temp);

  /*Mirf.setTADDR((byte *)"MOICO");          
  Mirf.send((byte *)&moswitch);                
  while(Mirf.isSending()) delay(100);*/

  delay(1000);
  if(commandsign == 0)
  { 
     delay(1000);
    int control = acswitch * 100 + order;
    Mirf.setTADDR((byte *)"AIRCO");          
    Mirf.send((byte *)&control);                
    while(Mirf.isSending()) delay(100);
  }
  if(commandsign == 1)
  {
    int control1 = 200;
    Mirf.setTADDR((byte *)"AIRCO");          
    Mirf.send((byte *)&control1);                
    while(Mirf.isSending()) delay(100);
  }
  if(commandsign == 3)
  {
    int control3 = 300 + order;
    Mirf.setTADDR((byte *)"AIRCO");          
    Mirf.send((byte *)&control3);                
    while(Mirf.isSending()) delay(100);
  }

 
  
 /* int ambient_humi = chumi* 100 + 20000;
  Mirf.setTADDR((byte *)"WIFID");  
  Mirf.send((byte *)&ambient_humi);
  while(Mirf.isSending()) delay(100);  

  Mirf.setTADDR((byte *)"AIRCO");          
  Mirf.send((byte *)&control);                
  while(Mirf.isSending()) delay(100);*/

 /* int wall_temp = wtemp * 100 + 20000;
  Mirf.setTADDR((byte *)"WIFID");  
  Mirf.send((byte *)&wall_temp);
  while(Mirf.isSending()) delay(100);
  Serial.print("W: ");Serial.println(wall_temp);*/
}

int reveivefWIFI()
{
  if(!Mirf.isSending() && Mirf.dataReady()) 
  {
    Mirf.getData((byte *) &command);

    Serial.print("Recive:");Serial.println(command);
  }
}

void SDstorage()
{
  file = sd.open("file.txt", FILE_WRITE);
  if (file) 
  {
    file.print("Ambient temperature(*C) = ");file.println(ctemp);
    file.print("Ambient humidity(*C) = ");file.println(chumi);
    file.print("Wall temperature(% )= ");file.println(wtemp);
    file.close();
    Serial.println("Done");
    Serial.print("Desired temp: ");Serial.println(ptemp[order]);
    Serial.print("time:");Serial.println(timer);
    Serial.println();
  }
}

float wall_temp()
{
  float temp = 0;
  u8 byte = 0;
  u8 stat = 0;
  
  thermocouple_temp(&temp);
  Serial.print("WALL TEMP:");
  Serial.println(temp);
  
  sensor.read_INT_stat(&stat);
  return temp;
}

void ambient_temp()
{
  ctemp = sht31.readTemperature();
  chumi = sht31.readHumidity(); 
  if (! isnan(ctemp))      // check if 'is not a number'
  {
    Serial.print("Temp *C = ");
    Serial.println(ctemp);
  }
  else
  {
    Serial.println("Failed to read temperature");
  }

  if (! isnan(chumi))      // check if 'is not a number'
  {
    Serial.print("Hum. % = ");
    Serial.println(chumi);
  }
  else
  {
    Serial.println("Failed to read humidity");
  }
  delay(1000);
}

err_t thermocouple_config() 
{
  err_t ret = NO_ERROR;
  CHECK_RESULT(ret, sensor.set_filt_coefficients(FILT_MID));

  for (int i = 0; i < 4; i++)
  {
      /*Conver temp num to 16bit data*/
      CHECK_RESULT(ret, sensor.set_alert_limit(i, sensor.covert_temp_to_reg_form(50 + i)));
      /*  Set hysteresis.for example,set hysteresis to 2â„ƒ,when the INT limitation is 30â„ƒ,interruption will be generated when
          the temp ecceed limitation,and the interruption flag will stay unless the temp below 30-2(limitation-hysteresis) 28â„ƒ. */
      CHECK_RESULT(ret, sensor.set_alert_hys(i, 2));

      /*Set when interruption generated the pin's status*/
      CHECK_RESULT(ret, sensor.set_alert_bit(i, ACTIVE_LOW));

      CHECK_RESULT(ret, sensor.clear_int_flag(i));

      /*default is comparator mode*/
      CHECK_RESULT(ret, sensor.set_alert_mode_bit(i, COMPARE_MODE));

      /*Set alert pin ENABLE.*/
     // CHECK_RESULT(ret, sensor.set_alert_enable(i, ENABLE));
   }

  /*device cfg*/
  CHECK_RESULT(ret, sensor.set_cold_junc_resolution(COLD_JUNC_RESOLUTION_0_25));
  CHECK_RESULT(ret, sensor.set_ADC_meas_resolution(ADC_14BIT_RESOLUTION));
  CHECK_RESULT(ret, sensor.set_burst_mode_samp(BURST_32_SAMPLE));
  CHECK_RESULT(ret, sensor.set_sensor_mode(NORMAL_OPERATION));

  return NO_ERROR;
}

err_t thermocouple_temp(float* value) 
{
  err_t ret = NO_ERROR;
  float hot_junc = 0;
  float junc_delta = 0;
  float cold_junc = 0;
  bool stat = true;

  CHECK_RESULT(ret, sensor.check_data_update(&stat));
  if (stat) {
      CHECK_RESULT(ret, sensor.read_hot_junc(&hot_junc));
      CHECK_RESULT(ret, sensor.read_junc_temp_delta(&junc_delta));

      CHECK_RESULT(ret, sensor.read_cold_junc(&cold_junc));

      *value = hot_junc;
  } else {
      SERIAL.println("data not ready!!");
  }

  return NO_ERROR;
}
