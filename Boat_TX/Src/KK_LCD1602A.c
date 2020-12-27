/*
Library for:				LCD1602A
Written by:					Kacper Kupiszewski & Wojciech Czechowski
Based on:					- HD44780U Datasheet
							- PCF8574 Datasheet
							- Mohamed Yaqoob's STM32 Tutorials
First update:				26/12/2020
Last update:				27/12/2020
*/

#include "KK_LCD1602A.h"


/* Library variables */
static I2C_HandleTypeDef* LCD1602A_hi2c;
static uint8_t LCD_I2C_SLAVE_ADDRESS = 0;


/* Private functions */
static void LCD1602A_sendCommand(uint8_t command)
{
  const uint8_t command_0_3 = (0xF0 & (command << 4));
  const uint8_t command_4_7 = (0xF0 & command);
  uint8_t i2cData[4] =
  {
      command_4_7 | LCD_EN | LCD_BK_LIGHT,
      command_4_7 | LCD_BK_LIGHT,
      command_0_3 | LCD_EN | LCD_BK_LIGHT,
      command_0_3 | LCD_BK_LIGHT,
  };
  HAL_I2C_Master_Transmit(LCD1602A_hi2c, LCD_I2C_SLAVE_ADDRESS, i2cData, 4, 200);
}

static void LCD1602A_sendData(uint8_t data)
{
  const uint8_t data_0_3 = (0xF0 & (data << 4));
  const uint8_t data_4_7 = (0xF0 & data);
  uint8_t i2cData[4] =
  {
      data_4_7 | LCD_EN | LCD_BK_LIGHT | LCD_RS,
      data_4_7 | LCD_BK_LIGHT | LCD_RS,
      data_0_3 | LCD_EN | LCD_BK_LIGHT | LCD_RS,
      data_0_3 | LCD_BK_LIGHT | LCD_RS,
  };
  HAL_I2C_Master_Transmit(LCD1602A_hi2c, LCD_I2C_SLAVE_ADDRESS, i2cData, 4, 200);
}


// LCD Init
uint8_t LCD1602A_init(I2C_HandleTypeDef *pI2cHandle)
{
    HAL_Delay(50);

    LCD1602A_hi2c = pI2cHandle;

    // Look For Proper I2C Slave Address
    if(HAL_I2C_IsDeviceReady(LCD1602A_hi2c, LCD_I2C_SLAVE_ADDRESS_0, 5, 100) != HAL_OK)
    {
    	if(HAL_I2C_IsDeviceReady(LCD1602A_hi2c, LCD_I2C_SLAVE_ADDRESS_1, 5, 100) != HAL_OK)
        {
    		return FALSE;
        }
        else
        {
            LCD_I2C_SLAVE_ADDRESS = LCD_I2C_SLAVE_ADDRESS_1;
        }
    }
    else
    {
	    LCD_I2C_SLAVE_ADDRESS = LCD_I2C_SLAVE_ADDRESS_0;
    }

    // Initialise LCD For 4-bit Operation
    HAL_Delay(50);

    // Attentions Sequence
    LCD1602A_sendCommand(0x30);
    HAL_Delay(5);
    LCD1602A_sendCommand(0x30);
    HAL_Delay(1);
    LCD1602A_sendCommand(0x30);
    HAL_Delay(8);
    LCD1602A_sendCommand(0x20);
    HAL_Delay(8);

    LCD1602A_sendCommand(LCD_FUNCTIONSET | LCD_FUNCTION_N);
    HAL_Delay(1);
    LCD1602A_sendCommand(LCD_DISPLAYCONTROL);
    HAL_Delay(1);
    LCD1602A_sendCommand(LCD_CLEARDISPLAY);
    HAL_Delay(3);
    LCD1602A_sendCommand(0x04 | LCD_ENTRY_ID);
    HAL_Delay(1);
    LCD1602A_sendCommand(LCD_DISPLAYCONTROL | LCD_DISPLAY_D);
    HAL_Delay(3);

    LCD1602A_clear();
    return TRUE;
}

// Set Cursor
void LCD1602A_setCursor(uint8_t row, uint8_t col)
{
    uint8_t maskData;
    maskData = (col) & 0x0F;
    if(row == 0)
    {
    	maskData |= 0x80;
    	LCD1602A_sendCommand(maskData);
    }
    else
    {
    	maskData |= 0xC0;
    	LCD1602A_sendCommand(maskData);
    }
}

// LCD Clear
void LCD1602A_clear(void)
{
	LCD1602A_sendCommand(LCD_CLEARDISPLAY);
	HAL_Delay(3);
}

// Print String On LCD - not written by me - idk what it exactly does for now, but works fine
void LCD1602A_printf(const char* str, ...)
{
	char stringArray[20];
	va_list args;

	va_start(args, str);
	vsprintf(stringArray, str, args);
	va_end(args);

	for(uint8_t i = 0;  i < strlen(stringArray) && i < 16; i++)
	{
		LCD1602A_sendData((uint8_t)stringArray[i]);
	}
}
