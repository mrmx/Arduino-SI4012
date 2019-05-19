/*
   Arduino-Si4012.ino

   Copyright (c) 2016, Manuel Polo <mistermx@gmail.com>

   This file is part of Arduino-Si4012 project.

   Please read attached LICENSE file.

    Created on: 2016-07-23
        Author: Manuel Polo
*/
#include "Si4012.h"

Si4012 radio;
uint8_t packet[255];

void setup() {
  Serial.begin(9600);
  for (int i = 0 ; i < 255; i++) packet[i] = i + 1;

  radio.init();
  radio.setModulation(false);
  radio.setBitrate(1, 8); //100bps
  Serial.println("Frequency:" + String(radio.getFrequency()));
  radio.setFrequency(350123000);
}

void loop() {
  radio.setData(packet, 200);
  radio.tx();
  delay(2000);
}
