#include <OneWire.h>
#include <LiquidCrystal.h>
#include <Wire.h>
#include <ArduinoWiFi.h>


// LCD=======================================================
//Bibliothek initialisieren
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
#define LCD_POWER_PIN 13
#define LCD_WIDTH 20
#define LCD_HEIGHT 4
unsigned long displayTimer = 0;
int displayOnTime = 12000;

/* DS18B20 Temperaturchip i/o */
OneWire  ds(19);  // on pin 5sendt

#define ANZ_DS1820_SENSORS 2
byte addr[ANZ_DS1820_SENSORS][8];
int SignBit[ANZ_DS1820_SENSORS];

unsigned long sendTime = 0;
int temperatureAir = 0;  //Kommastelle mal 100
int temperatureLiquid = 0;  //Kommastelle mal 100
int tempMode = 0;

int sollTemp = 1850; //Temperature 19*100   //nicht über 21grad!!
int coolMode = 0;
boolean first = true;
boolean modeFirst = true;

/* Http Send */
unsigned long lastHttpSendTime = 0;
unsigned long httpSendTime = 1000;


/* Ventil*/
#define coolingRelayPin 18

/* ENCODER */
//these pins can not be changed 2/3 are special pins
#define encoderPin1 3
#define encoderPin2 2
#define encoderSwitchPin 4 //push button switch
//LEDs
#define ledpinR 16
#define ledpinG 15
#define ledpinB 14
//
volatile int lastEncoded = 0;
volatile long encoderValue = 0;
volatile long lastEncoderValue = 0;
long lastencoderValue = 0;
int lastMSB = 0;
int lastLSB = 0;

/* DC MOTOR PUMP*/
#define motorPin 5

/* Programm*/
unsigned long coolTimeStart = 0;
unsigned long maxCoolTime = 300000;
unsigned long maxRestTime = 0;
unsigned long restStartTime = maxRestTime;
unsigned long buttonPressedTime = 0;
int programMode = 0;
unsigned long lastEncoderChange;

void setup(void) {
  /* Setup encoder pins as inputs */
  pinMode(encoderPin1, INPUT);
  pinMode(encoderPin2, INPUT);
  pinMode(encoderSwitchPin, INPUT);
  digitalWrite(encoderPin1, HIGH); //turn pullup resistor on
  digitalWrite(encoderPin2, HIGH); //turn pullup resistor on
  digitalWrite(encoderSwitchPin, LOW); //turn pullup resistor on
  //call updateEncoder() when any high/low changed seen
  //on interrupt 0 (pin 2), or interrupt 1 (pin 3)
  attachInterrupt(0, updateEncoder, CHANGE);
  attachInterrupt(1, updateEncoder, CHANGE);

  pinMode(LCD_POWER_PIN, OUTPUT);
  pinMode(coolingRelayPin, OUTPUT);
  pinMode(motorPin, OUTPUT);
  digitalWrite(motorPin, LOW); //turn pullup resistor on

  lcd.begin(LCD_WIDTH, LCD_HEIGHT, 1);
  lcd.setCursor(0, 0);
  lcd.print("Seebier");
  lcd.setCursor(0, 1);
  lcd.print("Gaerkuehlung");

  for (int i = 0; i < ANZ_DS1820_SENSORS; i++) {
    if (!ds.search(addr[i]))
    {
      lcd.setCursor(0, 0);
      lcd.print("No more addresses.");
      ds.reset_search();
      delay(250);
      return;
    }
  }
  
  Wifi.begin();

  //blink
  digitalWrite(ledpinB, HIGH);
  delay(500);
  digitalWrite(ledpinB, LOW);

  lcd.clear();
}



///////////////////////////////////////////////
void loop(void) {
  if (digitalRead(encoderSwitchPin)) {
    //button pressed
    //lcd.setCursor(14, 3);
    //lcd.print("H");

    //digitalWrite(coolingRelayPin, HIGH);
    displayTimer = millis();

  } else {
    //Button not pressed
    //lcd.setCursor(14, 3);
    //lcd.print("L");

    //digitalWrite(coolingRelayPin, LOW);
    buttonPressedTime = millis();
  }

  //show Display
  if (millis() - displayTimer < displayOnTime || millis() - lastEncoderChange < displayOnTime) {
    //lcd.display();
    digitalWrite(LCD_POWER_PIN, HIGH);
  } else {
    //lcd.noDisplay();u
    digitalWrite(LCD_POWER_PIN, LOW);
  }

  switch (programMode) {
    case 0:
      runCooler();
      break;
    case 1:
      //edit sollTemp
      setSollTemp();
      break;
  }
}


void lcdPrintInt4(int _x, int _y, int _val) {
  //max 4diggets long numbers = 1000
  lcd.setCursor(_x, _y);
  lcd.print("    ");

  int xpos = 0;
  if (_val > 999)
    xpos = _x + 0;
  else if (_val > 99)
    xpos = _x + 1;
  else if (_val > 9)
    xpos = _x + 2;


  lcd.setCursor(xpos, _y);
  lcd.print(_val);
}

void lcdPrintInt3(int _x, int _y, int _val) {
  //max 3diggets long numbers = 1000
  lcd.setCursor(_x, _y);
  lcd.print("   ");

  int xpos = 0;
  if (_val > 99)
    xpos = _x;
  else if (_val > 9)
    xpos = _x + 1;
  else
    xpos = _x + 2;


  lcd.setCursor(xpos, _y);
  lcd.print(_val);
}


