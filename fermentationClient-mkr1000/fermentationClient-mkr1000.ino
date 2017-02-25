
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include <WiFi101.h>

const char* _SSID     = "SEE-AP";
const char* _PASSWORD = "homebrew";

//MQTT params
char mqttCloudServer[]       = "192.168.0.16";  //192.168.0.16  - seebier.local
int  mqttCloudPort           = 1883;
char mqttCloudClientName[]   = "kuehl1";
//char mqttCloudUsername[]   = "[device-id]";
//char mqttCloudPassword[]   = "[device-token]"; 
char mqttCloudDataOut[]      = "/freezer/f1/isValues"; //Senden
char mqttCloudActionsIn[]    = "/freezer/f1/setValues"; //Empfangen

//WiFiSSLClient ipCloudStack;
WiFiClient netClient;
MQTTClient mqttCloudClient;

char buf[128];
float tempair, tempbeer;
int n = 0;

/* MQTT Send */
unsigned long lastMqttSendTime = 0;
unsigned long mqttSendTime = 1000;

void setup() {

  Serial.begin(57600);

  // Wifi Setting
  
  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(_SSID, _PASSWORD);
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  mqttCloudClient.begin(mqttCloudServer, mqttCloudPort, netClient);

  Serial.println("start MQTT connect"); Serial.println();

  while (!mqttCloudClient.connect(mqttCloudClientName)) {
    Serial.print("*");
    delay(500);
  }

  mqttCloudClient.subscribe(mqttCloudActionsIn);
  Serial.println("MQTT connected!");
}





void loop() {
  //float temperature = getTemp() * 9 / 5 + 32;

  
  mqttCloudClient.loop();
  

  //getNextSample(&temperature, &humidity);

  //Serial.print("Publishing... "); Serial.print(n); Serial.println();

  if (millis()-lastMqttSendTime > mqttSendTime){
    sendMQTTMessage();
    lastMqttSendTime = millis();
    tempbeer += 0.1;
    tempair += 0.1;
  }

}

void parseBuffer(String payload) {
  StaticJsonBuffer<200> jsonBuffer;
  String json = payload;
  JsonObject& root = jsonBuffer.parseObject(json);
  const char* nameparam = root["actions"][0]["name"];
  const int actionLEDRed = root["actions"][0]["parameters"]["led_red"];
  const int actionLEDYellow = root["actions"][0]["parameters"]["led_yellow"];


  /*
  if (actionLEDRed == 1) {
    if (savedRedValue != actionLEDRed) {
      digitalWrite(LED_RED_PIN, HIGH);
      savedRedValue = actionLEDRed;
    } 
    savedRedTime = millis();      
        
  } else {
    if (savedRedValue != actionLEDRed) {
      if (millis() - savedRedTime > RED_DELAY) {
        digitalWrite(LED_RED_PIN, LOW);
        savedRedValue = actionLEDRed;      
      }
    }
  }
  if (actionLEDYellow == 1) {
    if (savedYellowValue != actionLEDYellow) {
      digitalWrite(LED_YELLOW_PIN, HIGH);
      savedYellowValue = actionLEDYellow;
    } 
    savedYellowTime = millis();      
        
  } else {
    if (savedYellowValue != actionLEDYellow) {
      if (millis() - savedYellowTime > YELLOW_DELAY) {
        digitalWrite(LED_YELLOW_PIN, LOW);
        savedYellowValue = actionLEDYellow;      
      }
    }qm
  }
  */
}

void sendMQTTMessage() {
  loadBuffer(); // load current values into the buffer
  mqttCloudClient.publish(mqttCloudDataOut, buf);
}

void loadBuffer() {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& dataPair = jsonBuffer.createObject();

  dataPair["tempbeer"] = tempbeer;
  dataPair["tempair"] = tempair;

  dataPair.printTo(buf, sizeof(buf));
}

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  Serial.print("topic="); Serial.println(topic);
  Serial.print("payload="); Serial.println(payload);
  Serial.print("bytes="); Serial.println(bytes);
  Serial.print("length="); Serial.println(length);

  parseBuffer(payload);
}
