/*
Library for:				NRF24L01 - Polling Mode - new
Written by:					Kacper Kupiszewski & Wojciech Czechowski
Based on:					- NRF24L01 & NRF24L01+ Datasheet
							- Arduino NRF24L01 Tutorial
							- Mohamed Yaqoob's STM32 Tutorials
First update:				20/12/2020
Last update:				08/01/2021
*/

/* Includes */

#include "NRF24.h"

/* Private handles and variables */

static SPI_HandleTypeDef *nrf24_hspi;

uint8_t payload_size = PAYLOAD_SIZE;

/* Private macros */

// Data shift
#define _DS(x, n) (x << n)

/* Static function prototypes */

static void NRF24_CSN(uint8_t state);
static void NRF24_CE(uint8_t state);
static void NRF24_write_register(uint8_t reg, uint8_t value);
static void NRF24_write_registerN(uint8_t reg, const uint8_t* buf, uint8_t len);
static uint8_t NRF24_read_register(uint8_t reg);
static void NRF24_resetStatus(void);
static void NRF24_flush_TX(void);
static void NRF24_flush_RX(void);
static void NRF24_power(uint8_t state);

/* Functions */

// NRF24 Initialization function (20 and 53 page in the datasheet)
void NRF24_init(SPI_HandleTypeDef *nrfSPI)
{
	// Copy SPI handle
	nrf24_hspi = nrfSPI;

	// Put Pins To Idle State
	NRF24_CSN(HIGH);
	NRF24_CE(LOW);

	// Initial Delay
	HAL_Delay(5);

	// Soft Reset Registers
	NRF24_write_register(REG_CONFIG, 		_DS(1, CONFIG_CRCO) | _DS(1, CONFIG_EN_CRC));
	NRF24_write_register(REG_EN_AA, 		0x00);
	NRF24_write_register(REG_EN_RXADDR, 	_DS(1, EN_RXADDR_ERX_P0) | _DS(1, EN_RXADDR_ERX_P1));
	NRF24_write_register(REG_SETUP_AW, 		_DS(3, SETUP_AW_AW));
	NRF24_write_register(REG_SETUP_RETR, 	_DS(15, SETUP_RETR_ARC) | _DS(4, SETUP_RETR_ARD));
	NRF24_write_register(REG_RF_CH, 		_DS(52, RF_CH_RF_CH));
	NRF24_write_register(REG_RF_SETUP, 		_DS(1, RF_SETUP_LNA_HCURR) | _DS(3, RF_SETUP_RF_PWR));
	NRF24_write_register(REG_STATUS, 		0x00);
	NRF24_write_register(REG_OBSERVE_TX, 	0x00);
	NRF24_write_register(REG_CD, 			0x00);

	uint8_t pipeAddrVar[6];
	pipeAddrVar[4] = 0xE7;
	pipeAddrVar[3] = 0xE7;
	pipeAddrVar[2] = 0xE7;
	pipeAddrVar[1] = 0xE7;
	pipeAddrVar[0] = 0xE7;
	NRF24_write_registerN(REG_RX_ADDR_P0, pipeAddrVar, 5);

	pipeAddrVar[4] = 0xC2;
	pipeAddrVar[3] = 0xC2;
	pipeAddrVar[2] = 0xC2;
	pipeAddrVar[1] = 0xC2;
	pipeAddrVar[0] = 0xC2;
	NRF24_write_registerN(REG_RX_ADDR_P1, pipeAddrVar, 5);

	NRF24_write_register(REG_RX_ADDR_P2, 	0xC3);
	NRF24_write_register(REG_RX_ADDR_P3, 	0xC4);
	NRF24_write_register(REG_RX_ADDR_P4, 	0xC5);
	NRF24_write_register(REG_RX_ADDR_P5, 	0xC6);

	pipeAddrVar[4] = 0xE7;
	pipeAddrVar[3] = 0xE7;
	pipeAddrVar[2] = 0xE7;
	pipeAddrVar[1] = 0xE7;
	pipeAddrVar[0] = 0xE7;
	NRF24_write_registerN(REG_TX_ADDR, pipeAddrVar, 5);

	NRF24_write_register(REG_RX_PW_P0, 		0x00);
	NRF24_write_register(REG_RX_PW_P1, 		0x00);
	NRF24_write_register(REG_RX_PW_P2, 		0x00);
	NRF24_write_register(REG_RX_PW_P3, 		0x00);
	NRF24_write_register(REG_RX_PW_P4, 		0x00);
	NRF24_write_register(REG_RX_PW_P5, 		0x00);

	NRF24_write_register(REG_DYNPD, 		0x00);
	NRF24_write_register(REG_FEATURE, 		0x00);

	NRF24_resetStatus();

	NRF24_flush_TX();
	NRF24_flush_RX();

	NRF24_power(LOW);
}

// CSN Pin operations
static void NRF24_CSN(uint8_t state)
{
	if (state)
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
	else
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
}

// CE Pin operations
static void NRF24_CE(uint8_t state)
{
	if (state)
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
	else
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
}

// Write 1B to specific register (W_REGISTER command - 46 page in the datasheet)
static void NRF24_write_register(uint8_t reg, uint8_t value)
{
	uint8_t SPI_Buf[3];

	NRF24_CSN(LOW);

	//Transmit register address and data
	SPI_Buf[0] = reg | CMD_W_REGISTER;
	SPI_Buf[1] = value;
	HAL_SPI_Transmit(nrf24_hspi, SPI_Buf, 2, 100);

	NRF24_CSN(HIGH);
}

// Write >1B to specific register (W_REGISTER command - 46 page in the datasheet)
static void NRF24_write_registerN(uint8_t reg, const uint8_t* buf, uint8_t len)
{
	uint8_t SPI_Buf[3];

	NRF24_CSN(LOW);

	//Transmit register address and data
	SPI_Buf[0] = reg | CMD_W_REGISTER;
	HAL_SPI_Transmit(nrf24_hspi, SPI_Buf, 1, 100);
	HAL_SPI_Transmit(nrf24_hspi, (uint8_t*)buf, len, 100);

	NRF24_CSN(HIGH);
}

// Read 1B from specific register (R_REGISTER command - 46 page in the datasheet)
static uint8_t NRF24_read_register(uint8_t reg)
{
	uint8_t SPI_Buf[3];

	NRF24_CSN(LOW);

	//Transmit register address
	SPI_Buf[0] = reg & 0x1F;
	HAL_SPI_Transmit(nrf24_hspi, SPI_Buf, 1, 100);

	//Receive data
	HAL_SPI_Receive(nrf24_hspi, &SPI_Buf[1], 1, 100);

	NRF24_CSN(HIGH);

	return SPI_Buf[1];
}

// Reset Status (write 1 to clear - 55 page in the datasheet)
static void NRF24_resetStatus(void)
{
	NRF24_write_register(REG_STATUS, _DS(1, STATUS_MAX_RT) | _DS(1, STATUS_TX_DS) | _DS(1, STATUS_RX_DR));
}

// Flush TX Buffer (46 page in the datasheet)
static void NRF24_flush_TX(void)
{
	NRF24_write_register(CMD_FLUSH_TX, 0xFF);
}

// Flush RX Buffer (46 page in the datasheet)
static void NRF24_flush_RX(void)
{
	NRF24_write_register(CMD_FLUSH_RX, 0xFF);
}

// Power Up (PWR_UP_bit change - 53 page in the datasheet)
static void NRF24_power(uint8_t state)
{
	if(state)
		NRF24_write_register(REG_CONFIG, NRF24_read_register(REG_CONFIG) | _DS(1, CONFIG_PWR_UP));
	else
		NRF24_write_register(REG_CONFIG, NRF24_read_register(REG_CONFIG) & ~_DS(1, CONFIG_PWR_UP));
}

// Open TX Pipe (65 page in the datasheet)
void NRF24_openWritingPipe(uint64_t address)
{
	NRF24_write_registerN(REG_TX_ADDR, (uint8_t *)(&address), 5);
	NRF24_write_registerN(REG_RX_ADDR_P0, (uint8_t *)(&address), 5);

	// Set static payload size
	NRF24_write_register(REG_RX_PW_P0, payload_size);
}

// Open RX Pipe (65 and 66 page in the datasheet)
void NRF24_openReadingPipe(uint8_t number, uint64_t address)
{
	// Data in vectors for better code quality
	const uint8_t NRF24_ADDR_PX[] = {
			REG_RX_ADDR_P0,
			REG_RX_ADDR_P1,
			REG_RX_ADDR_P2,
			REG_RX_ADDR_P3,
			REG_RX_ADDR_P4,
			REG_RX_ADDR_P5
	};
	const uint8_t NRF24_RX_PW_PX[] = {
			REG_RX_PW_P0,
			REG_RX_PW_P1,
			REG_RX_PW_P2,
			REG_RX_PW_P3,
			REG_RX_PW_P4,
			REG_RX_PW_P5
	};

	if(number < 2){
		// Write 5B address to pipe (55 page in the datasheet)
		NRF24_write_registerN(NRF24_ADDR_PX[number], (uint8_t *)(&address), 5);
	}
	else{
		// Write LSB, because only this differs from P1 address (55 page in the datasheet)
		NRF24_write_registerN(NRF24_ADDR_PX[number], (uint8_t *)(&address), 1);
	}

	// Write payload size
	NRF24_write_register(NRF24_RX_PW_PX[number], payload_size);

	// Enable pipe
	NRF24_write_register(REG_EN_RXADDR, NRF24_read_register(REG_EN_RXADDR) | _DS(1, number));
}

// Write Data - function returns 1 if data has been sent successfully (described below)
uint8_t NRF24_write(const void* buf, uint8_t len)
{
	// Reset status register (in case - when i don't reset, it sometimes crashes)
	NRF24_resetStatus();

	// Transmitter power-up and PRIM_RX_bit clear (65 page in the datasheet)
	NRF24_CE(LOW);

	NRF24_power(HIGH);
	NRF24_write_register(REG_CONFIG, NRF24_read_register(REG_CONFIG) & ~_DS(1, CONFIG_PRIM_RX));

	NRF24_CSN(LOW);

	// Send payload with proper command (46 page in the datasheet)
	uint8_t wrPayloadCmd = CMD_W_TX_PAYLOAD;
	HAL_SPI_Transmit(nrf24_hspi, &wrPayloadCmd, 1, 100);
	HAL_SPI_Transmit(nrf24_hspi, (uint8_t *)buf, len, 100);

	NRF24_CSN(HIGH);

	// Enable Tx for 1ms (HIGH pulse on CE starts transmission - 65 page in the datasheet)
	NRF24_CE(HIGH);

	HAL_Delay(1);

	NRF24_CE(LOW);

	//Power down
	NRF24_power(LOW);
	NRF24_flush_TX();

	return (uint8_t)(NRF24_read_register(REG_STATUS) & _DS(1, STATUS_TX_DS));
}

// Read Data - function returns 1 if RX FIFO is empty (described below)
uint8_t NRF24_read(void* buf, uint8_t len)
{
	uint8_t cmdRxBuf;

	NRF24_CSN(LOW);

	// Read payload with proper command (46 page in the datasheet)
	cmdRxBuf = CMD_R_RX_PAYLOAD;
	HAL_SPI_Transmit(nrf24_hspi, &cmdRxBuf, 1, 100);
	HAL_SPI_Receive(nrf24_hspi, buf, len, 100);

	NRF24_CSN(HIGH);

	NRF24_flush_RX();

	return (uint8_t)(NRF24_read_register(REG_FIFO_STATUS) & _DS(1, FIFO_STATUS_RX_EMPTY));
}

// Start Listening On Pipes
void NRF24_startListening(void)
{
	// Power up and set RX mode (65 and 66 page in the datasheet)
	NRF24_power(HIGH);
	NRF24_write_register(REG_CONFIG, NRF24_read_register(REG_CONFIG) | _DS(1, CONFIG_PRIM_RX));

	// Flush buffers
	NRF24_flush_TX();
	NRF24_flush_RX();

	NRF24_CE(HIGH);

	// Wait 1 ms for radio to come on (20 page of the datasheet)
	HAL_Delay(1);
}

// Check For Available Data To Read
uint8_t NRF24_available(void)
{
	uint8_t result = (NRF24_read_register(REG_STATUS) & _DS(1, STATUS_RX_DR));

	if (result){
		// Clear the status bit
		NRF24_write_register(REG_STATUS, _DS(1, STATUS_RX_DR));
	}
	return result;
}
