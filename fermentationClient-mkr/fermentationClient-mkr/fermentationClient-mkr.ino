#include <Arduino.h>

#include <LiquidCrystal.h>
//#include <Wire.h>
#include <OneWire.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include <WiFi101.h>


// ====== wifi & mqtt==============================
const char* _SSID     = "SEE-AP";
const char* _PASSWORD = "homebrew";

char mqttBrokerIP[]       = "192.168.1.1";  //192.168.0.10  - seebier.local
int  mqttPort           = 1883;
char mqttClientName[]   = "freezer0";
char MQTT_TOPIC_TEMP_OUT[]      = "/freezer/f1/temperatures";     //receive on: f1 und f2
char MQTT_TOPIC_SUBSCRIBTION[] = "/freezer/f1/setValues";  //  /freezer/f1/setValues
char MQTT_TOPIC_RELAYTEST[] = "/relayTest/f1";
char MQTT_TOPIC_ACKN[] = "/freezer/f1/receivedMessage";

WiFiClient netClient;   //wifi cleint
MQTTClient mqttc;
IPAddress ipAdress;

boolean mqttReceiveTimeout = false;


// ====== lcd ==============================
LiquidCrystal lcd(7, 8, 9, 1, 11, 12); //mkr
#define LCD_POWER_PIN 0 //mkr
#define LCD_WIDTH 20
#define LCD_HEIGHT 4
unsigned long displayTimer = 0;
int displayOnTime = 12000;
int displayMode = 0;    //information set which is shown

// ====== encoder ========================
#define ENCODER_SWITCH_PIN 4 //push button switch
unsigned long lastEncoderButtonUpdate = 0;
int lastButtonState;
boolean buttonPressed = false;
boolean buttonClicked = false;
boolean buttonClickedFirst = true;
unsigned long displayOnTimeAfterClick = 15000;

// === Encoder LEDs ===
#define ledpinR 16  //not working jet!
#define ledpinG 15
#define ledpinB 14

// ====== Freezer Relay ========================
#define RELAY_PIN 19
int relayCMD = 0;

// ====== Screen Variables ========================
char targetDurationStr[15];   //"targetDurationStr" : "00D, 00:50:52"
float targetTemp = 0;

// ====== Temperature / DS18B20 Sensor ============
OneWire  ds(20);
#define ANZ_DS1820_SENSORS 2
byte tempSensorAddr[ANZ_DS1820_SENSORS][8];

float beerTemp = 0.0;
float airTemp = 0.0;
int tempMode = 0;
unsigned long lastTempRequestTime = 0;

// ====== Timer / Debug
int fermentationProgramMode = 0; //# 0=stopped; 1=running; 2=ended
#define LIFE_PIN 6
char celciusSign[] = "\337";  // \337C
unsigned long lastLifeToggleUpdate = 0;
boolean toggle = false;  //steady on light
unsigned long lastRelayTime = 0;  //last time when relay Test topic received
unsigned long  lastRefreshUpdate = 0;
boolean refresh = true;
boolean first = true;
unsigned long mqttMessageTimeout = 2000;
unsigned long lastMqttMessageReceived = 0;  //time when last mqtt msg received
unsigned long lastMqttSendTime = 0;         //time when last mqtt msg send

// Text Blink
boolean blinkText = false;       //cmd to blink a text
boolean blinkTextState = false; //state if Text is on or off while blinking
unsigned long lastblinkTextUpdate = 0;  //time when last changed blinkState

//LCD Backlight Blink Mode
int lcdBckLightMode = 0;  // 0=OFF; 1=ON; 2=BLINK
unsigned long lastButtonClicked = 0;  //time when last button pressed to turn on backlight of lcd
unsigned long lastBlinkDisplayLightUpdate = 0;  //when blinking display last was updated
boolean blinkDisplayLightState = false;


// ====== SETUP =================================================================================
void setup() {
  //##### debug ###
  Serial.begin(9600);
  pinMode(LIFE_PIN, OUTPUT);

  //#### lcd ####
  pinMode(LCD_POWER_PIN, OUTPUT);
  lcd.begin(LCD_WIDTH, LCD_HEIGHT, 1);
  digitalWrite(LCD_POWER_PIN, HIGH);

  //#### encoder ####
  pinMode(ENCODER_SWITCH_PIN, INPUT);
  //attachInterrupt(digitalPinToInterrupt(ENCODER_SWITCH_PIN), encoderButtonPressed, RISING);

  //#### relay ####
  pinMode(RELAY_PIN, OUTPUT);

  //#### connect temp sensors ####
  int sensorCount = 0;
  for (int i = 0; i < ANZ_DS1820_SENSORS; i++) {
    if (!ds.search(tempSensorAddr[i]))
    {
      lcd.setCursor(0, 0);
      lcd.print("Please connect 2 temperature sensors.");
      ds.reset_search();
      delay(250);
      return;
    }
    sensorCount++;
  }
  lcd.setCursor(0, 0);
  lcd.print("DS18-B20 Sensoren");
  lcd.setCursor(0, 1);
  lcd.print("count: ");
  lcd.print(sensorCount);
  lcd.setCursor(0, 2);
  // String addrString = String(tempSensorAddr[0]);
  // lcd.print("add1: ");
  // lcd.print(addrString);
  // lcd.setCursor(0, 3);
  // lcd.print("add2: ");
  // lcd.print(tempSensorAddr[1]);
  delay(1500);
  lcd.clear();

  //#### wifi ####
  connectWifi();

  //wif connected !
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi connected");
  lcd.setCursor(0, 1);
  lcd.print("IP  : ");
  lcd.println(ipAdress);

  //#### mqtt ####
  lcd.setCursor(0, 2);
  lcd.print("Connect to mqtt");
  mqttc.begin(mqttBrokerIP, mqttPort, netClient);
  connectMqttBroker();

  //mqtt connected !
  lcd.setCursor(0, 2);
  lcd.print("MQTT - connected ");
  lcd.setCursor(0, 3);
  lcd.print(mqttc.connected());

  delay(1000);

  // start
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Seebier");
  lcd.setCursor(0, 1);
  lcd.print("Gaerkuehlung");
}


// ====== LOOP ========================================================================================
void loop() {
  //life toogle (blinking LED)
  lifeToggle();
  getTemperatures();
  sendTempMqttMessage();  //send temp out

  //wifi
  if (WiFi.status() != WL_CONNECTED){
    connectWifi();
    digitalWrite(LCD_POWER_PIN, LOW);
  }
  //mqtt
  mqttc.loop();
  if (!mqttc.connected()) {
    connectMqttBroker();
  }


  if (millis() > mqttMessageTimeout && millis() - lastMqttMessageReceived > mqttMessageTimeout) {
    // NO MQTT MESSAGE RECEIVED !!!
    mqttReceiveTimeout = true;
    blinkText = true;
  } else {
    mqttReceiveTimeout = false;
  }

  //relay Test
  if (millis() - lastRelayTime < 1000) {
    digitalWrite(RELAY_PIN, HIGH);
  } else {
    digitalWrite(RELAY_PIN, LOW);
  }
  //set cooler Relay
  digitalWrite(RELAY_PIN, relayCMD);

  // Diplay Values
  checkDisplayRefresh();

  //check if button is pressed
  checkEncoderButtonPress();  //check if button pressed


  //Button pressed
  if (buttonClicked == true) {
    // Serial.print("b: ");
    // Serial.println(buttonClicked);

    if (millis()-lastButtonClicked < displayOnTimeAfterClick) {
      // Change Display Mode
      displayMode = (displayMode + 1) % 3;  //3 => 0 - 2 Modes

      lcd.clear();
      refresh = true;
      lastRefreshUpdate = millis();
    }
    lastButtonClicked = millis();
  }

  //
  setLcdBcklight();

  //------------------------------------------------------------
  //change the state of blink text
  if (blinkText){
    setBlinkState();
  }

  switch (displayMode) {
    case 0:
      //Main Screen
      if (refresh) {
        lcd.setCursor(0, 0);
        lcd.print("1)Main              ");
        lcd.setCursor(0, 1);
        lcd.print("Beer:               ");
        lcd.setCursor(0, 2);
        lcd.print("Air :               ");
        lcd.setCursor(0, 3);
        lcd.print("                    ");
      }

      char buf[20];
      // Beer Temp
      sprintf(buf, "%-.2f%s(%-.2f%s)   ", beerTemp, celciusSign, targetTemp, celciusSign);
      lcd.setCursor(5, 1);
      lcd.print(buf);

      // Air Temp
      sprintf(buf, "%-.2f%s   ", airTemp, celciusSign);
      lcd.setCursor(5, 2);
      lcd.print(buf);

      //Left Time
      if (mqttReceiveTimeout) {
        lcd.setCursor(0, 3);
        if (blinkTextState) {
          lcd.print(" !!NO MQTT Message!!");
        } else {
          lcd.print("                    ");
        }
      }
      //lcd.print("tdur:");
      //lcd.print(_targetDurationStr);
      return;
    case 1:
      //Target Screen
      if (refresh) {
        lcd.setCursor(0, 0);
        lcd.print("2)Target          ");

      }

      return;
    case 2:
      //WIFI Screen
      if (refresh) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("3)Wifi          ");
        lcd.setCursor(0, 1);
        lcd.print("SSID: ");
        lcd.print(_SSID);
        lcd.setCursor(0, 2);
        lcd.print("IP  : ");
        lcd.println(ipAdress);
      }
      lcd.setCursor(0, 3);
      if (WiFi.status() != WL_CONNECTED) {
        if (blinkTextState) {
          lcd.print(" !! No Connection !!");
        } else {
          lcd.print("                    ");
        }
      } else {
        lcd.print("     WiFi connected");
      }

      return;
  }

}
// ====== LOOP  ENDED  ========================================================================================






void lifeToggle() {
  //blink LED to visually show that programm running
  int timer1 = millis() - lastLifeToggleUpdate;
  if (timer1 > 1000) {
    if (toggle == true) {
      digitalWrite(LIFE_PIN, HIGH);
      toggle = false;
    } else {
      digitalWrite(LIFE_PIN, LOW);
      toggle = true;
    }
    lastLifeToggleUpdate = millis();
  }
}

void checkDisplayRefresh() {
  //refresh whole display from time to time to avoid failures
  if (millis() - lastRefreshUpdate > 3000 || first == true) {
    first = false;
    refresh = true;
    lastRefreshUpdate = millis();
  } else {
    refresh = false;
  }
}

void setBlinkState(){
  if (millis() - lastblinkTextUpdate > 500 && blinkTextState == false) {
    //show Text
    blinkTextState = true;
  } else if (millis() - lastblinkTextUpdate > 1200 && blinkTextState == true) {
    //dont show text
    blinkTextState = false;
    lastblinkTextUpdate = millis();
  }
}

void setLcdBcklight(){
  // ========BACKLIGHT STATE MACHINE
  if (fermentationProgramMode != 2 && lcdBckLightMode == 2) {
     lcdBckLightMode = 0;
  }

  if (lcdBckLightMode == 0 && (millis()-lastButtonClicked) < displayOnTimeAfterClick){
    //when button clicked tun light on
    lcdBckLightMode = 1;
  }
  else if (lcdBckLightMode==1 && millis() - lastButtonClicked >= displayOnTimeAfterClick) {
    // backlight of when no button clicked after time x
    lcdBckLightMode = 0;
  }

  if (fermentationProgramMode == 2){
    // from mqtt >> fermantation stopped
    lcdBckLightMode = 2;
  }


  //turn backlight on and off
  // Serial.print("bckLightMode ");
  // Serial.println(lcdBckLightMode);


  // ========TURN LED ON / OFF ===============================
  if (lcdBckLightMode == 0) {
    // backlight OFF
    digitalWrite(LCD_POWER_PIN, LOW);
  }
  else if (lcdBckLightMode == 1 ||
    mqttReceiveTimeout) {
    // backlight on when button clicked
    digitalWrite(LCD_POWER_PIN, HIGH);
  }

  //BLINKING
  if (lcdBckLightMode == 2){
    //Backlght Blinking
      if (blinkDisplayLightState && millis()-lastBlinkDisplayLightUpdate > 1000){
        lastBlinkDisplayLightUpdate = millis();
        blinkDisplayLightState = false;
        digitalWrite(LCD_POWER_PIN, LOW);
      } else if (blinkDisplayLightState == false && millis()-lastBlinkDisplayLightUpdate > 300){
        lastBlinkDisplayLightUpdate = millis();
        blinkDisplayLightState = true;
        digitalWrite(LCD_POWER_PIN, HIGH);
      }

  }
}
