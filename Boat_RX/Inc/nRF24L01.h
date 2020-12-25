/*
Library:				NRF24L01 REGISTER MAP
Written by:				Kacper Kupiszewski
Based on:				Mohamed Yaqoob library
Date written:			23.12.2020
*/

#define _BV(x) (1<<(x))

/* General */
#define LOW					0x00
#define HIGH				0x01

#define FALSE 				0x00
#define TRUE 				0x01

/* Settings */

#define PAYLOAD_SIZE 		0x02
#define ACK_PAYLOAD_SIZE	0x02
#define MAX_PAYLOAD_SIZE 	0x20

/* Memory Map */
#define REG_CONFIG      	0x00
#define REG_EN_AA       	0x01
#define REG_EN_RXADDR   	0x02
#define REG_SETUP_AW    	0x03
#define REG_SETUP_RETR  	0x04
#define REG_RF_CH       	0x05
#define REG_RF_SETUP    	0x06
#define REG_STATUS      	0x07
#define REG_OBSERVE_TX  	0x08
#define REG_CD          	0x09
#define REG_RX_ADDR_P0  	0x0A
#define REG_RX_ADDR_P1  	0x0B
#define REG_RX_ADDR_P2  	0x0C
#define REG_RX_ADDR_P3  	0x0D
#define REG_RX_ADDR_P4  	0x0E
#define REG_RX_ADDR_P5  	0x0F
#define REG_TX_ADDR     	0x10
#define REG_RX_PW_P0    	0x11
#define REG_RX_PW_P1    	0x12
#define REG_RX_PW_P2    	0x13
#define REG_RX_PW_P3    	0x14
#define REG_RX_PW_P4    	0x15
#define REG_RX_PW_P5    	0x16
#define REG_FIFO_STATUS 	0x17
#define REG_DYNPD	    	0x1C
#define REG_FEATURE	    	0x1D

/* MEMORY MAP SETTINGS */
#define REG_CONFIG_2BYTES_CRC					0x0C
#define REG_CONFIG_RESET_STATUS					0x70
#define REG_CONFIG_POWER_DOWN					0x02
#define REG_CONFIG_POWER_UP						0x02
#define REG_CONFIG_PRIM_RX						0x01
#define REG_CONFIG_POWER_UP_RX					0x03
#define REG_CONFIG_GET_RX_DX					0x40
#define REG_CONFIG_GET_TX_DS					0x20
#define REG_CONFIG_GET_TX_DS_MAX_RT				0x30
#define REG_EN_AA_AUTO_ACK_ALL_PIPES			0x3F
#define REG_EN_AA_AUTO_ACK_NO_PIPES				0x00
#define REG_EN_RXADDR_PIPES_1_2_ENABLE			0x03
#define REG_SETUP_AW_5BYTES_ADDR_FIELD			0x03
#define REG_SETUP_RETR_SET_15RETR_1250DELAY		0x4F
#define REG_RF_CH_SET_CHANNEL_52				0x34
#define REG_RF_SETUP_POWER_0DBM_2MBPS			0x0F
#define REG_STATUS_CLEAR						0x0E
#define REG_OBSERVE_TX_CLEAR					0x00
#define REG_CD_CLEAR							0x00
#define REG_RX_PW_P_PIPE_NOT_USED				0x00
#define REG_FIFO_STATUS_GET_RX_FIFO_EMPTY_FLAG	0x01
#define REG_DYNPD_DISABLE_DYNAMIC_PAYLOAD		0x00
#define REG_FEATURE_DISABLE_DYNAMIC_PAYLOAD		0x00

/* Bit Mnemonics */
#define MASK_RX_DR  		0x06
#define MASK_TX_DS  		0x05
#define MASK_MAX_RT 		0x04
#define BIT_EN_CRC      	0x03
#define BIT_CRCO        	0x02
#define BIT_PWR_UP      	0x01
#define BIT_PRIM_RX     	0x00
#define BIT_ENAA_P5     	0x05
#define BIT_ENAA_P4     	0x04
#define BIT_ENAA_P3     	0x03
#define BIT_ENAA_P2     	0x02
#define BIT_ENAA_P1     	0x01
#define BIT_ENAA_P0     	0x00
#define BIT_ERX_P5      	0x05
#define BIT_ERX_P4      	0x04
#define BIT_ERX_P3      	0x03
#define BIT_ERX_P2      	0x02
#define BIT_ERX_P1      	0x01
#define BIT_ERX_P0      	0x00
#define BIT_AW          	0x00
#define BIT_ARD         	0x04
#define BIT_ARC         	0x00
#define BIT_PLL_LOCK    	0x04
#define BIT_RF_DR       	0x03
#define BIT_RF_PWR      	0x06
#define BIT_RX_DR       	0x06
#define BIT_TX_DS       	0x05
#define BIT_MAX_RT      	0x04
#define BIT_RX_P_NO     	0x01
#define BIT_TX_FULL     	0x00
#define BIT_PLOS_CNT    	0x04
#define BIT_ARC_CNT     	0x00
#define BIT_TX_REUSE    	0x06
#define BIT_FIFO_FULL   	0x05
#define BIT_TX_EMPTY    	0x04
#define BIT_RX_FULL     	0x01
#define BIT_RX_EMPTY    	0x00
#define BIT_DPL_P5	    	0x05
#define BIT_DPL_P4	    	0x04
#define BIT_DPL_P3	    	0x03
#define BIT_DPL_P2	    	0x02
#define BIT_DPL_P1	    	0x01
#define BIT_DPL_P0	    	0x00
#define BIT_EN_DPL	    	0x02
#define BIT_EN_ACK_PAY  	0x01
#define BIT_EN_DYN_ACK  	0x00

/* Instruction Mnemonics */
#define CMD_R_REGISTER    	0x00
#define CMD_W_REGISTER    	0x20
#define CMD_REGISTER_MASK 	0x1F
#define CMD_ACTIVATE      	0x50
#define CMD_R_RX_PL_WID   	0x60
#define CMD_R_RX_PAYLOAD  	0x61
#define CMD_W_TX_PAYLOAD  	0xA0
#define CMD_W_ACK_PAYLOAD 	0xA8
#define CMD_FLUSH_TX      	0xE1
#define CMD_FLUSH_RX      	0xE2
#define CMD_REUSE_TX_PL   	0xE3
#define CMD_NOP           	0xFF
#define CMD_FLUSH			0xFF

/* Non-P omissions */
#define LNA_HCURR   		0x00

/* P model memory Map */
#define REG_RPD         	0x09

/* P model bit Mnemonics */
#define RF_DR_LOW   		0x05
#define RF_DR_HIGH  		0x03
#define RF_PWR_LOW  		0x01
#define RF_PWR_HIGH 		0x02
