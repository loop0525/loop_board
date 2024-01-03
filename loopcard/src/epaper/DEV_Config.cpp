
#include "DEV_Config.h"

void GPIO_Config(void)
{
    pinMode(EPD_BUSY_PIN,  INPUT);
    pinMode(EPD_RST_PIN , OUTPUT);
    pinMode(EPD_DC_PIN  , OUTPUT);
    
    pinMode(EPD_SCK_PIN, OUTPUT);
    pinMode(EPD_MOSI_PIN, OUTPUT);
    pinMode(EPD_CS_PIN , OUTPUT);

    digitalWrite(EPD_CS_PIN , HIGH);
    digitalWrite(EPD_SCK_PIN, LOW);
}
/******************************************************************************
function:	Module Initialize, the BCM2835 library and initialize the pins, SPI protocol
parameter:
Info:
******************************************************************************/
UBYTE DEV_Module_Init(void)
{
	//gpio
	GPIO_Config();

	//serial printf
	Serial.begin(115200);

	// spi
	// SPI.setDataMode(SPI_MODE0);
	// SPI.setBitOrder(MSBFIRST);
	// SPI.setClockDivider(SPI_CLOCK_DIV4);
	// SPI.begin();

	return 0;
}

/******************************************************************************
function:
			SPI read and write
******************************************************************************/
void DEV_SPI_WriteByte(UBYTE data)
{
    //SPI.beginTransaction(spi_settings);
    digitalWrite(EPD_CS_PIN, GPIO_PIN_RESET);

    for (int i = 0; i < 8; i++)
    {
        if ((data & 0x80) == 0) digitalWrite(EPD_MOSI_PIN, GPIO_PIN_RESET); 
        else                    digitalWrite(EPD_MOSI_PIN, GPIO_PIN_SET);

        data <<= 1;
        digitalWrite(EPD_SCK_PIN, GPIO_PIN_SET);     
        digitalWrite(EPD_SCK_PIN, GPIO_PIN_RESET);
    }

    //SPI.transfer(data);
    digitalWrite(EPD_CS_PIN, GPIO_PIN_SET);
    //SPI.endTransaction();	
}
