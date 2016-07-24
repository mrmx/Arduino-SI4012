/*
   Si4022.h

   Copyright (c) 2015, Manuel Polo <mistermx@gmail.com>

   This file is part of Arduino-Si4012 project.

   Please read attached LICENSE file.

    Created on: 2016-07-23
        Author: Manuel Polo
*/

#include <Arduino.h>

#ifndef SI4012_H_
#define SI4012_H_

/**
   Uncomment following flag to enable serial debug output messages.
*/
//#define SI4012_DEBUG


#define SI4012_DEFAULT_ADDR 0x70
#define DEFAULT_FSK_DEV 63
#define DEFAULT_FIFO_ALMOST_FULL_THR 0xF0
#define DEFAULT_FIFO_ALMOST_EMPTY_THR 0x10
#define DEFAULT_FIFO_AUTO_TX_THR 0x20

#define RESPONSE_STATUS_OK 0x80
#define CMD_OK 0
#define CMD_ERROR 0xFF
/*******************************************************************************
    Command opcodes
 *******************************************************************************/
/*    Command     Opcode    Description */

#define GET_REV     0x10    /* Device revision information */
#define SET_PROPERTY  0x11    /* Sets device properties */
#define GET_PROPERTY  0x12    /* Gets device properties */
#define LED_CTRL    0x13    /* LED Control */
#define CHANGE_STATE  0x60    /* Configures device mode */
#define GET_STATE   0x61    /* Get device mode */
#define TX_START    0x62    /* Start data transmission */
#define SET_INT     0x63    /* Enable interrupts */
#define GET_INT_STATUS  0x64    /* Read & clear interrupts */
#define INIT_FIFO   0x65    /* Clears Tx FIFO */
#define SET_FIFO    0x66    /* Stores data in FIFO for Tx */
#define TX_STOP     0x67    /* Stops transmission */
#define GET_BAT_STATUS  0x68    /* Gets battery status//alpha and beta steps */

/*******************************************************************************
    Property codes
 *******************************************************************************/
/*    Property     ID    Description */
#define PROP_CHIP_CONFIG  0x10
#define PROP_LED_INTENSITY  0x11
#define PROP_MODULATION_FSKDEV  0x20
#define PROP_FIFO_THRESHOLD  0x30
#define PROP_BITRATE_CONFIG 0x31
#define PROP_TX_FREQ  0x40
#define PROP_PA_CONFIG  0x60

/********************************************************************************
   Command arguments and response data
 ********************************************************************************/
/* COMMAND: LED_CTRL */
#define LED_OFF      0x00
#define LED_LOW      0x01
#define LED_MED      0x02
#define LED_ON       0x03

/* COMMAND: SET_INT */
#define ENFFUNDER   0x80    /* Enable FIFO Underflow */
#define ENTXFFAFULL   0x40    /* Enable TX FIFO Almost Full */
#define ENTXFFAEM   0x20    /* Enable TX FIFO Almost Empty */
#define ENFFOVER    0x10    /* Enable FIFO Overflow */
#define ENPKSENT    0x08    /* Enable Packet Sent */
#define ENLBD     0x04    /* Enable Low Battery Detect */
#define ENTUNE      0x02    /* Enable Tune Complete */

/* COMMAND: TX_START */
#define AUTOTX      0x04
#define IDLESTATE   0x00
#define SHDNSTATE   0x01
#define STBYMODE    0x00
#define SENSORMODE    0x01
#define TUNEMODE    0x02
#define FIFOMODE    0x00
#define CWMODE      0x01
#define PN9_0MODE   0x02
#define PN9_1MODE   0x03


class Si4012 {

  public:
    uint8_t init(uint8_t i2Addr = SI4012_DEFAULT_ADDR);
    void setLed(uint8_t level = LED_ON);
    uint8_t setConfig(bool useXO = false, bool lsbFirst = false, bool fskDevPola = false);
    uint8_t setModulation(bool fsk, uint8_t fskDeviation = DEFAULT_FSK_DEV);
    uint8_t setFrequency(uint32_t frequency);
    uint32_t getFrequency();
    uint8_t setBitrate(uint16_t rate, uint8_t rampRate = 2);
    uint8_t setData(uint8_t * data, uint8_t packetSize);
    uint8_t tx(bool autoTx = false, uint8_t nextState = IDLESTATE, uint8_t iddleMode = STBYMODE, uint8_t dataMode = FIFOMODE);
    uint8_t setFifoThreshold(uint8_t almostEmpty = DEFAULT_FIFO_ALMOST_EMPTY_THR, uint8_t almostFull = DEFAULT_FIFO_ALMOST_FULL_THR, uint8_t autoTx = DEFAULT_FIFO_AUTO_TX_THR);


    uint8_t getState();
    uint8_t getInterruptState();

  private:
    uint8_t i2cAddress;
    uint16_t packetSize;

    /* REQ/RES buffers */
    uint8_t reqBuf[7];
    uint8_t resBuf[10];

    uint8_t clearTXfifo();
    uint8_t getProductDeviceInfo();

    uint8_t enableInterrupts(uint8_t flags = ENPKSENT);


    uint8_t setProperty(uint8_t propertyId);
    uint8_t getProperty(uint8_t propertyId);

    uint8_t sendRequest(uint8_t cmd, uint8_t reqLen = 0);
    uint8_t sendRequest(uint8_t cmd, uint8_t * reqData, uint8_t reqLen);
    uint8_t readResponse(uint8_t resLen = 0);
};

#endif /* SI4012_H_ */
