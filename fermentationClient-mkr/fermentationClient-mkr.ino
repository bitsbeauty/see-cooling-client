#include <LiquidCrystal.h>
//#include <Wire.h>
//#include <OneWire.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include <WiFi101.h>

// ====== wifi & mqtt==============================
const char* _SSID     = "SEE-AP";
const char* _PASSWORD = "homebrew";

char mqttBrokerIP[]       = "192.168.0.10";  //192.168.0.16  - seebier.local
int  mqttPort           = 1883;
char mqttClientName[]   = "freezer0";
char mqtt_topic_DataOut[]      = "/freezer/f1/isValues";     //receive on: f1 und f2
char MQTT_TOPIC_SUBSCRIBTION[] = "/freezer/f1/setValues";  //  /freezer/f1/setValues
char MQTT_TOPIC_RELAYTEST[] = "/relayTest/f1";

WiFiClient netClient;   //wifi cleint
MQTTClient mqttc;


// ====== lcd ==============================
LiquidCrystal lcd(7, 8, 9, 1, 11, 12); //mkr
#define LCD_POWER_PIN 0 //mkr
#define LCD_WIDTH 20
#define LCD_HEIGHT 4
unsigned long displayTimer = 0;
int displayOnTime = 12000;

// ====== encoder ========================
#define ENCODER_SWITCH_PIN 4 //push button switch

// ====== Freezer Relay ========================
#define RELAY_PIN 19


// ====== Debug Values 
unsigned long lastUpdateMillis = 0;
boolean toggle = false;  //steady on light
unsigned long lastRelayTime = 0;  //last time when relay Test topic received


// ====== SETUP =======================================================
void setup() {
  Serial.begin(9600);

  //#### lcd ####
  pinMode(LCD_POWER_PIN, OUTPUT);
  lcd.begin(LCD_WIDTH, LCD_HEIGHT, 1);
  digitalWrite(LCD_POWER_PIN, HIGH);

  //#### encoder ####
  pinMode(ENCODER_SWITCH_PIN, INPUT);

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
  lcd.print("IP: ");
  lcd.println(WiFi.localIP());

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

  mqttc.loop();
  //show Display

  int timer1 = millis() - lastUpdateMillis;
  if (timer1 > 1000) {
    lcd.setCursor(0, 2);
    lcd.print("                    ");
    lcd.setCursor(0, 3);
    lcd.print("                    ");

    if (toggle == true) {
      //digitalWrite(RELAY_PIN, HIGH);
      toggle = false;
    } else {
      //digitalWrite(RELAY_PIN, LOW);
      toggle = true;
    }
    lastUpdateMillis = millis();
  }

  //relay Test
  if (millis() - lastRelayTime < 1000){
    digitalWrite(RELAY_PIN, HIGH);
  } else {
    digitalWrite(RELAY_PIN, LOW);
  }


 // lcd.setCursor(0, 3);
  //lcd.print("       ");

  //digitalWrite(LCD_POWER_PIN, HIGH);
  //digitalWrite(RELAY_PIN, HIGH);
  //lcd.setCursor(0, 3);
  //lcd.print(timer1);
  //delay(300);

  /* lcd.setCursor(0, 3);
    lcd.print("OFF");
    delay(500);
  */
  //digitalWrite(RELAY_PIN, LOW);
  // digitalWrite(LCD_POWER_PIN, LOW);
  //Serial.println("OFF");
  //delay(1500);


  /*if (millis() - displayTimer < displayOnTime || millis() - lastEncoderChange < displayOnTime) {
    //lcd.display();
    digitalWrite(LCD_POWER_PIN, HIGH);
    } else {
    //lcd.noDisplay();u
    digitalWrite(LCD_POWER_PIN, LOW);
    }
  */

}
