/*
Library for:				NRF24L01 - Polling Mode
Written by:					Kacper Kupiszewski & Wojciech Czechowski
Based on:					- NRF24L01 & NRF24L01+ Datasheet
							- Arduino NRF24L01 Tutorial
							- Mohamed Yaqoob's Youtube Channel
First update:				23/12/2020
Last update:				26/12/2020
*/

//List of header files
#include "stm32f4xx_hal.h"   //** Change this according to your STM32 series **//
#include "nRF24L01.h"
#include <stdio.h>
#include <string.h>

#ifndef KK_NRF24_H
#define KK_NRF24_H

/* PIPE VECTORS */

// Pipe Address Registers
static const uint8_t NRF24_ADDR_REGS[7] = {
		REG_RX_ADDR_P0,
		REG_RX_ADDR_P1,
		REG_RX_ADDR_P2,
		REG_RX_ADDR_P3,
		REG_RX_ADDR_P4,
		REG_RX_ADDR_P5,
		REG_TX_ADDR
};

// RX_PW_Px Registers Addresses
static const uint8_t RF24_RX_PW_PIPE[6] = {
		REG_RX_PW_P0,
		REG_RX_PW_P1,
		REG_RX_PW_P2,
		REG_RX_PW_P3,
		REG_RX_PW_P4,
		REG_RX_PW_P5
};


/* CSN / CE OPERATIONS */
void NRF24_CSN(uint8_t state);
void NRF24_CE(uint8_t state);


/* BASIC READ / WRITE REGISTER OPERATIONS */
uint8_t NRF24_read_register(uint8_t reg);
void NRF24_read_registerN(uint8_t reg, uint8_t *buf, uint8_t len);
void NRF24_write_register(uint8_t reg, uint8_t value);
void NRF24_write_registerN(uint8_t reg, const uint8_t* buf, uint8_t len);


/* CUSTOM SETTINGS */
void NRF24_ACTIVATE_cmd(void);
void NRF24_resetStatus(void);
void NRF24_setPayloadSize(uint8_t size);
uint8_t NRF24_getPayloadSize(void);
void NRF24_powerDown(void);

/* PIPE OPERATIONS */
uint8_t NRF24_available(void);
uint8_t NRF24_write( const void* buf, uint8_t len );
uint8_t NRF24_read( void* buf, uint8_t len );
void NRF24_openWritingPipe(uint64_t address);
void NRF24_openReadingPipe(uint8_t number, uint64_t address);
void NRF24_startListening(void);
void NRF24_stopListening(void);


/* DEFAULT INITIALIZATION */
void NRF24_begin(GPIO_TypeDef *nrf24PORT, uint16_t nrfCSN_Pin, uint16_t nrfCE_Pin, SPI_HandleTypeDef nrfSPI);
void nrf24_DebugUART_Init(UART_HandleTypeDef nrf24Uart);


/* Flush RX / TX Functions */
void NRF24_flush_TX(void);
void NRF24_flush_RX(void);


/* PRINT SETTINGS FUNCTIONS */
void printRadioSettings(void);

#endif
