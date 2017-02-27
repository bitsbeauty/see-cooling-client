void connectWifi() {
  //connect to wifi
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

  if (topic == MQTT_TOPIC_SUBSCRIBTION) {
    parseBuffer(payload);
    lastRelayTime = millis();
  }

  if (topic == MQTT_TOPIC_RELAYTEST) {
    lastRelayTime = millis();
  }

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
  //Serial.println("PARSING");

  StaticJsonBuffer<200> jsonBuffer;
  String json = payload;
  JsonObject& root = jsonBuffer.parseObject(json);

  targetTemp = root["targetTemp"];
  //targetDurationStr = root["targetDurationStr"].asString();
  //_targetDurationStr.toCharArray(targetDurationStr, 13);
  //targetTemp = _targetTemp;
  /*
    lcd.setCursor(0, 2);
    lcd.print("target: ");
    lcd.print(targetTemp);
    lcd.print("");
  */

  const char* _targetDurationStr = root["targetDurationStr"];
  //targetDurationStr = root["targetDurationStr"].asString();
  //_targetDurationStr.toCharArray(targetDurationStr, 13);
  lcd.setCursor(0, 3);
    lcd.print("tdur:");
    lcd.print(_targetDurationStr);

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
