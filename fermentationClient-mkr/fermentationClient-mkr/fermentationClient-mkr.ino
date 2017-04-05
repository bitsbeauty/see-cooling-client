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
int displayMode = 0;

// ====== encoder ========================
#define ENCODER_SWITCH_PIN 4 //push button switch
unsigned long lastEncoderButtonUpdate = 0;
int lastButtonState;
boolean buttonPressed = false;
boolean buttonClicked = false;
boolean buttonClickedFirst = true;

// ====== Freezer Relay ========================
#define RELAY_PIN 19

// ====== Screen Variables ========================
char targetDurationStr[15];   //"targetDurationStr" : "00D, 00:50:52"
float targetTemp = 0;

// ====== Temperature / DS18B20 Sensor ============
OneWire  ds(20);
byte tempSensorAddr[8];

float beerTemp = 0.0;
float airTemp = 0.0;
int tempMode = 0;
unsigned long lastTempRequestTime = 0;

// ====== Timer / Debug
#define LIFE_PIN 6
char celciusSign[] = "\337";  // \337C
unsigned long lastLifeToggleUpdate = 0;
boolean toggle = false;  //steady on light
unsigned long lastRelayTime = 0;  //last time when relay Test topic received
unsigned long  lastRefreshUpdate = 0;
boolean refresh = true;
boolean first = true;
unsigned long mqttMessageTimeout = 2000;
unsigned long lastMqttMessageReceived = 0;
unsigned long lastblinkTimerUpdate = 0;
boolean blinkeMsg = false;
unsigned long lcdLightLastUpdate = 0;
boolean lcdLightOn = true;
unsigned long lastMqttSendTime = 0;

// ====== SETUP =======================================================
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

  //#### wifi ####
  lcd.setCursor(0, 0);
  lcd.print("Connect to wifi");
  lcd.setCursor(0, 1);
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


// ====== LOOP =======================================================
void loop() {
  //life toogle (blinking LED)
  lifeToggle();
 Serial.println("*");
  //getTemperatures();
  //checkMqttSendInterval();  //send temp out

  //mqtt
  mqttc.loop();
  if (!mqttc.connected()) {
    connectMqttBroker();
  }

  if (millis() > mqttMessageTimeout && millis() - lastMqttMessageReceived > mqttMessageTimeout) {
    // NO MQTT MESSAGE RECEIVED !!!
    mqttReceiveTimeout = true;

    if (millis() - lastblinkTimerUpdate > 500 && blinkeMsg == false) {
      //show Text
      blinkeMsg = true;
    } else if (millis() - lastblinkTimerUpdate > 1200 && blinkeMsg == true) {
      //dont show text
      blinkeMsg = false;
      lastblinkTimerUpdate = millis();
    }
  } else {
    mqttReceiveTimeout = false;
  }

  //relay Test
  if (millis() - lastRelayTime < 1000) {
    digitalWrite(RELAY_PIN, HIGH);
  } else {
    digitalWrite(RELAY_PIN, LOW);
  }

  checkEncoderButtonPress();  //check if button pressed

  // Diplay Values
  checkDisplayRefresh();

  //Button pressed
  if (buttonClicked == true) {
    lcdLightLastUpdate = millis();

    if (lcdLightOn == false) {
      lcdLightOn = true;

    } else {
      // Change Display Mode
      displayMode = (displayMode + 1) % 3;  //3 => 0 - 2 Modes

      lcd.clear();
      refresh = true;
      lastRefreshUpdate = millis();
    }
  }

  if (lcdLightOn && millis() - lcdLightLastUpdate < 100) {
    digitalWrite(LCD_POWER_PIN, HIGH);
  } else if (lcdLightOn && millis() - lcdLightLastUpdate >= 15000) {
    // LCD Screen POWER DOWN MODE
    lcdLightOn = false;
  } else if (lcdLightOn == false) {
    digitalWrite(LCD_POWER_PIN, LOW);
  }

  //char buf[20];
  //sprintf(buf, "dm:%i - ref:%i", displayMode, refresh);
  //Serial.println(buf);


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
      lcd.setCursor(0, 3);
      if (mqttReceiveTimeout) {
        if (blinkeMsg) {
          lcd.print(" !!NO MQTT Message!!");
        } else {
          lcd.print("                    ");
        }
      } else {
        //lcd.print("tdur:");
        //lcd.print(_targetDurationStr);
      }
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
        if (blinkeMsg) {
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


void lifeToggle() {
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