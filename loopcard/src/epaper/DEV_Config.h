
#ifndef _DEV_CONFIG_H_
#define _DEV_CONFIG_H_

#include <Arduino.h>
#include <stdint.h>
#include <stdio.h>

/**
 * data
**/
#define UBYTE   uint8_t
#define UWORD   uint16_t
#define UDOUBLE uint32_t

/**
 * GPIO config
**/
#define EPD_SCK_PIN  8
#define EPD_MOSI_PIN 9
#define EPD_CS_PIN   7
#define EPD_RST_PIN  5
#define EPD_DC_PIN   6
#define EPD_BUSY_PIN 19

#define GPIO_PIN_SET   1
#define GPIO_PIN_RESET 0

/**
 * GPIO read and write
**/
#define DEV_Digital_Write(_pin, _value) digitalWrite(_pin, _value == 0? LOW:HIGH)
#define DEV_Digital_Read(_pin) digitalRead(_pin)

/**
 * delay x ms
**/
#define DEV_Delay_ms(__xms) delay(__xms)

/*------------------------------------------------------------------------------------------------------*/
UBYTE DEV_Module_Init(void);
void DEV_SPI_WriteByte(UBYTE data);

#endif
