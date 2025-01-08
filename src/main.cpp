#include <Arduino.h>
#include <Wire.h>
#include "LiquidCrystal_I2C.h"
#include "EEPROM.h"
#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <SHT1x.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "JkJ3hAJmwij2h1XpTJPpkDFBGbOvE_uh";
// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "TP-LINK_6E26";
char pass[] = "12201662";
BlynkTimer timer;
// untested

// button
#define none_button   0
#define left_button   4 //4
#define right_button  25 //5
#define up_button     26 //6
#define down_button   27 //7
const unsigned short button_array[] = {left_button, right_button, up_button, down_button};
unsigned long delay_time = 0;
// sht10
#define dataPin  23
#define clockPin 18
SHT1x sht1x(dataPin, clockPin);
float air_humi = 0, air_temp = 0;
float max_air_humi, min_air_humi;
// moist sensor
#define moist 36 //A0
float soil_humi   = 0;
float max_soil_humi, min_soil_humi;
// lcd
LiquidCrystal_I2C lcd(0x27, 20, 4);
// pump
#define pump_air   19 //12
#define pump_soil  0 //11
#define pump_on  1
#define pump_off 0
// control system
float set_max_1(float set_max, float set_min);
float set_max_2(float set_max, float set_min);
float set_max_3(float set_max, float set_min);
float set_max_4(float set_max, float set_min);
float set_min_1(float set_max, float set_min);
float set_min_2(float set_max, float set_min);
float set_min_3(float set_max, float set_min);
float set_min_4(float set_max, float set_min);
void save_setting();
//beep signal
#define beep 2
// 
int column_setup = 1;
int row_setup = 1;
int lcd_time = 3;
// arrow  
byte arrow_down[8]=
{
  B00100,
  B00100,
  B00100,
  B00100,
  B10101,
  B01110,
  B00100,
  B00000,
} ;
byte arrow_up[8]=
{
  B00000,
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B00100,
  B00100,
} ;
void clear_arrow();

// Blynk
void sendSensor()
{
  Blynk.virtualWrite(V0, air_temp);
  Blynk.virtualWrite(V1, air_humi);
  Blynk.virtualWrite(V2, soil_humi);
  Blynk.virtualWrite(V3, min_air_humi);
  Blynk.virtualWrite(V4, max_air_humi);
  Blynk.virtualWrite(V5, min_soil_humi);
  Blynk.virtualWrite(V6, max_soil_humi);  
}
BLYNK_WRITE(V3)
{
  min_air_humi = param.asFloat();
  if (min_air_humi >= max_air_humi) 
  {
    min_air_humi = max_air_humi - 0.5;
  }
  Serial.print("V3 Slider value is: ");
  Serial.println(min_air_humi);
  save_setting();
}
BLYNK_WRITE(V4)
{
  max_air_humi = param.asFloat();
  if (max_air_humi <= min_air_humi) 
  {
    max_air_humi = min_air_humi + 0.5;
  }
  Serial.print("V4 Slider value is: ");
  Serial.println(max_air_humi);
  save_setting();
}
BLYNK_WRITE(V5)
{
  min_soil_humi = param.asFloat();
  if (min_soil_humi >= max_soil_humi) 
  {
    min_soil_humi = max_soil_humi - 5;
  }
  Serial.print("V5 Slider value is: ");
  Serial.println(min_soil_humi);
  save_setting();
}
BLYNK_WRITE(V6)
{
  max_soil_humi = param.asFloat();
  if (max_soil_humi <= min_soil_humi) 
  {
    max_soil_humi = min_soil_humi + 5;
  }
  Serial.print("V6 Slider value is: ");
  Serial.println(max_soil_humi);
  save_setting();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Wire.begin(13, 15);
  Blynk.begin(auth, ssid, pass);
  for(int x = 0; x < 4; x++)
  {
    pinMode(button_array[x], INPUT);
  }
  pinMode(moist, INPUT);
  lcd.init();
  lcd.noBacklight();
  pinMode(pump_air, OUTPUT);
  digitalWrite(pump_air, pump_on);
  pinMode(pump_soil, OUTPUT);
  digitalWrite(pump_soil, pump_on);
  lcd.createChar(1, arrow_down);
  lcd.createChar(2, arrow_up);
  pinMode(beep, OUTPUT);
  digitalWrite(beep, LOW);
  EEPROM.begin(20);
  max_air_humi = EEPROM.readFloat(12);
  min_air_humi = EEPROM.readFloat(8);
  max_soil_humi = EEPROM.readFloat(4);
  min_soil_humi = EEPROM.readFloat(0);
  timer.setInterval(1000L, sendSensor);
}

int read_button() 
{
  for(int x = 0; x < 4; x++)
  {
    if(digitalRead(button_array[x]) == 1)
    {
      lcd_time = millis();
      delay_time = millis();
      lcd.backlight();      
      digitalWrite(beep, HIGH);
      delay(50);
      digitalWrite(beep, LOW);
      delay(200);
      Serial.println(button_array[x]);
      return button_array[x];
    }
  }
  return none_button;
}

void save_setting() 
{
  EEPROM.writeFloat(12, max_air_humi);
  EEPROM.writeFloat(8, min_air_humi);
  EEPROM.writeFloat(4 ,max_soil_humi);
  EEPROM.writeFloat(0, min_soil_humi);
  EEPROM.commit();
}

float set_max_1(float set_max, float set_min)
{
  column_setup = 1;
    switch (read_button())
    {
    case up_button:
      if(set_max < 100 - 10) set_max = set_max + 10;
      break;
    case down_button:
      if(set_max >= set_min + 10) set_max = set_max - 10;
      break;
    case right_button:
      set_max_2(set_max, set_min);
      break;
    case left_button:
      if (row_setup == 1) row_setup = 2;
      else if (row_setup == 2) row_setup = 1;
      set_min_4(set_max, set_min);
      break;

    default:
      break;
    }
  return set_max;
}

float set_max_2(float set_max, float set_min)
{
  column_setup = 2;
    switch (read_button())
    {
    case up_button:
      if(set_max < 100 - 1) set_max = set_max + 1;
      break;
    case down_button:
      if(set_max >= set_min + 1) set_max = set_max - 1;;
      break;
    case right_button:
      set_max_3(set_max, set_min);
      break;
    case left_button:
      set_max_1(set_max, set_min);
      break;

    default:
      break;
    }
  return set_max;
}

float set_max_3(float set_max, float set_min)
{
  column_setup = 3;
    switch (read_button())
    {
    case up_button:
      if(set_max < 100 - 0.1) set_max = set_max + 0.1;
      break;
    case down_button:
      if(set_max >= set_min + 0.1) set_max = set_max - 0.1;
      break;
    case right_button:
      set_max_4(set_max, set_min);
      break;
    case left_button:
      set_max_2(set_max, set_min);
      break;

    default:
      break;
    }
  return set_max;
}

float set_max_4(float set_max, float set_min)
{
  column_setup = 4;
    switch (read_button())
    {
    case up_button:
      if(set_max < 100 - 0.01) set_max = set_max + 0.01;
      break;
    case down_button:
      if(set_max >= set_min + 0.01) set_max = set_max - 0.01;
      break;
    case right_button:
      set_min_1(set_max, set_min);
      break;
    case left_button:
      set_max_3(set_max, set_min);
      break;

    default:
      break;
    }
  return set_max;
}

float set_min_1(float set_max, float set_min)
{
  column_setup = 5;
    switch (read_button())
    {
    case up_button:
      if(set_min < set_max - 10) set_min = set_min + 10;
      break;
    case down_button:
      if(set_min >= 10) set_min = set_min - 10;
      break;
    case right_button:
      set_min_2(set_max, set_min);
      break;
    case left_button:
      set_max_4(set_max, set_min);
      break;

    default:
      break;
    }
  return set_min;
}

float set_min_2(float set_max, float set_min)
{
  column_setup = 6;
    switch (read_button())
    {
    case up_button:
      if(set_min < set_max - 1) set_min = set_min + 1;
      break;
    case down_button:
      if(set_min >= 0 + 1) set_min = set_min - 1;
      break;
    case right_button:
      set_min_3(set_max, set_min);
      break;
    case left_button:
      set_min_1(set_max, set_min);
      break;

    default:
      break;
    }
  return set_min;
}

float set_min_3(float set_max, float set_min)
{
  column_setup = 7;
    switch (read_button())
    {
    case up_button:
      if(set_min < set_max - 0.1) set_min = set_min + 0.1;
      break;
    case down_button:
      if(set_min >= 0 + 0.1) set_min = set_min - 0.1;
      break;
    case right_button:
      set_min_4(set_max, set_min);
      break;
    case left_button:
      set_min_2(set_max, set_min);
      break;

    default:
      break;
    }
  return set_min;
}

float set_min_4(float set_max, float set_min)
{
  column_setup = 8;
    switch (read_button())
    {
    case up_button:
      if(set_min < set_max - 0.01) set_min = set_min + 0.01;
      break;
    case down_button:
      if(set_min >= 0 + 0.01) set_min = set_min - 0.01;
      break;
    case right_button:
      if (row_setup == 1) row_setup = 2;
      else if (row_setup == 2) row_setup = 1;
      set_max_1(set_max, set_min);
      break;
    case left_button:
      set_min_3(set_max, set_min);
      break;

    default:
      break;
    }
  return set_min;
}

void clear_arrow() 
{
  lcd.setCursor(9, 0);
    lcd.print("           ");
    lcd.setCursor(9, 3);
    lcd.print("           ");
}

void loop() {
  float tempo = 0;
  // put your main code here, to run repeatedly:
  if (millis() >= (unsigned long)60000 * 3 + lcd_time)
  {
    lcd.noBacklight();
  }
  if (millis() >= (unsigned long)3000 + delay_time) {
    delay_time = millis();
    save_setting();
    // sht10
    tempo = sht1x.readHumidity();
    if (tempo >= 0 && tempo < 100)  {
      air_humi = tempo;
    }
    tempo = sht1x.readTemperatureC();
    if (tempo >= 0 && tempo < 100) {
      air_temp = tempo;
    }
    if(air_humi >= max_air_humi)
    {
      digitalWrite(pump_air, pump_off);
    }
    if (air_humi <= min_air_humi)
    {
      digitalWrite(pump_air, pump_on);
    }
  }
  // moist sensor
    tempo = analogRead(moist);
    tempo = map(tempo, 0, 1800, 100, 0);
    if (tempo >=0 && tempo < 100) {
      soil_humi = tempo;
    }
    if(soil_humi >= max_soil_humi)
    {
      digitalWrite(pump_soil, pump_off);
    }
    if (soil_humi <= min_soil_humi)
    {
      digitalWrite(pump_soil, pump_on);
    }
  /* lcd 20x4 */
  lcd.setCursor(0, 0);
  if (air_temp < 10 ) lcd.print(" ");
  lcd.print(air_temp);
  lcd.setCursor(5, 0);
  lcd.print(" C ");
  lcd.setCursor(0, 1);
  if (air_humi < 10 ) lcd.print(" ");
  lcd.print(air_humi);
  lcd.setCursor(5, 1);
  lcd.print(" % ");
  lcd.setCursor(0, 2);
  if (soil_humi < 10 ) lcd.print(" ");
  lcd.print(soil_humi);
  lcd.setCursor(5, 2);
  lcd.print(" % ");
  lcd.setCursor(9, 1);
  if (max_air_humi < 10 ) lcd.print(" ");
  lcd.print(max_air_humi, 2);
  lcd.print(" ");
  lcd.setCursor(15, 1);
  if (min_air_humi < 10 ) lcd.print(" ");
  lcd.print(min_air_humi, 2);
  lcd.setCursor(9, 2);
  if (max_soil_humi < 10 ) lcd.print(" ");
  lcd.print(max_soil_humi, 2);
  lcd.print(" ");
  lcd.setCursor(15, 2);
  if (min_soil_humi < 10 ) lcd.print(" ");
  lcd.print(min_soil_humi, 2);
  // column_setup
  if (row_setup == 1)
  {
    switch (column_setup)
    {
    case 1:
      clear_arrow();
      lcd.setCursor(9, 0);
      lcd.write(1);
      max_air_humi = set_max_1(max_air_humi, min_air_humi);
      break;
    case 2:
      clear_arrow();
      lcd.setCursor(10, 0);
      lcd.write(1);
      max_air_humi = set_max_2(max_air_humi, min_air_humi);
      break;
    case 3:
      clear_arrow();
      lcd.setCursor(12, 0);
      lcd.write(1);
      max_air_humi = set_max_3(max_air_humi, min_air_humi);
      break;
    case 4:
      clear_arrow();
      lcd.setCursor(13, 0);
      lcd.write(1);
      max_air_humi = set_max_4(max_air_humi, min_air_humi);
      break;
    case 5:
      clear_arrow();
      lcd.setCursor(15, 0);
      lcd.write(1);
      min_air_humi = set_min_1(max_air_humi, min_air_humi);
      break;
    case 6:
      clear_arrow();
      lcd.setCursor(16, 0);
      lcd.write(1);
      min_air_humi = set_min_2(max_air_humi, min_air_humi);
      break;
    case 7:
      clear_arrow();
      lcd.setCursor(18, 0);
      lcd.write(1);
      min_air_humi = set_min_3(max_air_humi, min_air_humi);
      break;
    case 8:
      clear_arrow();
      lcd.setCursor(19, 0);
      lcd.write(1);
      min_air_humi = set_min_4(max_air_humi, min_air_humi);
      break;

    default:
      break;
    }
  }

  if (row_setup == 2)
  {
    switch (column_setup)
    {
    case 1:
      clear_arrow();
      lcd.setCursor(9, 3);
      lcd.write(2);
      max_soil_humi = set_max_1(max_soil_humi, min_soil_humi);
      break;
    case 2:
      clear_arrow();
      lcd.setCursor(10, 3);
      lcd.write(2);
      max_soil_humi = set_max_2(max_soil_humi, min_soil_humi);
      break;
    case 3:
      clear_arrow();
      lcd.setCursor(12, 3);
      lcd.write(2);
      max_soil_humi = set_max_3(max_soil_humi, min_soil_humi);
      break;
    case 4:
      clear_arrow();
      lcd.setCursor(13, 3);
      lcd.write(2);
      max_soil_humi = set_max_4(max_soil_humi, min_soil_humi);
      break;
    case 5:
      clear_arrow();
      lcd.setCursor(15, 3);
      lcd.write(2);
      min_soil_humi = set_min_1(max_soil_humi, min_soil_humi);
      break;
    case 6:
      clear_arrow();
      lcd.setCursor(16, 3);
      lcd.write(2);
      min_soil_humi = set_min_2(max_soil_humi, min_soil_humi);
      break;
    case 7:
      clear_arrow();
      lcd.setCursor(18, 3);
      lcd.write(2);
      min_soil_humi = set_min_3(max_soil_humi, min_soil_humi);
      break;
    case 8:
      clear_arrow();
      lcd.setCursor(19, 3);
      lcd.write(2);
      min_soil_humi = set_min_4(max_soil_humi, min_soil_humi);
      break;

    default:
      break;
    }
  }
  Blynk.run();
  timer.run();
}
