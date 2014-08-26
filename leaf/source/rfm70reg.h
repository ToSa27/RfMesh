#ifndef _RFM70REG_H_
#define _RFM70REG_H_

#define RFM70_MAX_PACKET_LEN  32

//***************************************************************************//
//
// RFM70 SPI commands
//
//***************************************************************************//

//! SPI comamnd to read a received payload
#define RFM70_CMD_R_RX_PAYLOAD         0x61

//! SPI command to write a payload to be sent
#define RFM70_CMD_W_TX_PAYLOAD         0xA0

//! SPI command to empty the transmit queue
#define RFM70_CMD_FLUSH_TX             0xE1

//! SPI command to empty the receive queue
#define RFM70_CMD_FLUSH_RX             0xE2

//! SPI command to start continuous retransmission
#define RFM70_CMD_REUSE_TX_PL          0xE3

//! SPI command to write a payload to be sent without auto-acknowledgement
#define RFM70_CMD_W_TX_PAYLOAD_NOACK   0xB0

//! SPI command to write the payload to be transmitted with an ack
#define RFM70_CMD_W_ACK_PAYLOAD        0xA8

//! SPI command to toggle register bank or toggle extended functions
#define RFM70_CMD_ACTIVATE             0x50

//! SPI command to read the payload length for the top payload in the FIFO
#define RFM70_CMD_R_RX_PL_WID          0x60

//! SPI 'no peration', can be used to read the status register
#define RFM70_CMD_NOP                  0xFF

//***************************************************************************//
//
// RFM70 register addresses
//
//***************************************************************************//

//! CONFIG : rfm70 configuration register
//
//! Bits (0 = LSB):
//! - 7 : reserved, must be 0
//! - 6 : 1 masks RX_DR (see...) from IRQ pin, 0 allows
//! - 5 : 1 masks RX_DS (see...) from IRQ pin, 0 allows
//! - 4 : 1 masks MAX_RT (see...) from IRQ pin, 0 allows
//! - 3 : 1 enables CRC (forced high when EN_AA != 0)
//! - 2 : 0 = 1 byte CRC, 1 = 2 byte CRC
//! - 1 : 0 = power down, 1 = power up
//! - 0 : 0 = transmit mode, 1 = receive mode
#define RFM70_REG_CONFIG               0x00

//! EN_AA : enable auto ack on pipes
//
//! Bits (0 = LSB):
//! - 7, 6 : reserved, must be 00
//! - 5 : 0 disables auto ack on pipe 5, 1 enables
//! - 4 : 0 disables auto ack on pipe 4, 1 enables
//! - 3 : 0 disables auto ack on pipe 3, 1 enables
//! - 2 : 0 disables auto ack on pipe 2, 1 enables
//! - 1 : 0 disables auto ack on pipe 1, 1 enables
//! - 0 : 0 disables auto ack on pipe 0, 1 enables
#define RFM70_REG_EN_AA                0x01

//! EN_RXADDR : enable receive pipes
//
//! Bits (0 = LSB):
//! - 7, 6 : reserved, must be 00
//! - 5 : 0 disables receive pipe 5, 1 enables
//! - 4 : 0 disables receive pipe 4, 1 enables
//! - 3 : 0 disables receive pipe 3, 1 enables
//! - 2 : 0 disables receive pipe 2, 1 enables
//! - 1 : 0 disables receive pipe 1, 1 enables
//! - 0 : 0 disables receive pipe 0, 1 enables
#define RFM70_REG_EN_RXADDR            0x02

//! SETUP_AW : set address length
//
//! Bits (0 = LSB):
//! - 7 .. 2 : reserved, must be 000000
//! - 1 .. 0 : 00 = illegal, 01 = 3 bytes, 10 = 4 bytes, 11 = 5 bytes
#define RFM70_REG_SETUP_AW             0x03

//! SETUP_RETR : retransmission settings
//
//! Bits (0 = LSB):
//! - 7 .. 4 : delay between (re) transmissions, ( n + 1 ) * 250 us
//! - 3 .. 0 : max number of retransmissions, 0 disableles retransmissions
#define RFM70_REG_SETUP_RETR           0x04

//! RF_CH : RF channel (frequency)
//
//! The RF channel frequency is 2.4 MHz + n * 1 MHz.
#define RFM70_REG_RF_CH                0x05

//! RF_SETUP : RF setup: data rate, transmit power, LNA
//
//! Bits (0 = LSB):
//! - 7 .. 4 : reserved, must be 0011
//! - 3 : air data rate, 0 = 1 Mbps, 1 = 2 Mbps
//! - 2 .. 1 : transmit power, 00 = -10 dBm, 01 = -5 dBm, 10 = 0 dBm, 11 = 5 dBm
//! - 0 : LNA gain, 0 = - 20 dB (low gain), 1 = standard
#define RFM70_REG_RF_SETUP             0x06

//! STATUS : status register
//
//! The value of this register is also clocked out
//! while a SPI command is clocked in.
//!
//! Bits (0 = LSB):
//! - 7 : active register bank, 0 = bank 0, 1 = bank 1
//! - 6 : data available, 0 = RX FIFO not empty, 1 = RX FIFO empty
//! - 5 : data sent, 0 = no packet sent, 1 = packet has been sent
//! - 4 : 1 = maximum number of retransmissions reached
//! - 3 .. 1 : data pipe of the message at the RX queue head
//! - 0 : TX FIFO full: 0 = TX FIFO not full, 1 = TX FIFO full
//!
//! Bits 6,5,4 are cleared by writing a 1 (!) in that position.
//! When bit 4 is set this will block any communication.
//! When auto retransmission is enabled bit 5 will be set only
//! after the acknowledge has been received.
#define RFM70_REG_STATUS               0x07

//! OBSERVE_TX : lost and retransmitted packets
//
//! Bits (0 = LSB):
//! - 7 .. 4 : counts number of lost packets
//! - 3 .. 0 : counts retranmits 
//! The lost packets counter will not increment beyond 15. 
//! It is reset by writing to the channel frequency register.
//!
//! The retransmits counter can not increment beyond 15 because
//! the maximum number of transmissions is 15. This counter
//! is reset when the transmission of a new packet starts.
#define RFM70_REG_OBSERVE_TX           0x08

//! CD : carrier detect
//
//! Bits (0 = LSB):
//! - 7 .. 1 : reserved
//! - 1 : carrier detect
#define RFM70_REG_CD                   0x09

//! RX_ADDR_PO : receive address for data pipe 0, 5 bytes
//
//! This is the (up to) 5 byte receive address for data pipe 0.
//! For auto acknowledgement to work this address must be 
//! the same as the transmit address.
#define RFM70_REG_RX_ADDR_P0           0x0A

//! RX_ADDR_P1 : receive address for data pipe 1, 5 bytes
//
//! This is the (up to) 5 byte receive address for data pipe 1.
//! The higher bytes (all but the LSB) are also used in
//! the receive addresses of data pipes 2 .. 5.
#define RFM70_REG_RX_ADDR_P1           0x0B

//! RX_ADDR_P2 : receive address for data pipe 2, 1 byte
//
//! This is the LSB of the receive address for data pipe 2.
//! The higher bytes are copied from the receive address of
//! data pipe 1.
#define RFM70_REG_RX_ADDR_P2           0x0C

//! RX_ADDR_P3 : receive address for data pipe 3, 1 byte
//
//! This is the LSB of the receive address for data pipe 3.
//! The higher bytes are copied from the receive address of
//! data pipe 1.
#define RFM70_REG_RX_ADDR_P3           0x0D

//! RX_ADDR_P4 : receive address for data pipe 4, 1 byte
//
//! This is the LSB of the receive address for data pipe 4.
//! The higher bytes are copied from the receive address of
//! data pipe 1.
#define RFM70_REG_RX_ADDR_P4           0x0E

//! RX_ADDR_P5 : receive address for data pipe 5, 1 byte
//
//! This is the LSB of the receive address for data pipe 2.
//! The higher bytes are copied from the receive address of
//! data pipe 5.
#define RFM70_REG_RX_ADDR_P5           0x0F

//! TX_ADDR : tranmsit adress, 5 bytes
//
//! This is the (up to) 5 byte adress used in transmitted packets.
//! For auto acknowledgement to work this address must be 
//! the same as the pipe 0 receive address.
#define RFM70_REG_TX_ADDR              0x10

//! RX_PW_P0 : number of bytes in package received into pipe 0
//
//! This is the number of data bytes in the message at
//! the head of receive pipe 0.
#define RFM70_REG_RX_PW_P0             0x11

//! RX_PW_P1 : number of bytes in package received into pipe 1
//
//! This is the number of data bytes in the message at
//! the head of receive pipe 1.
#define RFM70_REG_RX_PW_P1             0x12

//! RX_PW_P2 : number of bytes in package received into pipe 2
//
//! This is the number of data bytes in the message at
//! the head of receive pipe 2.
#define RFM70_REG_RX_PW_P2             0x13

//! RX_PW_P3 : number of bytes in package received into pipe 3
//
//! This is the number of data bytes in the message at
//! the head of receive pipe 3.
#define RFM70_REG_RX_PW_P3             0x14

//! RX_PW_P4 : number of bytes in package received into pipe 4
//
//! This is the number of data bytes in the message at
//! the head of receive pipe 4.
#define RFM70_REG_RX_PW_P4             0x15

//! RX_PW_P5 : number of bytes in package received into pipe 5
//
//! This is the number of data bytes in the message at
//! the head of receive pipe 5.
#define RFM70_REG_RX_PW_P5             0x16

//! FIFO_STATUS : receive and transmit FIFO status (readonly)
//
//! Bits (0 = LSB):
//! - 7   : reserved, only 0 allowed
//! - 6   : high = re-use last transmitted packet
//! - 5   : high = transmit FIFO is full
//! - 4   : high = transmit FIFO is empty
//! - 3:2 : reserved, only 0 allowed
//! - 1   : high = receive FIFO is full
//! - 0   : high = receive FIFO is empty
#define RFM70_REG_FIFO_STATUS          0x17

//! DYNPD: dynamic payload flags
//
//! Bits (0 = LSB):
//! - 7:6 : reserved, only 00 allowed
//! - 5   : high = dynamic payload enabled on data pipe 5
//! - 4   : high = dynamic payload enabled on data pipe 4
//! - 3   : high = dynamic payload enabled on data pipe 3
//! - 2   : high = dynamic payload enabled on data pipe 2
//! - 1   : high = dynamic payload enabled on data pipe 1
//! - 0   : high = dynamic payload enabled on data pipe 0
//! Setting dynamic payload on pipe x requires EN_DPL 
//! (in the special features flags register) and ENAA_Px.
#define RFM70_REG_DYNPD                0x1C

//! FEATURE: special fature flags
//
//! Bits (0 = LSB):
//! - 7:3 : reserved, only 00000 allowed
//! - 2   : (EN_DPL) high = enable dynamic payload length
//! - 1   : (EN_ACK_PAY) high = enable payload with ack
//! - 0   : (EN_DYN_ACK) high = enables W_TX_PAYLOAD_NOACK command 
#define RFM70_REG_FEATURE              0x1D

// RFM70 SPI read and write commands
#define RFM70_CMD_READ_REG            0x00
#define RFM70_CMD_WRITE_REG           0x20

//interrupt status
#define STATUS_RX_DR    0x40
#define STATUS_TX_DS    0x20
#define STATUS_MAX_RT   0x10

#define STATUS_TX_FULL  0x01

//FIFO_STATUS
#define FIFO_STATUS_TX_REUSE  0x40
#define FIFO_STATUS_TX_FULL   0x20
#define FIFO_STATUS_TX_EMPTY  0x10

#define FIFO_STATUS_RX_FULL   0x02
#define FIFO_STATUS_RX_EMPTY  0x01

// ToSa Mesh specifics - first three bytes of each address and channel number
#define RF_MESH_RFM70_NET1				0x55
#define RF_MESH_RFM70_NET2				0x00
#define RF_MESH_RFM70_NET3				0x00
#define RF_MESH_RFM70_CHANNEL				10

#endif
