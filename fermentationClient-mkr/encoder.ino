void checkEncoderButtonPress() {
  int buttonState = digitalRead(ENCODER_SWITCH_PIN);
  int _buttonPressedTime = 0;

  if (buttonState == HIGH) {
    if (buttonState != lastButtonState) {
      lastEncoderButtonUpdate = millis();
    }

    //check if pressed
    _buttonPressedTime = millis() - lastEncoderButtonUpdate;
    //Serial.println("here");
    if (_buttonPressedTime > 20) {
      buttonPressed = true;

      //check if button clicked
      if (buttonClickedFirst == true) {
        //on first click
        buttonClicked = true;
        buttonClickedFirst = false;
      } else {
        //button pressed and hold
        buttonClicked = false;
      }
    }
  } else {
    //RESET
    buttonPressed = false;
    buttonClicked = false;
    buttonClickedFirst = true;
  }

  
  char buf[20];
  sprintf(buf, "%i - %i -p:%i -c:%i", buttonState, _buttonPressedTime, buttonPressed, buttonClicked);
  Serial.println(buf);
  

  lastButtonState = buttonState;
}

