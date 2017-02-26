#include <LiquidCrystal.h>

LiquidCrystal lcd(7, 8, 9, 1, 11, 12); //mkr
#define LCD_POWER_PIN 1 //mkr
#define LCD_WIDTH 20
#define LCD_HEIGHT 4
unsigned long displayTimer = 0;
int displayOnTime = 12000;


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
unsigned long lastEncoderChange;


void setup() {
  
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

  lcd.begin(LCD_WIDTH, LCD_HEIGHT, 1);
  lcd.setCursor(0, 0);
  lcd.print("Seebier");
  lcd.setCursor(0, 1);
  lcd.print("Gaerkuehlung");

  //digitalWrite(LCD_POWER_PIN, HIGH);
}

void loop() {


  //show Display
  digitalWrite(LCD_POWER_PIN, HIGH);
  Serial.println("ON");
  delay(1500);
  digitalWrite(LCD_POWER_PIN, LOW);
  Serial.println("OFF");
  delay(1500);
  
  /*if (millis() - displayTimer < displayOnTime || millis() - lastEncoderChange < displayOnTime) {
    //lcd.display();
    digitalWrite(LCD_POWER_PIN, HIGH);
  } else {
    //lcd.noDisplay();u
    digitalWrite(LCD_POWER_PIN, LOW);
  }
  */

}
