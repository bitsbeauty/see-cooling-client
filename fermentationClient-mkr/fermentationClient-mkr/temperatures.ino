#include <Arduino.h>

void getTemperatures() {
  //########## ACHTUNG - DEBUG WITHOUT SENSOR!!! #########################
  // if (millis() % 1000 < 2) {
  //   beerTemp += 0.1;
  //   airTemp += 0.1;
  // }
  // ################################################

  //get Temperature
  if (tempMode == 0) {
    //send request
    requestTemp();
    lastTempRequestTime = millis();
    tempMode++;

  } else if (tempMode == 1) {
    //wait for temperature
    if (millis() - lastTempRequestTime > 750) {
      readTemp();
      tempMode = 0;
    }
  }

}

//////// REQUEST TEMP  ////////
void requestTemp() {
  for (int i = 0; i < ANZ_DS1820_SENSORS; i++) {
    // if ( tempSensorAddr[0] != 0x28) {
    //   //TODO: Sensor Failure
    //   //lcd.setCursor(0, 0);
    //   //lcd.print("GerÃ¤t ist aus der DS18B20 Familie.");
    // }

    ds.reset();
    ds.select(tempSensorAddr[i]);
    ds.write(0x44, 1);        // start conversion, with parasite power on at the end

    //delay(1000);     // maybe 750ms is enough, maybe not
  }
}


void readTemp() {
  //  NNEEEDDDSS TWO SENSORS!!

  int HighByte, LowByte, TReading, Tc_100, Whole, Fract;
  //char buf[20];
  byte data[12];
  int signBit;

  // we might do a ds.depower() here, but the reset will take care of it.

  for (int sensor=0;sensor<ANZ_DS1820_SENSORS;sensor++)
  {

  ds.reset();
  ds.select(tempSensorAddr[sensor]);
  ds.write(0xBE);         // Read Scratchpad

  for (int i = 0; i < 9; i++)
  { // we need 9 bytes
    data[i] = ds.read();
  }

  LowByte = data[0];
  HighByte = data[1];
  TReading = (HighByte << 8) + LowByte;
  signBit = TReading & 0x8000;  // test most sig bit
  if (signBit) // negative
  {
    TReading = (TReading ^ 0xffff) + 1; // 2's comp
  }
  //Tc_100 = (TReading*100/2);  //for DS18 S20 family device
  Tc_100 = (6 * TReading) + TReading / 4;    // multiply by (100 * 0.0625) or 6.25

  Whole = Tc_100 / 100;  // separate off the whole and fractional portions
  Fract = Tc_100 % 100;

  switch (sensor) {
    case 0:
      airTemp = Tc_100/100.0;
      //temperatureAir = Tc_100;
      //sprintf(buf, "%s:%c%d.%d\337C   ", "Air ", signBit ? '-' : '+', Whole, Fract < 10 ? 0 : Fract);
      break;
    case 1:
      beerTemp = Tc_100/100.0;;
      //temperatureLiquid = Tc_100;
      //sprintf(buf, "%s:%c%d.%d\337C(%d.%d)", "Beer", signBit ? '-' : '+', Whole, Fract < 10 ? 0 : Fract, sollTemp / 100, sollTemp % 100 < 10 ? 0 : sollTemp % 100);
      break;
  }


  //lcd.setCursor(0, 1 + sensor % LCD_HEIGHT);
  //lcd.print(buf);

}

}
