void runCooler() {
  //change sollTemp when button pressed longer
  if (millis() - buttonPressedTime > 2000) {
    lastEncoderChange = millis();
    programMode++;
    modeFirst = true;
  }

  //get Temperature
  if (tempMode == 0) {
    //send request
    requestTemp();
    tempMode++;
  } else if (tempMode == 1) {
    //wait for temperature
    if (millis() - sendTime >= 750) {
      readTemp();
      tempMode = 0;
    }
  }

  ////////// Temperature Vergelich
  unsigned long restRunTime, coolRunTime;
  lcd.setCursor(0, 3);
  lcd.print(">>Pumpe:");
  switch (coolMode) {
    case 0:
      //IDLE - check if temp is to high

      restRunTime = millis() - restStartTime;
      if (temperatureLiquid > sollTemp && restRunTime > maxRestTime) {
        coolTimeStart = millis();
        coolDown(true); //power pump
        modeFirst = true;
        coolMode++;
      } else if (temperatureLiquid <= (sollTemp - 250)) {
        coolDown(false);

      }

      if (temperatureLiquid > sollTemp) {
        lcd.setCursor(8, 3);
        lcd.print("waiting(   )");
        lcdPrintInt3(16, 3, (maxRestTime - restRunTime) / 1000);
      } else {
        lcd.setCursor(8, 3);
        lcd.print("stopped-2 cold!");
      }


      break;

    case 1:
      // IS COOLING MODE
      // check if turn out the coolDown

      if (temperatureLiquid <= (sollTemp - 250)) {
        coolDown(false);
        coolMode = 0;
      }
      coolRunTime = millis() - coolTimeStart;
      if (coolRunTime > maxCoolTime ) {
        // Error because run to long without temp change
        coolDown(false);
        restStartTime = millis();
        coolMode = 0;
      }


      lcd.setCursor(8, 3);
      char buf[20];
      sprintf(buf, "running(%d) ", (maxCoolTime - coolRunTime) / 1000);
      lcd.print(buf);
      //lcdPrintInt3(16, 3, buf);

      break;
  }

  lcd.setCursor(0, 0);
  lcd.print("Gaersteuerung");

  /////////////
  //////send over http to mysql database

  if (millis() - lastHttpSendTime > httpSendTime) {
    sendTempOverWifi();
  }

  /*
  //Headline
  char buf[20];

  // IST temp
  sprintf(buf, "IST:  %c%d.%d\337C     ", SignBit ? '-' : '+', temperatureLiquid / 100, temperatureLiquid % 100 < 10 ? 0 : temperatureLiquid % 100);
  lcd.setCursor(0, 1);
  lcd.print(buf);

  // SOLL temp
  sprintf(buf, "soll: %c%d.%d\337C     ", SignBit ? '-' : '+', sollTemp / 100, sollTemp % 100 < 10 ? 0 : sollTemp % 100);
  lcd.setCursor(0, 2);
  lcd.print(buf);


    sprintf(buf, "IST:  %c%d\337C     ", SignBit ? '-' : '+', temperature);
    lcd.setCursor(0, 2);
    lcd.print(buf);
  */
}


void coolDown(boolean _switch) {
  //
  if (_switch) {
    digitalWrite(coolingRelayPin, HIGH);
    //digitalWrite(motorPin, HIGH);

    //led
    analogWrite(ledpinR, 255);
    analogWrite(ledpinG, 100);
    analogWrite(ledpinB, 50);

  } else {
    digitalWrite(coolingRelayPin, LOW);
    //digitalWrite(motorPin, LOW);

    //led
    analogWrite(ledpinR, 50);
    analogWrite(ledpinG, 10);
    analogWrite(ledpinB, 255);
  }

}

