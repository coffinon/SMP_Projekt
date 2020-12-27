/*
Library for:				NRF24L01 - Polling Mode
Written by:					Kacper Kupiszewski & Wojciech Czechowski
Based on:					- NRF24L01 & NRF24L01+ Datasheet
							- Arduino NRF24L01 Tutorial
							- Mohamed Yaqoob's STM32 Tutorials
First update:				23/12/2020
Last update:				27/12/2020
*/

/* INCLUDES */

#include "KK_NRF24.h"


/* VARIABLES */
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define _BOOL(x)  (((x) >  0 ) ?  1  :  0 )

static uint64_t pipe0_reading_address;
static uint8_t payload_size;

/* NRF24L01 PINS AND HANDLES */

static GPIO_TypeDef			*nrf24_PORT;
static uint16_t				nrf24_CSN_PIN;
static uint16_t				nrf24_CE_PIN;

static SPI_HandleTypeDef 	*nrf24_hspi;
static UART_HandleTypeDef 	*nrf24_huart;

/* Private Static Function Prototypes */

static void NRF24_ACTIVATE_cmd(void);
static void NRF24_resetStatus(void);
static void NRF24_powerDown(void);

static void NRF24_flush_TX(void);
static void NRF24_flush_RX(void);


/*########################### CSN / CE OPERATIONS ###########################################*/

// CSN
static void NRF24_CSN(uint8_t state)
{
	if (state)
		HAL_GPIO_WritePin(nrf24_PORT, nrf24_CSN_PIN, GPIO_PIN_SET);
	else
		HAL_GPIO_WritePin(nrf24_PORT, nrf24_CSN_PIN, GPIO_PIN_RESET);
}

// CE
static void NRF24_CE(uint8_t state)
{
	if (state)
		HAL_GPIO_WritePin(nrf24_PORT, nrf24_CE_PIN, GPIO_PIN_SET);
	else
		HAL_GPIO_WritePin(nrf24_PORT, nrf24_CE_PIN, GPIO_PIN_RESET);
}

/*##################### BASIC READ / WRITE REGISTER OPERATIONS ##############################*/

// Read Single Byte From Register
static uint8_t NRF24_read_register(uint8_t reg)
{
	uint8_t SPI_Buf[3];

	NRF24_CSN(LOW);

	//Transmit register address
	SPI_Buf[0] = reg & CMD_REGISTER_MASK;
	HAL_SPI_Transmit(nrf24_hspi, SPI_Buf, 1, 100);

	//Receive data
	HAL_SPI_Receive(nrf24_hspi, &SPI_Buf[1], 1, 100);

	NRF24_CSN(HIGH);

	return SPI_Buf[1];
}

// Read Multiple Bytes From Register
static void NRF24_read_registerN(uint8_t reg, uint8_t *buf, uint8_t len)
{
	uint8_t SPI_Buf[3];

	NRF24_CSN(LOW);

	//Transmit register address
	SPI_Buf[0] = reg & CMD_REGISTER_MASK;
	HAL_SPI_Transmit(nrf24_hspi, SPI_Buf, 1, 100);

	//Receive data
	HAL_SPI_Receive(nrf24_hspi, buf, len, 100);

	NRF24_CSN(HIGH);
}

// Write Single Byte To Register
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

// Write Multiple Bytes To Register
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


/*########################### CUSTOM SETTINGS AND COMMANDS ##################################*/

// Activate CMD
static void NRF24_ACTIVATE_cmd(void)
{
	uint8_t cmdRxBuf[2];

	NRF24_CSN(LOW);

	//Read data from Rx payload buffer
	cmdRxBuf[0] = CMD_ACTIVATE;
	cmdRxBuf[1] = 0x73;
	HAL_SPI_Transmit(nrf24_hspi, cmdRxBuf, 2, 100);

	NRF24_CSN(HIGH);
}

// Reset Status
static void NRF24_resetStatus(void)
{
	NRF24_write_register(REG_STATUS, NRF24_read_register(REG_CONFIG) & MASK_REG_CONFIG_RESET_STATUS);
}

// Power Down
static void NRF24_powerDown(void)
{
	NRF24_write_register(REG_CONFIG, NRF24_read_register(REG_CONFIG) & ~MASK_REG_CONFIG_POWER);
}


/*############################### PIPE OPERATIONS ###########################################*/

// Check For Available Data To Read
uint8_t NRF24_available(void)
{
	uint8_t status = NRF24_read_register(REG_STATUS);

	uint8_t result = (status & MASK_REG_CONFIG_RX_DX);

	if (result)
	{
		// Clear the status bit
		NRF24_write_register(REG_STATUS, MASK_REG_CONFIG_RX_DX );

		// Handle ack payload receipt
		if (status & MASK_REG_CONFIG_TX_DS)
		{
			NRF24_write_register(REG_STATUS, MASK_REG_CONFIG_TX_DS);
		}
	}
	return result;
}

// Write Data
uint8_t NRF24_write( const void* buf, uint8_t len )
{
	// Data monitor
	uint8_t retStatus;
	uint8_t observe_tx;
	uint8_t status;
	const uint32_t timeout = 10;

	NRF24_resetStatus();

	// Transmitter power-up
	NRF24_CE(LOW);

	NRF24_write_register(REG_CONFIG, (NRF24_read_register(REG_CONFIG) | MASK_REG_CONFIG_POWER) & ~MASK_REG_CONFIG_PRIM_RX);

	NRF24_CE(HIGH);

	HAL_Delay(1);

	// Send the payload
	uint8_t wrPayloadCmd;

	NRF24_CSN(LOW);

	//Send Write Tx payload command followed by pbuf data
	wrPayloadCmd = CMD_W_TX_PAYLOAD;
	HAL_SPI_Transmit(nrf24_hspi, &wrPayloadCmd, 1, 100);
	HAL_SPI_Transmit(nrf24_hspi, (uint8_t *)buf, len, 100);

	NRF24_CSN(HIGH);

	// Enable Tx for 1ms
	NRF24_CE(HIGH);

	HAL_Delay(1);

	NRF24_CE(LOW);

	uint32_t sent_at = HAL_GetTick();
	do
	{
		NRF24_read_registerN(REG_OBSERVE_TX, &observe_tx, 1);

		//Get status register
		status = NRF24_read_register(REG_STATUS);
	}
	while(!(status & MASK_REG_CONFIG_TX_DS_MAX_RT) && ((HAL_GetTick() - sent_at) < timeout ));

	retStatus = NRF24_read_register(REG_STATUS) & MASK_REG_CONFIG_TX_DS;

	//Power down
	NRF24_available();
	NRF24_flush_TX();

	return retStatus;
}

// Read Data
uint8_t NRF24_read( void* buf, uint8_t len )
{
	uint8_t cmdRxBuf;
	uint8_t data_len = MIN(len, payload_size);

	NRF24_CSN(LOW);

	//Read data from Rx payload buffer
	cmdRxBuf = CMD_R_RX_PAYLOAD;
	HAL_SPI_Transmit(nrf24_hspi, &cmdRxBuf, 1, 100);
	HAL_SPI_Receive(nrf24_hspi, buf, data_len, 100);

	NRF24_CSN(HIGH);

	uint8_t rxStatus = NRF24_read_register(REG_FIFO_STATUS) & MASK_REG_FIFO_STATUS_RX_FIFO_EMPTY_FLAG;

	NRF24_flush_RX();
	return rxStatus;
}

// Open TX Pipe
void NRF24_openWritingPipe(uint64_t address)
{
	NRF24_write_registerN(REG_RX_ADDR_P0, (uint8_t *)(&address), 5);
	NRF24_write_registerN(REG_TX_ADDR, (uint8_t *)(&address), 5);

	NRF24_write_register(REG_RX_PW_P0, payload_size);
}

// Open RX Pipe
void NRF24_openReadingPipe(uint8_t number, uint64_t address)
{
	if (number == 0)
		pipe0_reading_address = address;

	if(number <= 6)
	{
		if(number < 2)
		{
			//Address width is 5 bytes
			NRF24_write_registerN(NRF24_ADDR_REGS[number], (uint8_t *)(&address), 5);
		}
		else
		{
			NRF24_write_registerN(NRF24_ADDR_REGS[number], (uint8_t *)(&address), 1);
		}

		//Write payload size
		NRF24_write_register(RF24_RX_PW_PIPE[number], payload_size);

		//Enable pipe
		NRF24_write_register(REG_EN_RXADDR, NRF24_read_register(REG_EN_RXADDR) | (1 << number));
	}
}

// Start Listening On Pipes
void NRF24_startListening(void)
{
	//Power up and set to RX mode
	NRF24_write_register(REG_CONFIG, NRF24_read_register(REG_CONFIG) | MASK_REG_CONFIG_POWER_UP_RX);

	// Restore Pipe 0 Address If Exists
	if(pipe0_reading_address)
		NRF24_write_registerN(REG_RX_ADDR_P0, (uint8_t *)(&pipe0_reading_address), 5);

	//Flush buffers
	NRF24_flush_TX();
	NRF24_flush_RX();

	NRF24_CE(HIGH);
	// Wait 1 ms For The Radio To Come On
	HAL_Delay(1);
}

// Stop Listening On Pipes
void NRF24_stopListening(void)
{
	NRF24_CE(LOW);

	NRF24_flush_TX();
	NRF24_flush_RX();
}


/*########################## DEFAULT INITIALIZATION #########################################*/

// NRF24 INIT
void NRF24_init(GPIO_TypeDef *nrf24PORT, uint16_t nrfCSN_Pin, uint16_t nrfCE_Pin, SPI_HandleTypeDef *nrfSPI)
{
	// Copy SPI handle, Pins And Port Variables
	nrf24_hspi = nrfSPI;

	nrf24_PORT 		= 	nrf24PORT;
	nrf24_CSN_PIN 	= 	nrfCSN_Pin;
	nrf24_CE_PIN 	= 	nrfCE_Pin;

	// Put Pins To Idle State
	NRF24_CSN(HIGH);
	NRF24_CE(LOW);

	// Initial Delay
	HAL_Delay(5);

	// Soft Reset Registers
	NRF24_write_register(REG_CONFIG, 		MASK_REG_CONFIG_2BYTES_CRC);
	NRF24_write_register(REG_EN_AA, 		MASK_REG_EN_AA_AUTO_ACK_NO_PIPES);
	NRF24_write_register(REG_EN_RXADDR, 	MASK_REG_EN_RXADDR_PIPES_1_2_ENABLE);
	NRF24_write_register(REG_SETUP_AW, 		MASK_REG_SETUP_AW_5BYTES_ADDR_FIELD);
	NRF24_write_register(REG_SETUP_RETR, 	MASK_REG_SETUP_RETR_SET_15RETR_1250DELAY);
	NRF24_write_register(REG_RF_CH, 		MASK_REG_RF_CH_SET_CHANNEL_52);
	NRF24_write_register(REG_RF_SETUP, 		MASK_REG_RF_SETUP_POWER_0DBM_2MBPS);
	NRF24_write_register(REG_STATUS, 		MASK_REG_STATUS_CLEAR);
	NRF24_write_register(REG_OBSERVE_TX, 	MASK_REG_OBSERVE_TX_CLEAR);
	NRF24_write_register(REG_CD, 			MASK_REG_CD_CLEAR);

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

	NRF24_write_register(REG_RX_PW_P0, 		MASK_REG_RX_PW_P_PIPE_NOT_USED);
	NRF24_write_register(REG_RX_PW_P1, 		MASK_REG_RX_PW_P_PIPE_NOT_USED);
	NRF24_write_register(REG_RX_PW_P2, 		MASK_REG_RX_PW_P_PIPE_NOT_USED);
	NRF24_write_register(REG_RX_PW_P3, 		MASK_REG_RX_PW_P_PIPE_NOT_USED);
	NRF24_write_register(REG_RX_PW_P4, 		MASK_REG_RX_PW_P_PIPE_NOT_USED);
	NRF24_write_register(REG_RX_PW_P5, 		MASK_REG_RX_PW_P_PIPE_NOT_USED);

	NRF24_ACTIVATE_cmd();

	NRF24_write_register(REG_DYNPD, 		MASK_REG_DYNPD_DISABLE_DYNAMIC_PAYLOAD);
	NRF24_write_register(REG_FEATURE, 		MASK_REG_FEATURE_DISABLE_DYNAMIC_PAYLOAD);

	printRadioSettings();

	//Set payload size
	payload_size = MIN(PAYLOAD_SIZE, MAX_PAYLOAD_SIZE);

	//Reset status register
	NRF24_resetStatus();

	//Flush buffers
	NRF24_flush_TX();
	NRF24_flush_RX();

	NRF24_powerDown();
}

// Init UART Debug for NRF24
void nrf24_DebugUART_Init(UART_HandleTypeDef *nrf24Uart)
{
	nrf24_huart = nrf24Uart;
}


/*############################ Flush RX / TX Functions ######################################*/

// Flush TX Buffer
static void NRF24_flush_TX(void)
{
	NRF24_write_register(CMD_FLUSH_TX, CMD_FLUSH);
}

// Flush RX Buffer
static void NRF24_flush_RX(void)
{
	NRF24_write_register(CMD_FLUSH_RX, CMD_FLUSH);
}


/*############################ PRINT SETTINGS FUNCTIONS #####################################*/

// Print Radio Settings
void printRadioSettings(void)
{
	uint8_t reg8Val;
	char uartTxBuf[100];

	sprintf(uartTxBuf, "\r\n**********************************************\r\n");
	HAL_UART_Transmit(nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	// Print CRC Setting
	reg8Val = NRF24_read_register(REG_CONFIG);
	if(reg8Val & (1 << 3))
	{
		if(reg8Val & (1 << 2))
			sprintf(uartTxBuf, "CRC:\r\n		Enabled, 2 Bytes \r\n");
		else
			sprintf(uartTxBuf, "CRC:\r\n		Enabled, 1 Byte \r\n");
	}
	else
		sprintf(uartTxBuf, "CRC:\r\n		Disabled \r\n");
	HAL_UART_Transmit(nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	// Print Auto ACK Setting
	reg8Val = NRF24_read_register(REG_EN_AA);
	sprintf(uartTxBuf, "ENAA:\r\n		P0:	%d\r\n		P1:	%d\r\n		P2:	%d\r\n		P3:	%d\r\n		P4:	%d\r\n		P5:	%d\r\n",
	_BOOL(reg8Val & (1 << 0)), _BOOL(reg8Val & (1 << 1)), _BOOL(reg8Val & (1 << 2)), _BOOL(reg8Val & (1 << 3)), _BOOL(reg8Val & (1 << 4)), _BOOL(reg8Val & (1 << 5)));
	HAL_UART_Transmit(nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	// Print Enabled RX Addresses
	reg8Val = NRF24_read_register(REG_EN_RXADDR);
	sprintf(uartTxBuf, "EN_RXADDR:\r\n		P0:	%d\r\n		P1:	%d\r\n		P2:	%d\r\n		P3:	%d\r\n		P4:	%d\r\n		P5:	%d\r\n",
	_BOOL(reg8Val & (1 << 0)), _BOOL(reg8Val & (1 << 1)), _BOOL(reg8Val & (1 << 2)), _BOOL(reg8Val & (1 << 3)), _BOOL(reg8Val & (1 << 4)), _BOOL(reg8Val & (1 << 5)));
	HAL_UART_Transmit(nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	// Print Address Width
	reg8Val = NRF24_read_register(REG_SETUP_AW) & 0x03;
	reg8Val += 2;
	sprintf(uartTxBuf, "SETUP_AW:\r\n		%d bytes \r\n", reg8Val);
	HAL_UART_Transmit(nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	// Print RF Channel
	reg8Val = NRF24_read_register(REG_RF_CH);
	sprintf(uartTxBuf, "RF_CH:\r\n		%d CH \r\n", reg8Val & 0x7F);
	HAL_UART_Transmit(nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	// Print Data Rate And Power
	reg8Val = NRF24_read_register(REG_RF_SETUP);
	if(reg8Val & (1 << 3))
		sprintf(uartTxBuf, "Data Rate:\r\n		2Mbps \r\n");
	else
		sprintf(uartTxBuf, "Data Rate:\r\n		1Mbps \r\n");
	HAL_UART_Transmit(nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	reg8Val &= (3 << 1);
	reg8Val = (reg8Val >> 1);
	if(reg8Val == 0)
		sprintf(uartTxBuf, "RF_PWR:\r\n		-18dB \r\n");
	else if(reg8Val == 1)
		sprintf(uartTxBuf, "RF_PWR:\r\n		-12dB \r\n");
	else if(reg8Val == 2)
		sprintf(uartTxBuf, "RF_PWR:\r\n		-6dB \r\n");
	else if(reg8Val == 3)
		sprintf(uartTxBuf, "RF_PWR:\r\n		 0dB \r\n");
	HAL_UART_Transmit(nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	// Print RX Pipes Addresses
	uint8_t pipeAddrs[6];
	NRF24_read_registerN(REG_RX_ADDR_P0, pipeAddrs, 5);
	sprintf(uartTxBuf, "RX_Pipe0 Addrs:\r\n		%02X,%02X,%02X,%02X,%02X  \r\n", pipeAddrs[4], pipeAddrs[3], pipeAddrs[2],pipeAddrs[1],pipeAddrs[0]);
	HAL_UART_Transmit(nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	NRF24_read_registerN(REG_RX_ADDR_P1, pipeAddrs, 5);
	sprintf(uartTxBuf, "RX_Pipe1 Addrs:\r\n		%02X,%02X,%02X,%02X,%02X  \r\n", pipeAddrs[4], pipeAddrs[3], pipeAddrs[2],pipeAddrs[1],pipeAddrs[0]);
	HAL_UART_Transmit(nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	NRF24_read_registerN(REG_RX_ADDR_P2, pipeAddrs, 1);
	sprintf(uartTxBuf, "RX_Pipe2 Addrs:\r\n		xx,xx,xx,xx,%02X  \r\n", pipeAddrs[0]);
	HAL_UART_Transmit(nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	NRF24_read_registerN(REG_RX_ADDR_P3, pipeAddrs, 1);
	sprintf(uartTxBuf, "RX_Pipe3 Addrs:\r\n		xx,xx,xx,xx,%02X  \r\n", pipeAddrs[0]);
	HAL_UART_Transmit(nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	NRF24_read_registerN(REG_RX_ADDR_P4, pipeAddrs, 1);
	sprintf(uartTxBuf, "RX_Pipe4 Addrs:\r\n		xx,xx,xx,xx,%02X  \r\n", pipeAddrs[0]);
	HAL_UART_Transmit(nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	NRF24_read_registerN(REG_RX_ADDR_P5, pipeAddrs, 1);
	sprintf(uartTxBuf, "RX_Pipe5 Addrs:\r\n		xx,xx,xx,xx,%02X  \r\n", pipeAddrs[0]);
	HAL_UART_Transmit(nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	NRF24_read_registerN(REG_TX_ADDR, pipeAddrs, 5);
	sprintf(uartTxBuf, "TX Addrs:\r\n		%02X,%02X,%02X,%02X,%02X  \r\n", pipeAddrs[4], pipeAddrs[3], pipeAddrs[2],pipeAddrs[1],pipeAddrs[0]);
	HAL_UART_Transmit(nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	// Print RX Payload Width In Each Pipe
	reg8Val = NRF24_read_register(REG_RX_PW_P0);
	sprintf(uartTxBuf, "RX_PW_P0:\r\n		%d bytes \r\n", reg8Val & 0x3F);
	HAL_UART_Transmit(nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	reg8Val = NRF24_read_register(REG_RX_PW_P1);
	sprintf(uartTxBuf, "RX_PW_P1:\r\n		%d bytes \r\n", reg8Val & 0x3F);
	HAL_UART_Transmit(nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	reg8Val = NRF24_read_register(REG_RX_PW_P2);
	sprintf(uartTxBuf, "RX_PW_P2:\r\n		%d bytes \r\n", reg8Val & 0x3F);
	HAL_UART_Transmit(nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	reg8Val = NRF24_read_register(REG_RX_PW_P3);
	sprintf(uartTxBuf, "RX_PW_P3:\r\n		%d bytes \r\n", reg8Val & 0x3F);
	HAL_UART_Transmit(nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	reg8Val = NRF24_read_register(REG_RX_PW_P4);
	sprintf(uartTxBuf, "RX_PW_P4:\r\n		%d bytes \r\n", reg8Val & 0x3F);
	HAL_UART_Transmit(nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	reg8Val = NRF24_read_register(REG_RX_PW_P5);
	sprintf(uartTxBuf, "RX_PW_P5:\r\n		%d bytes \r\n", reg8Val & 0x3F);
	HAL_UART_Transmit(nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	// Print Dynamic Payload Enable For Each Pipe
	reg8Val = NRF24_read_register(REG_DYNPD);
	sprintf(uartTxBuf, "DYNPD_pipe:\r\n		P0:	%d\r\n		P1:	%d\r\n		P2:	%d\r\n		P3:	%d\r\n		P4:	%d\r\n		P5:	%d\r\n",
	_BOOL(reg8Val & (1 << 0)), _BOOL(reg8Val & (1 << 1)), _BOOL(reg8Val & (1 << 2)), _BOOL(reg8Val & (1 << 3)), _BOOL(reg8Val & (1 << 4)), _BOOL(reg8Val & (1 << 5)));
	HAL_UART_Transmit(nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	// Print If Dynamic Payload Feature Is Enabled
	reg8Val = NRF24_read_register(REG_FEATURE);
	if(reg8Val & (1 << 2)) sprintf(uartTxBuf, "EN_DPL:\r\n		Enabled \r\n");
	else sprintf(uartTxBuf, "EN_DPL:\r\n		Disabled \r\n");
	HAL_UART_Transmit(nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	// Print If ACK Payload Feature Is Enabled
	if(reg8Val & (1 << 1)) sprintf(uartTxBuf, "EN_ACK_PAY:\r\n		Enabled \r\n");
	else sprintf(uartTxBuf, "EN_ACK_PAY:\r\n		Disabled \r\n");
	HAL_UART_Transmit(nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	sprintf(uartTxBuf, "\r\n**********************************************\r\n");
	HAL_UART_Transmit(nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);
}
