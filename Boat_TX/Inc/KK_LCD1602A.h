/*
Library for:				LCD1602A
Written by:					Kacper Kupiszewski & Wojciech Czechowski
Based on:					- HD44780U Datasheet
							- PCF8574 Datasheet
							- Mohamed Yaqoob's Tutorial
First update:				26/12/2020
Last update:				26/12/2020
*/

#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#ifndef KK_LCD1602A_H
#define KK_LCD1602A_H

/* General */
#define FALSE 				0x00
#define TRUE 				0x01

/* LCD Commands */
#define LCD_CLEARDISPLAY    0x01
#define LCD_RETURNHOME      0x02
#define LCD_ENTRYMODESET    0x04
#define LCD_DISPLAYCONTROL  0x08
#define LCD_CURSORSHIFT     0x10
#define LCD_FUNCTIONSET     0x20
#define LCD_SETCGRAMADDR    0x40
#define LCD_SETDDRAMADDR    0x80

/* Commands */
#define LCD_ENTRY_SH      0x01
#define LCD_ENTRY_ID      0x02
#define LCD_DISPLAY_B     0x01
#define LCD_DISPLAY_C     0x02
#define LCD_DISPLAY_D     0x04
#define LCD_SHIFT_RL      0x04
#define LCD_SHIFT_SC      0x08
#define LCD_FUNCTION_F    0x04
#define LCD_FUNCTION_N    0x08
#define LCD_FUNCTION_DL   0x10

/* I2C Control bits */
#define LCD_RS        (1 << 0)
#define LCD_RW        (1 << 1)
#define LCD_EN        (1 << 2)
#define LCD_BK_LIGHT  (1 << 3)

#define LCD_I2C_SLAVE_ADDRESS_0  0x4E
#define LCD_I2C_SLAVE_ADDRESS_1  0x7E

uint8_t lcd16x2_i2c_init(I2C_HandleTypeDef *pI2cHandle);
void lcd16x2_i2c_setCursor(uint8_t row, uint8_t col);
void lcd16x2_i2c_clear(void);
void lcd16x2_i2c_printf(const char* str, ...);

#endif
