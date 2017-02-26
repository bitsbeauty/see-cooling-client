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
  /*Serial.print("topic="); Serial.println(topic);
  Serial.print("payload="); Serial.println(payload);
  Serial.print("bytes="); Serial.println(bytes);
  Serial.print("length="); Serial.println(length);
  */

  if (topic == MQTT_TOPIC_SUBSCRIBTION){
   // parseBuffer(payload);
  }
  
  if (topic == MQTT_TOPIC_RELAYTEST){
    lastRelayTime = millis();;
  }
  lcd.setCursor(0, 2);
  lcd.print(topic);
}

void parseBuffer(String payload) {
  /*

     {
    "targetTemp" : "5.50",
    "targetDuration" : "00D, 00:31:38",
    "relay" : "0",
    "runtime" : "00D, 00:28:21"
    }

  */
  Serial.println("PARSING");

  StaticJsonBuffer<200> jsonBuffer;
  String json = payload;
  JsonObject& root = jsonBuffer.parseObject(json);

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
