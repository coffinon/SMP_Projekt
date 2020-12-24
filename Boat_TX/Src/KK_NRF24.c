/*
Library:					NRF24L01 / NRF24L01+
Written by:					Kacper Kupiszewski
Based on:					Mohamed Yaqoob
Date Written:				23/12/2020
*/

/* INCLUDES */

#include "KK_NRF24.h"


/* VARIABLES */
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define _BOOL(x) (((x)>0) ? 1:0)

static uint64_t pipe0_reading_address;
static uint8_t payload_size;
static uint8_t dynamic_payloads_enabled;
static uint8_t ack_payload_length;
static uint8_t ack_payload_available;

/* NRF24L01 PINS AND HANDLES */

static GPIO_TypeDef			*nrf24_PORT;
static uint16_t				nrf24_CSN_PIN;
static uint16_t				nrf24_CE_PIN;

static SPI_HandleTypeDef nrf24_hspi;
static UART_HandleTypeDef nrf24_huart;


/*###########################################################################################*/

/* CSN / CE OPERATIONS */

// CSN
void NRF24_csn(int state)
{
	if(state) HAL_GPIO_WritePin(nrf24_PORT, nrf24_CSN_PIN, GPIO_PIN_SET);
	else HAL_GPIO_WritePin(nrf24_PORT, nrf24_CSN_PIN, GPIO_PIN_RESET);
}

// CE
void NRF24_ce(int state)
{
	if(state) HAL_GPIO_WritePin(nrf24_PORT, nrf24_CE_PIN, GPIO_PIN_SET);
	else HAL_GPIO_WritePin(nrf24_PORT, nrf24_CE_PIN, GPIO_PIN_RESET);
}

/* BASIC READ / WRITE REGISTER OPERATIONS */

// Read Single Byte From Register
uint8_t NRF24_read_register(uint8_t reg)
{
	uint8_t spiBuf[3];
	uint8_t retData;
	//Put CSN low
	NRF24_csn(0);
	//Transmit register address
	spiBuf[0] = reg&0x1F;
	HAL_SPI_Transmit(&nrf24_hspi, spiBuf, 1, 100);
	//Receive data
	HAL_SPI_Receive(&nrf24_hspi, &spiBuf[1], 1, 100);
	retData = spiBuf[1];
	//Bring CSN high
	NRF24_csn(1);
	return retData;
}

// Read Multiple Bytes From Register
void NRF24_read_registerN(uint8_t reg, uint8_t *buf, uint8_t len)
{
	uint8_t spiBuf[3];
	//Put CSN low
	NRF24_csn(0);
	//Transmit register address
	spiBuf[0] = reg&0x1F;
	//spiStatus = NRF24_SPI_Write(spiBuf, 1);
	HAL_SPI_Transmit(&nrf24_hspi, spiBuf, 1, 100);
	//Receive data
	HAL_SPI_Receive(&nrf24_hspi, buf, len, 100);
	//Bring CSN high
	NRF24_csn(1);
}

// Write Single Byte To Register
void NRF24_write_register(uint8_t reg, uint8_t value)
{
	uint8_t spiBuf[3];
	//Put CSN low
	NRF24_csn(0);
	//Transmit register address and data
	spiBuf[0] = reg|0x20;
	spiBuf[1] = value;
	HAL_SPI_Transmit(&nrf24_hspi, spiBuf, 2, 100);
	//Bring CSN high
	NRF24_csn(1);
}

// Write Multiple Bytes To Register
void NRF24_write_registerN(uint8_t reg, const uint8_t* buf, uint8_t len)
{
	uint8_t spiBuf[3];
	//Put CSN low
	NRF24_csn(0);
	//Transmit register address and data
	spiBuf[0] = reg|0x20;
	HAL_SPI_Transmit(&nrf24_hspi, spiBuf, 1, 100);
	HAL_SPI_Transmit(&nrf24_hspi, (uint8_t*)buf, len, 100);
	//Bring CSN high
	NRF24_csn(1);
}

// Get Status Register Value
uint8_t NRF24_get_status(void)
{
	uint8_t statReg;
	statReg = NRF24_read_register(REG_STATUS);
	return statReg;
}


/* DEFAULT INITIALIZATION */

// NRF24 INIT
void NRF24_begin(GPIO_TypeDef *nrf24PORT, uint16_t nrfCSN_Pin, uint16_t nrfCE_Pin, SPI_HandleTypeDef nrfSPI)
{
	//Copy SPI handle variable
	memcpy(&nrf24_hspi, &nrfSPI, sizeof(nrfSPI));
	//Copy Pins and Port variables
	nrf24_PORT = nrf24PORT;
	nrf24_CSN_PIN = nrfCSN_Pin;
	nrf24_CE_PIN = nrfCE_Pin;

	//Put pins to idle state
	NRF24_csn(1);
	NRF24_ce(0);
	//5 ms initial delay
	HAL_Delay(5);

	//**** Soft Reset Registers default values ****//
	NRF24_write_register(0x00, 0x0C);
	NRF24_write_register(0x01, 0x3f);
	NRF24_write_register(0x02, 0x03);
	NRF24_write_register(0x03, 0x03);
	NRF24_write_register(0x04, 0x03);
	NRF24_write_register(0x05, 0x02);
	NRF24_write_register(0x06, 0x0F);
	NRF24_write_register(0x07, 0x0e);
	NRF24_write_register(0x08, 0x00);
	NRF24_write_register(0x09, 0x00);

	uint8_t pipeAddrVar[6];
	pipeAddrVar[4]=0xE7; pipeAddrVar[3]=0xE7; pipeAddrVar[2]=0xE7; pipeAddrVar[1]=0xE7; pipeAddrVar[0]=0xE7;
	NRF24_write_registerN(0x0A, pipeAddrVar, 5);

	pipeAddrVar[4]=0xC2; pipeAddrVar[3]=0xC2; pipeAddrVar[2]=0xC2; pipeAddrVar[1]=0xC2; pipeAddrVar[0]=0xC2;
	NRF24_write_registerN(0x0B, pipeAddrVar, 5);

	NRF24_write_register(0x0C, 0xC3);
	NRF24_write_register(0x0D, 0xC4);
	NRF24_write_register(0x0E, 0xC5);
	NRF24_write_register(0x0F, 0xC6);

	pipeAddrVar[4]=0xE7; pipeAddrVar[3]=0xE7; pipeAddrVar[2]=0xE7; pipeAddrVar[1]=0xE7; pipeAddrVar[0]=0xE7;
	NRF24_write_registerN(0x10, pipeAddrVar, 5);

	NRF24_write_register(0x11, 0);
	NRF24_write_register(0x12, 0);
	NRF24_write_register(0x13, 0);
	NRF24_write_register(0x14, 0);
	NRF24_write_register(0x15, 0);
	NRF24_write_register(0x16, 0);

	NRF24_ACTIVATE_cmd();
	NRF24_write_register(0x1c, 0);
	NRF24_write_register(0x1d, 0);

	printRadioSettings();
	//Initialise retries 15 and delay 1250 usec
	NRF24_setRetries(15, 15);
	//Disable dynamic payload
	NRF24_disableDynamicPayloads();
	//Set payload size
	NRF24_setPayloadSize(32);

	//Reset status register
	NRF24_resetStatus();
	//Initialise channel to 76
	NRF24_setChannel(76);
	//Flush buffers
	NRF24_flush_tx();
	NRF24_flush_rx();

	NRF24_powerDown();
}

// Init UART Debug for NRF24
void nrf24_DebugUART_Init(UART_HandleTypeDef nrf24Uart)
{
	memcpy(&nrf24_huart, &nrf24Uart, sizeof(nrf24Uart));
}


/* CUSTOM SETTINGS */

// Activate CMD
void NRF24_ACTIVATE_cmd(void)
{
	uint8_t cmdRxBuf[2];
	//Read data from Rx payload buffer
	NRF24_csn(0);
	cmdRxBuf[0] = CMD_ACTIVATE;
	cmdRxBuf[1] = 0x73;
	HAL_SPI_Transmit(&nrf24_hspi, cmdRxBuf, 2, 100);
	NRF24_csn(1);
}

// Set Transmission Retries And Delay
void NRF24_setRetries(uint8_t delay, uint8_t count)
{
	NRF24_write_register(REG_SETUP_RETR,(delay&0xf)<<BIT_ARD | (count&0xf)<<BIT_ARC);
}

// Disable Dynamic Payloads
void NRF24_disableDynamicPayloads(void)
{
	NRF24_write_register(REG_FEATURE,NRF24_read_register(REG_FEATURE) &  ~(_BV(BIT_EN_DPL)) );
	//Disable for all pipes
	NRF24_write_register(REG_DYNPD,0);
	dynamic_payloads_enabled = FALSE;
}

// Set Payload Size
void NRF24_setPayloadSize(uint8_t size)
{
	const uint8_t max_payload_size = 32;
  payload_size = MIN(size,max_payload_size);
}

// Reset Status
void NRF24_resetStatus(void)
{
	NRF24_write_register(REG_STATUS,_BV(BIT_RX_DR) | _BV(BIT_TX_DS) | _BV(BIT_MAX_RT) );
}

// Set RF Channel
void NRF24_setChannel(uint8_t channel)
{
	const uint8_t max_channel = 127;
  NRF24_write_register(REG_RF_CH,MIN(channel,max_channel));
}

// Power Down
void NRF24_powerDown(void)
{
	NRF24_write_register(REG_CONFIG,NRF24_read_register(REG_CONFIG) & ~_BV(BIT_PWR_UP));
}

// Set Auto ACK
void NRF24_setAutoAck(uint8_t enable)
{
	if ( enable )
    NRF24_write_register(REG_EN_AA, 0x3F);
  else
    NRF24_write_register(REG_EN_AA, 0x00);
}

// Open TX Pipe
void NRF24_openWritingPipe(uint64_t address)
{
	NRF24_write_registerN(REG_RX_ADDR_P0, (uint8_t *)(&address), 5);
  NRF24_write_registerN(REG_TX_ADDR, (uint8_t *)(&address), 5);

	const uint8_t max_payload_size = 32;
  NRF24_write_register(REG_RX_PW_P0,MIN(payload_size,max_payload_size));
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
		NRF24_write_register(RF24_RX_PW_PIPE[number],payload_size);
		//Enable pipe
		NRF24_write_register(REG_EN_RXADDR, NRF24_read_register(REG_EN_RXADDR) | _BV(number));
	}
}

// Start Listening On Pipes
void NRF24_startListening(void)
{
	//Power up and set to RX mode
	NRF24_write_register(REG_CONFIG, NRF24_read_register(REG_CONFIG) | (1UL<<1) |(1UL <<0));
	//Restore pipe 0 address if exists
	if(pipe0_reading_address)
		NRF24_write_registerN(REG_RX_ADDR_P0, (uint8_t *)(&pipe0_reading_address), 5);

	//Flush buffers
	NRF24_flush_tx();
	NRF24_flush_rx();
	//Set CE HIGH to start listenning
	NRF24_ce(1);
	//Wait for 1 ms for the radio to come on
	HAL_Delay(1);
}

// Stop Listening On Pipes
void NRF24_stopListening(void)
{
	NRF24_ce(0);
	NRF24_flush_tx();
	NRF24_flush_rx();
}

/* PIPE OPERATIONS */

// Check For Available Data To Read
uint8_t NRF24_available(void)
{
	return NRF24_availablePipe(NULL);
}

// Check If Data Are Available And On Which Pipe
uint8_t NRF24_availablePipe(uint8_t* pipe_num)
{
	uint8_t status = NRF24_get_status();

  uint8_t result = ( status & _BV(BIT_RX_DR) );

  if (result)
  {
    // If the caller wants the pipe number, include that
    if ( pipe_num )
      *pipe_num = ( status >> BIT_RX_P_NO ) & 0x7;

    // Clear the status bit
    NRF24_write_register(REG_STATUS,_BV(BIT_RX_DR) );

    // Handle ack payload receipt
    if ( status & _BV(BIT_TX_DS) )
    {
      NRF24_write_register(REG_STATUS,_BV(BIT_TX_DS));
    }
  }
  return result;
}

// Write Data
uint8_t NRF24_write( const void* buf, uint8_t len )
{
	uint8_t retStatus;
	//Start writing
	NRF24_resetStatus();
	NRF24_startWrite(buf,len);
	//Data monitor
  uint8_t observe_tx;
  uint8_t status;
  uint32_t sent_at = HAL_GetTick();
	const uint32_t timeout = 10; //ms to wait for timeout
	do
  {
    NRF24_read_registerN(REG_OBSERVE_TX,&observe_tx,1);
		//Get status register
		status = NRF24_get_status();
  }
  while( ! ( status & ( _BV(BIT_TX_DS) | _BV(BIT_MAX_RT) ) ) && ( HAL_GetTick() - sent_at < timeout ) );


	uint8_t tx_ok, tx_fail;
	NRF24_checkInterruptFlags(&tx_ok,&tx_fail, &ack_payload_available);
	retStatus = tx_ok;
	if ( ack_payload_available )
  {
    ack_payload_length = NRF24_getDynamicPayloadSize();
	}

	//Power down
	NRF24_available();
	NRF24_flush_tx();
	return retStatus;
}

// Read Data
uint8_t NRF24_read( void* buf, uint8_t len )
{
	NRF24_read_payload( buf, len );
	uint8_t rxStatus = NRF24_read_register(REG_FIFO_STATUS) & _BV(BIT_RX_EMPTY);
	NRF24_flush_rx();
	NRF24_getDynamicPayloadSize();
	return rxStatus;
}

// Write Payload To Be Transmitted
void NRF24_write_payload(const void* buf, uint8_t len)
{
	uint8_t wrPayloadCmd;
	//Bring CSN low
	NRF24_csn(0);
	//Send Write Tx payload command followed by pbuf data
	wrPayloadCmd = CMD_W_TX_PAYLOAD;
	HAL_SPI_Transmit(&nrf24_hspi, &wrPayloadCmd, 1, 100);
	HAL_SPI_Transmit(&nrf24_hspi, (uint8_t *)buf, len, 100);
	//Bring CSN high
	NRF24_csn(1);
}

// Read Payload To Be Received
void NRF24_read_payload(void* buf, uint8_t len)
{
	uint8_t cmdRxBuf;
	//Get data length using payload size
	uint8_t data_len = MIN(len, NRF24_getPayloadSize());
	//Read data from Rx payload buffer
	NRF24_csn(0);
	cmdRxBuf = CMD_R_RX_PAYLOAD;
	HAL_SPI_Transmit(&nrf24_hspi, &cmdRxBuf, 1, 100);
	HAL_SPI_Receive(&nrf24_hspi, buf, data_len, 100);
	NRF24_csn(1);
}

// Get Payload Size
uint8_t NRF24_getPayloadSize(void)
{
	return payload_size;
}

// Get Dynamic Payload Size
uint8_t NRF24_getDynamicPayloadSize(void)
{
	return NRF24_read_register(CMD_R_RX_PL_WID);
}


//40. Start write (for IRQ mode)
void NRF24_startWrite( const void* buf, uint8_t len )
{
  // Transmitter power-up
  NRF24_ce(0);
  NRF24_write_register(REG_CONFIG, ( NRF24_read_register(REG_CONFIG) | _BV(BIT_PWR_UP) ) & ~_BV(BIT_PRIM_RX) );
  NRF24_ce(1);
  HAL_Delay(1);

  // Send the payload
  NRF24_write_payload( buf, len );

  // Enable Tx for 15usec
  NRF24_ce(1);
  HAL_Delay(1);
  NRF24_ce(0);
}


// Check Interrupt Flags
void NRF24_checkInterruptFlags(uint8_t *tx_ok, uint8_t *tx_fail, uint8_t *rx_ready)
{
	uint8_t status = NRF24_get_status();
	*tx_ok = 0;
	NRF24_write_register(REG_STATUS,_BV(BIT_RX_DR) | _BV(BIT_TX_DS) | _BV(BIT_MAX_RT) );
  // Report to the user what happened
  *tx_ok = status & _BV(BIT_TX_DS);
  *tx_fail = status & _BV(BIT_MAX_RT);
  *rx_ready = status & _BV(BIT_RX_DR);
}



/* Flush RX / TX Functions */

// Flush TX Buffer
void NRF24_flush_tx(void)
{
	NRF24_write_register(CMD_FLUSH_TX, 0xFF);
}

// Flush RX Buffer
void NRF24_flush_rx(void)
{
	NRF24_write_register(CMD_FLUSH_RX, 0xFF);
}

/* PRINT SETTINGS FUNCTIONS */

// Print Radio Settings
void printRadioSettings(void)
{
	uint8_t reg8Val;
	char uartTxBuf[100];
	sprintf(uartTxBuf, "\r\n**********************************************\r\n");
	HAL_UART_Transmit(&nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);
	//a) Get CRC settings - Config Register
	reg8Val = NRF24_read_register(0x00);
	if(reg8Val & (1 << 3))
	{
		if(reg8Val & (1 << 2)) sprintf(uartTxBuf, "CRC:\r\n		Enabled, 2 Bytes \r\n");
		else sprintf(uartTxBuf, "CRC:\r\n		Enabled, 1 Byte \r\n");
	}
	else
	{
		sprintf(uartTxBuf, "CRC:\r\n		Disabled \r\n");
	}
	HAL_UART_Transmit(&nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);
	//b) AutoAck on pipes
	reg8Val = NRF24_read_register(0x01);
	sprintf(uartTxBuf, "ENAA:\r\n		P0:	%d\r\n		P1:	%d\r\n		P2:	%d\r\n		P3:	%d\r\n		P4:	%d\r\n		P5:	%d\r\n",
	_BOOL(reg8Val&(1<<0)), _BOOL(reg8Val&(1<<1)), _BOOL(reg8Val&(1<<2)), _BOOL(reg8Val&(1<<3)), _BOOL(reg8Val&(1<<4)), _BOOL(reg8Val&(1<<5)));
	HAL_UART_Transmit(&nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);
	//c) Enabled Rx addresses
	reg8Val = NRF24_read_register(0x02);
	sprintf(uartTxBuf, "EN_RXADDR:\r\n		P0:	%d\r\n		P1:	%d\r\n		P2:	%d\r\n		P3:	%d\r\n		P4:	%d\r\n		P5:	%d\r\n",
	_BOOL(reg8Val&(1<<0)), _BOOL(reg8Val&(1<<1)), _BOOL(reg8Val&(1<<2)), _BOOL(reg8Val&(1<<3)), _BOOL(reg8Val&(1<<4)), _BOOL(reg8Val&(1<<5)));
	HAL_UART_Transmit(&nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);
	//d) Address width
	reg8Val = NRF24_read_register(0x03)&0x03;
	reg8Val +=2;
	sprintf(uartTxBuf, "SETUP_AW:\r\n		%d bytes \r\n", reg8Val);
	HAL_UART_Transmit(&nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);
	//e) RF channel
	reg8Val = NRF24_read_register(0x05);
	sprintf(uartTxBuf, "RF_CH:\r\n		%d CH \r\n", reg8Val&0x7F);
	HAL_UART_Transmit(&nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);
	//f) Data rate & RF_PWR
	reg8Val = NRF24_read_register(0x06);
	if(reg8Val & (1 << 3)) sprintf(uartTxBuf, "Data Rate:\r\n		2Mbps \r\n");
	else sprintf(uartTxBuf, "Data Rate:\r\n		1Mbps \r\n");
	HAL_UART_Transmit(&nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);
	reg8Val &= (3 << 1);
	reg8Val = (reg8Val>>1);
	if(reg8Val == 0) sprintf(uartTxBuf, "RF_PWR:\r\n		-18dB \r\n");
	else if(reg8Val == 1) sprintf(uartTxBuf, "RF_PWR:\r\n		-12dB \r\n");
	else if(reg8Val == 2) sprintf(uartTxBuf, "RF_PWR:\r\n		-6dB \r\n");
	else if(reg8Val == 3) sprintf(uartTxBuf, "RF_PWR:\r\n		0dB \r\n");
	HAL_UART_Transmit(&nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);
	//g) RX pipes addresses
	uint8_t pipeAddrs[6];
	NRF24_read_registerN(0x0A, pipeAddrs, 5);
	sprintf(uartTxBuf, "RX_Pipe0 Addrs:\r\n		%02X,%02X,%02X,%02X,%02X  \r\n", pipeAddrs[4], pipeAddrs[3], pipeAddrs[2],pipeAddrs[1],pipeAddrs[0]);
	HAL_UART_Transmit(&nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	NRF24_read_registerN(0x0A+1, pipeAddrs, 5);
	sprintf(uartTxBuf, "RX_Pipe1 Addrs:\r\n		%02X,%02X,%02X,%02X,%02X  \r\n", pipeAddrs[4], pipeAddrs[3], pipeAddrs[2],pipeAddrs[1],pipeAddrs[0]);
	HAL_UART_Transmit(&nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	NRF24_read_registerN(0x0A+2, pipeAddrs, 1);
	sprintf(uartTxBuf, "RX_Pipe2 Addrs:\r\n		xx,xx,xx,xx,%02X  \r\n", pipeAddrs[0]);
	HAL_UART_Transmit(&nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	NRF24_read_registerN(0x0A+3, pipeAddrs, 1);
	sprintf(uartTxBuf, "RX_Pipe3 Addrs:\r\n		xx,xx,xx,xx,%02X  \r\n", pipeAddrs[0]);
	HAL_UART_Transmit(&nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	NRF24_read_registerN(0x0A+4, pipeAddrs, 1);
	sprintf(uartTxBuf, "RX_Pipe4 Addrs:\r\n		xx,xx,xx,xx,%02X  \r\n", pipeAddrs[0]);
	HAL_UART_Transmit(&nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	NRF24_read_registerN(0x0A+5, pipeAddrs, 1);
	sprintf(uartTxBuf, "RX_Pipe5 Addrs:\r\n		xx,xx,xx,xx,%02X  \r\n", pipeAddrs[0]);
	HAL_UART_Transmit(&nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	NRF24_read_registerN(0x0A+6, pipeAddrs, 5);
	sprintf(uartTxBuf, "TX Addrs:\r\n		%02X,%02X,%02X,%02X,%02X  \r\n", pipeAddrs[4], pipeAddrs[3], pipeAddrs[2],pipeAddrs[1],pipeAddrs[0]);
	HAL_UART_Transmit(&nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	//h) RX PW (Payload Width 0 - 32)
	reg8Val = NRF24_read_register(0x11);
	sprintf(uartTxBuf, "RX_PW_P0:\r\n		%d bytes \r\n", reg8Val&0x3F);
	HAL_UART_Transmit(&nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	reg8Val = NRF24_read_register(0x11+1);
	sprintf(uartTxBuf, "RX_PW_P1:\r\n		%d bytes \r\n", reg8Val&0x3F);
	HAL_UART_Transmit(&nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	reg8Val = NRF24_read_register(0x11+2);
	sprintf(uartTxBuf, "RX_PW_P2:\r\n		%d bytes \r\n", reg8Val&0x3F);
	HAL_UART_Transmit(&nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	reg8Val = NRF24_read_register(0x11+3);
	sprintf(uartTxBuf, "RX_PW_P3:\r\n		%d bytes \r\n", reg8Val&0x3F);
	HAL_UART_Transmit(&nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	reg8Val = NRF24_read_register(0x11+4);
	sprintf(uartTxBuf, "RX_PW_P4:\r\n		%d bytes \r\n", reg8Val&0x3F);
	HAL_UART_Transmit(&nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	reg8Val = NRF24_read_register(0x11+5);
	sprintf(uartTxBuf, "RX_PW_P5:\r\n		%d bytes \r\n", reg8Val&0x3F);
	HAL_UART_Transmit(&nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	//i) Dynamic payload enable for each pipe
	reg8Val = NRF24_read_register(0x1c);
	sprintf(uartTxBuf, "DYNPD_pipe:\r\n		P0:	%d\r\n		P1:	%d\r\n		P2:	%d\r\n		P3:	%d\r\n		P4:	%d\r\n		P5:	%d\r\n",
	_BOOL(reg8Val&(1<<0)), _BOOL(reg8Val&(1<<1)), _BOOL(reg8Val&(1<<2)), _BOOL(reg8Val&(1<<3)), _BOOL(reg8Val&(1<<4)), _BOOL(reg8Val&(1<<5)));
	HAL_UART_Transmit(&nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	//j) EN_DPL (is Dynamic payload feature enabled ?)
	reg8Val = NRF24_read_register(0x1d);
	if(reg8Val&(1<<2)) sprintf(uartTxBuf, "EN_DPL:\r\n		Enabled \r\n");
	else sprintf(uartTxBuf, "EN_DPL:\r\n		Disabled \r\n");
	HAL_UART_Transmit(&nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);

	//k) EN_ACK_PAY
	if(reg8Val&(1<<1)) sprintf(uartTxBuf, "EN_ACK_PAY:\r\n		Enabled \r\n");
	else sprintf(uartTxBuf, "EN_ACK_PAY:\r\n		Disabled \r\n");
	HAL_UART_Transmit(&nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);


	sprintf(uartTxBuf, "\r\n**********************************************\r\n");
	HAL_UART_Transmit(&nrf24_huart, (uint8_t *)uartTxBuf, strlen(uartTxBuf), 10);
}







