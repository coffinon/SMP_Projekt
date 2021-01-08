/*
Library for:				NRF24L01 - Polling Mode - new
Written by:					Kacper Kupiszewski & Wojciech Czechowski
Based on:					- NRF24L01 & NRF24L01+ Datasheet
							- Arduino NRF24L01 Tutorial
							- Mohamed Yaqoob's STM32 Tutorials
First update:				20/12/2020
Last update:				08/01/2021
*/

#ifndef NRF24_H
#define NRF24_H

/* Headers */

#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <string.h>

/* General defines */

#define LOW						0x00
#define HIGH					0x01

#define PAYLOAD_SIZE			0x02
#define MAX_PAYLOAD_SIZE		0x20

/* SPI Commands (46 page in the datasheet) */

#define CMD_R_REGISTER    		0x00
#define CMD_W_REGISTER    		0x20
#define CMD_R_RX_PAYLOAD  		0x61
#define CMD_W_TX_PAYLOAD  		0xA0
#define CMD_FLUSH_TX      		0xE1
#define CMD_FLUSH_RX      		0xE2
#define CMD_REUSE_TX_PL   		0xE3
#define CMD_ACTIVATE      		0x50
#define CMD_R_RX_PL_WID   		0x60
#define CMD_W_ACK_PAYLOAD_P0 	0xA8
#define CMD_W_ACK_PAYLOAD_P1 	0xA9
#define CMD_W_ACK_PAYLOAD_P2 	0xAA
#define CMD_W_ACK_PAYLOAD_P3 	0xAB
#define CMD_W_ACK_PAYLOAD_P4 	0xAC
#define CMD_W_ACK_PAYLOAD_P5 	0xAD
#define CMD_TX_PAYLOAD_NO_ACK	0xB0
#define CMD_NOP           		0xFF

/* Register map (53 page in the datasheet) */

#define REG_CONFIG      		0x00
#define REG_EN_AA       		0x01
#define REG_EN_RXADDR   		0x02
#define REG_SETUP_AW    		0x03
#define REG_SETUP_RETR  		0x04
#define REG_RF_CH       		0x05
#define REG_RF_SETUP    		0x06
#define REG_STATUS      		0x07
#define REG_OBSERVE_TX  		0x08
#define REG_CD          		0x09
#define REG_RX_ADDR_P0  		0x0A
#define REG_RX_ADDR_P1  		0x0B
#define REG_RX_ADDR_P2  		0x0C
#define REG_RX_ADDR_P3  		0x0D
#define REG_RX_ADDR_P4  		0x0E
#define REG_RX_ADDR_P5  		0x0F
#define REG_TX_ADDR     		0x10
#define REG_RX_PW_P0    		0x11
#define REG_RX_PW_P1    		0x12
#define REG_RX_PW_P2    		0x13
#define REG_RX_PW_P3    		0x14
#define REG_RX_PW_P4    		0x15
#define REG_RX_PW_P5    		0x16
#define REG_FIFO_STATUS 		0x17
#define REG_DYNPD	    		0x1C
#define REG_FEATURE	    		0x1D

/* CONFIG Register map (53 page in the datasheet) */

#define CONFIG_PRIM_RX			0x00
#define CONFIG_PWR_UP			0x01
#define CONFIG_CRCO				0x02
#define CONFIG_EN_CRC			0x03
#define CONFIG_MASK_MAX_RT		0x04
#define CONFIG_MASK_TX_DS		0x05
#define CONFIG_MASK_RX_DR		0x06

/* EN_AA Register map (53 page in the datasheet) */

#define EN_AA_ENAA_P0			0x00
#define EN_AA_ENAA_P1			0x01
#define EN_AA_ENAA_P2			0x02
#define EN_AA_ENAA_P3			0x03
#define EN_AA_ENAA_P4			0x04
#define EN_AA_ENAA_P5			0x05

/* EN_RXADDR Register map (53 page in the datasheet) */

#define EN_RXADDR_ERX_P0		0x00
#define EN_RXADDR_ERX_P1		0x01
#define EN_RXADDR_ERX_P2		0x02
#define EN_RXADDR_ERX_P3		0x03
#define EN_RXADDR_ERX_P4		0x04
#define EN_RXADDR_ERX_P5		0x05

/* SETUP_AW Register map (54 page in the datasheet) */

#define SETUP_AW_AW				0x00

/* SETUP_RETR Register map (54 page in the datasheet) */

#define SETUP_RETR_ARC			0x00
#define SETUP_RETR_ARD			0x04

/* RF_CH Register map (54 page in the datasheet) */

#define RF_CH_RF_CH				0x00

/* RF_SETUP Register map (54 page in the datasheet) */

#define RF_SETUP_LNA_HCURR		0x00
#define RF_SETUP_RF_PWR			0x01
#define RF_SETUP_RF_DR			0x03
#define RF_SETUP_PLL_LOCK		0x04

/* STATUS Register map (55 page in the datasheet) */

#define STATUS_TX_FULL			0x00
#define STATUS_RX_P_NO			0x01
#define STATUS_MAX_RT			0x04
#define STATUS_TX_DS			0x05
#define STATUS_RX_DR			0x06

/* OBSERVE_TX Register map (55 page in the datasheet) */

#define OBSERVE_TX_ARC_CNT		0x00
#define OBSERVE_TX_PLOS_CNT		0x04

/* CD Register map (55 page in the datasheet) */

#define CD_CD					0x00

/* RX_ADDR_PX Register map (55 page in the datasheet) */

#define RX_ADDR_P0				0x00
#define RX_ADDR_P1				0x00
#define RX_ADDR_P2				0x00
#define RX_ADDR_P3				0x00
#define RX_ADDR_P4				0x00
#define RX_ADDR_P5				0x00

/* TX_ADDR Register map (56 page in the datasheet) */

#define TX_ADDR					0x00

/* RX_PW_PX Register map (56 page in the datasheet) */

#define RX_PW_P0				0x00
#define RX_PW_P1				0x00
#define RX_PW_P2				0x00
#define RX_PW_P3				0x00
#define RX_PW_P4				0x00
#define RX_PW_P5				0x00

/* FIFO_STATUS Register map (57 page in the datasheet) */

#define FIFO_STATUS_RX_EMPTY	0x00
#define FIFO_STATUS_RX_FULL		0x01
#define FIFO_STATUS_TX_EMPTY	0x04
#define FIFO_STATUS_TX_FULL		0x05
#define FIFO_STATUS_TX_REUSE	0x06

/* DYNPD Register map (58 page in the datasheet) */

#define DYNPD_DPL_P0			0x00
#define DYNPD_DPL_P1			0x01
#define DYNPD_DPL_P2			0x02
#define DYNPD_DPL_P3			0x03
#define DYNPD_DPL_P4			0x04
#define DYNPD_DPL_P5			0x05

/* FEATURE Register map (58 page in the datasheet) */

#define FEATURE_EN_DYN_ACK		0x00
#define FEATURE_EN_ACK_PAY		0x01
#define FEATURE_EN_DPL			0x02

/* Functions */

void NRF24_init(SPI_HandleTypeDef *nrfSPI);
void NRF24_openWritingPipe(uint64_t address);
void NRF24_openReadingPipe(uint8_t number, uint64_t address);
uint8_t NRF24_write(const void* buf, uint8_t len);
uint8_t NRF24_read(void* buf, uint8_t len);
void NRF24_startListening(void);
uint8_t NRF24_available(void);

#endif
