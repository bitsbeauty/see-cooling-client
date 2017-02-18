void setSollTemp() {
  if (modeFirst) {
    modeFirst = false;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SollTemp Aendern");

    //led
    analogWrite(ledpinR, 0);
    analogWrite(ledpinG, 20);
    analogWrite(ledpinB, 0);

  }

  if (millis() - lastEncoderChange > 6000) {
    programMode = 0;
    modeFirst = false;
  }

  char buf[20];
  if (encoderValue < lastEncoderValue || encoderValue > lastEncoderValue) {

    int counterValue = (encoderValue - lastEncoderValue);
    if (counterValue > 0) {
      sollTemp = sollTemp + 10;
    } else if (counterValue < 0){
      sollTemp = sollTemp - 10;
    }

    //sprintf(buf, "%d     ", counterValue);
    //lcd.setCursor(8, 3);
    //lcd.print(buf);

    lastEncoderValue = encoderValue;
  }

  sprintf(buf, "%d.%d  ", sollTemp / 100, sollTemp%100);
  lcd.setCursor(8, 2);
  lcd.print(buf);

  //lcd.setCursor(0, 3);
  //lcd.print(sollTemp);



  /*
    lcd.setCursor(0, 3);
    lcd.print(encoderValue);

    lcd.setCursor(10, 3);
    lcd.print(lastEncoderValue);
  */
}


void updateEncoder() {
  int MSB = digitalRead(encoderPin1); //MSB = most significant bit
  int LSB = digitalRead(encoderPin2); //LSB = least significant bit

  int encoded = (MSB << 1) | LSB; //converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue ++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue --;

  lastEncoded = encoded; //store this value for next time
  lastEncoderChange = millis();
}
