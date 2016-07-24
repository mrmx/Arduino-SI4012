/*
   Si4012.cpp

   Copyright (c) 2016, Manuel Polo <mistermx@gmail.com>

   This file is part of Arduino-Si4012 project.

   Please read attached LICENSE file.

    Created on: 2016-07-23
        Author: Manuel Polo
*/

#include "Si4012.h"
#include <Arduino.h>
#include <Wire.h>

uint8_t Si4012::init(uint8_t i2cAddr) {
  i2cAddress = i2cAddr;
#ifdef SI4012_DEBUG
  Serial.begin(9600); while (!Serial);
  Serial.print("Si4012 init addr: 0x");
  Serial.println(i2cAddress, HEX);
#endif
  getInterruptState();//get and clear ints
  enableInterrupts();
  getState();
  //getProductDeviceInfo();
  clearTXfifo();
  setConfig();
  return 0;
}

uint8_t Si4012::setFrequency(uint32_t frequency) {
  reqBuf[1] = (frequency >> 24) & 0xFF;
  reqBuf[2] = (frequency >> 16) & 0xFF;
  reqBuf[3] = (frequency >> 8) & 0xFF;
  reqBuf[4] = frequency  & 0xFF;
  return setProperty(PROP_TX_FREQ);
}

uint32_t Si4012::getFrequency() {
  uint32_t frequency = 0;
  if (getProperty(PROP_TX_FREQ) == CMD_OK) {
    frequency |= ((uint32_t)resBuf[0] << 24 ) &  0xFF000000;
    frequency |= ((uint32_t)resBuf[1] << 16 ) &  0xFF0000;
    frequency |= ((uint32_t)resBuf[2] << 8 ) &  0xFF00;
    frequency |= resBuf[3] & 0xFF;
  }
  return frequency;
}

uint8_t Si4012::setBitrate(uint16_t rate, uint8_t rampRate) {
  reqBuf[1] = (rate >> 10) & 0xFF;
  reqBuf[2] = rate & 0xFF;
  reqBuf[3] = rampRate & 0xF;
  return setProperty(PROP_BITRATE_CONFIG);
}

uint8_t Si4012::setModulation(bool fsk, uint8_t fskDeviation) {
  reqBuf[1] = fsk ;
  reqBuf[2] = fskDeviation;
  return setProperty(PROP_MODULATION_FSKDEV);
}

uint8_t Si4012::setConfig(bool useXO, bool lsbFirst, bool fskDevPola) {
  reqBuf[1] = (useXO << 3) | (lsbFirst << 2) | fskDevPola ;
  return setProperty(PROP_CHIP_CONFIG);
}

void Si4012::setLed(uint8_t level) {
  reqBuf[1] = level & 0x3;
  setProperty(PROP_LED_INTENSITY);
  reqBuf[0] = level & 0x1;
  sendRequest(LED_CTRL, 1);
  readResponse();
}

uint8_t Si4012::setData(uint8_t * data, uint8_t packetSize) {
  clearTXfifo();
  sendRequest(SET_FIFO, data, packetSize);
  if (readResponse() == RESPONSE_STATUS_OK) {
    this->packetSize = packetSize;
    return CMD_OK;
  }
  return CMD_ERROR;
}

uint8_t Si4012::tx(bool autoTx, uint8_t nextState, uint8_t iddleMode, uint8_t dataMode) {
  reqBuf[0] = (packetSize >> 8) & 0xFF;
  reqBuf[1] = packetSize & 0xFF;
  reqBuf[2] = (autoTx << 3) | (nextState & 0x03);
  reqBuf[3] = iddleMode & 0x07;
  reqBuf[4] = dataMode & 0x03;
  sendRequest(TX_START, 5);
  if (readResponse(1) == RESPONSE_STATUS_OK) {
    return CMD_OK;
  }
  return CMD_ERROR;
}

uint8_t Si4012::setFifoThreshold(uint8_t almostEmpty, uint8_t almostFull, uint8_t autoTx) {
  reqBuf[1] = almostFull;
  reqBuf[2] = almostEmpty;
  reqBuf[3] = autoTx;
  return setProperty(PROP_FIFO_THRESHOLD);
}

uint8_t Si4012::clearTXfifo() {
  sendRequest(INIT_FIFO);
  if (readResponse() == RESPONSE_STATUS_OK) {
    return CMD_OK;
  }
  return CMD_ERROR;
}

uint8_t Si4012::getProductDeviceInfo() {
  sendRequest(GET_REV);
  if (readResponse(10) == RESPONSE_STATUS_OK) {
    return CMD_OK;
  }
  return CMD_ERROR;
}

uint8_t Si4012::getState() {
  sendRequest(GET_STATE);
  if (readResponse(5) == RESPONSE_STATUS_OK) {
    return CMD_OK;
  }
  return CMD_ERROR;
}

uint8_t Si4012::getInterruptState() {
  sendRequest(GET_INT_STATUS);
  if (readResponse(1) == RESPONSE_STATUS_OK) {
    return resBuf[0];
  }
  return CMD_ERROR;
}

uint8_t Si4012::enableInterrupts(uint8_t flags) {
  reqBuf[0] = flags;
  sendRequest(SET_INT, 1);
  if (readResponse() == RESPONSE_STATUS_OK) {
    return CMD_OK;
  }
  return CMD_ERROR;
}

uint8_t Si4012::setProperty(uint8_t propertyId) {
  reqBuf[0] = propertyId;
  sendRequest(SET_PROPERTY, 1 + (propertyId >> 4));
  if (readResponse() == RESPONSE_STATUS_OK) {
    return CMD_OK;
  }
  return CMD_ERROR;
}

uint8_t Si4012::getProperty(uint8_t propertyId) {
  reqBuf[0] = propertyId;
  sendRequest(GET_PROPERTY, 1);
  if (readResponse((propertyId >> 4)) == RESPONSE_STATUS_OK) {
    return CMD_OK;
  }
  return CMD_ERROR;
}

uint8_t Si4012::sendRequest(uint8_t cmd, uint8_t reqLen) {
  return sendRequest(cmd, reqBuf, reqLen);
}

uint8_t Si4012::sendRequest(uint8_t cmd, uint8_t * reqData, uint8_t reqLen) {
#ifdef SI4012_DEBUG
  Serial.print("sendRequest 0x"); Serial.print(cmd, HEX); Serial.print(" len:"); Serial.println(reqLen, DEC);
#endif
  Wire.beginTransmission(i2cAddress);
  Wire.write(cmd);
  for (int i = 0 ; i < reqLen ; i++) {
    Wire.write(reqData[i]);
#ifdef SI4012_DEBUG
    Serial.print("Data " + String(i + 1) + " req: 0x"); Serial.println(reqData[i] , HEX);
#endif
  }
  return Wire.endTransmission();
}

uint8_t Si4012::readResponse(uint8_t resLen) {
  Wire.requestFrom(i2cAddress, (uint8_t)(resLen + 1));
  uint8_t status = Wire.read();
#ifdef SI4012_DEBUG
  Serial.print("Status response:"); Serial.println(status, HEX);
#endif
  for (int i = 0 ; i < resLen ; i++) {
    resBuf[i] = Wire.read();
#ifdef SI4012_DEBUG
    Serial.print("Data " + String(i + 1) + " res: 0x"); Serial.println(resBuf[i] , HEX);
#endif
  }
  return status;
}
