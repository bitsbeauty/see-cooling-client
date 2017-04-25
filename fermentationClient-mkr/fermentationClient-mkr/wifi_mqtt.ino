#include <Arduino.h>

void connectWifi() {
  //connect to wifi
  digitalWrite(LCD_POWER_PIN, HIGH);

  lcd.setCursor(0, 0);
  lcd.print("Connect to wifi");
  lcd.setCursor(0, 1);

  int cpos = 0;
  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(_SSID, _PASSWORD);

    lcd.print(".");
    if (cpos < 4) {
      cpos++;
    } else {
      lcd.setCursor(0, 1);
      lcd.print("    ");
      lcd.setCursor(0, 1);
    }
    delay(300);
  }
  ipAdress = WiFi.localIP();

}


void connectMqttBroker() {
  lcd.setCursor(0, 3);

  int cpos = 0;
  while (!mqttc.connect(mqttClientName)) {
    lcd.print(".");
    if (cpos < 4) {
      cpos++;
    } else {
      lcd.setCursor(0, 3);
      lcd.print("    ");
      lcd.setCursor(0, 3);
    }
    delay(500);
  }

  mqttc.subscribe(MQTT_TOPIC_SUBSCRIBTION);
  mqttc.subscribe(MQTT_TOPIC_RELAYTEST);
}


void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  lastMqttMessageReceived = millis();
  //lcd.setCursor(0, 0);
  //lcd.print(topic);
  Serial.println(topic);

  if (topic == MQTT_TOPIC_SUBSCRIBTION) {
    parseBuffer(payload);
  }

  if (topic == MQTT_TOPIC_RELAYTEST) {
    lastRelayTime = millis();
  }
  //sendmqttackn();
  mqttc.publish(MQTT_TOPIC_ACKN, topic);  //TODO
}

void parseBuffer(String payload) {
  /*

    {
    "targetTemp" : "5.00",
    "targetDurationStr" : "00D, 00:50:52",
    "relay" : "0",
    "runtimeStr" : "00D, 06:09:07"
    }

  */
  // Serial.println("PARSING");

  StaticJsonBuffer<200> jsonBuffer;
  String json = payload;
  JsonObject& root = jsonBuffer.parseObject(json);

  // Test if parsing succeeds.
  if (!root.success()) {
    lcd.setCursor(0, 3);
    lcd.print("  parseObject() failed ");
    Serial.println(" parsedObject() failed ");
    return;
  }

  if (root.containsKey("fpMode")){
    // Serial.println(" contain == fpMode ");
    fermentationProgramMode = root["fpMode"];
  }
  if (root.containsKey("relay")){
    // Serial.println(" contain == relay ");
    relayCMD = root["relay"];
  }
  if (root.containsKey("targetTemp")){
    // Serial.println(" contain == targetTemp ");
    targetTemp = root["targetTemp"];
  }

  //targetDurationStr = root["targetDurationStr"].asString();
  //_targetDurationStr.toCharArray(targetDurationStr, 13);
  //targetTemp = _targetTemp;
  /*
    lcd.setCursor(0, 2);
    lcd.print("target: ");
    lcd.print(targetTemp);
    lcd.print("");
  */

  if (root.containsKey("targetDurationStr")){
    // Serial.println(" contain = targetDurationStr ");
    const char* _targetDurationStr = root["targetDurationStr"];
    //_targetDurationStr = root["targetDurationStr"].asString();
    // _targetDurationStr.toCharArray(targetDurationStr, 13);
    if (displayMode == 1) {
      lcd.setCursor(0, 3);
      lcd.print("tdur:");
      lcd.print(_targetDurationStr);
    }
  }

  if (root.containsKey("leftRuntimeStr")){
    const char* _leftRuntimeStr = root["leftRuntimeStr"];
    if (displayMode == 0) {
      lcd.setCursor(0, 3);
      lcd.print(" -T :");
      lcd.print(_leftRuntimeStr);
    }
  }



  /*
    const char* nameparam = root["actions"][0]["name"];
    const int actionLEDRed = root["actions"][0]["parameters"]["led_red"];
    const int actionLEDYellow = root["actions"][0]["parameters"]["led_yellow"];

    Serial.print("name="); Serial.println(nameparam);
    Serial.print("led_red="); Serial.println(actionLEDRed);
    Serial.print("led_yellow="); Serial.println(actionLEDYellow);
    Serial.println();
  */
}

/////// SEND MESSAGE //////////

void sendmqttackn(){
  // acknoledge message that msg pckage is received
  mqttc.publish(MQTT_TOPIC_ACKN, "True");
}

void sendTempMqttMessage() {
  if (millis() - lastMqttSendTime > 1000) {
    char sendBuf[128];
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& dataPair = jsonBuffer.createObject();

    dataPair["beerTemp"] = beerTemp;
    dataPair["airTemp"] = airTemp;

    dataPair.printTo(sendBuf, sizeof(sendBuf));
    mqttc.publish(MQTT_TOPIC_TEMP_OUT, sendBuf);

    lastMqttSendTime = millis();
  }
}
