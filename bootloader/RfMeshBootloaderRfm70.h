#ifndef __RfMeshBootloaderRfm70_h__
#define __RfMeshBootloaderRfm70_h__

#include "rfm70reg.h"

typedef struct {
    uint16_t to;
    uint8_t len;
	uint8_t data[RF_MESH_RAW_MESSAGE_LEN_MAX];
} rawMessageRfm70;

// private functions for rfm70 start
static void delaymicro() {
	asm("nop"); 
}

static void delaymilli(uint16_t t) {
	uint16_t d, a;
	for (d = 0; d < t; d++)
		for (a = 0; a < 500; a++)
			asm("nop"); 
}

static void led(uint8_t mask, uint16_t t) {
	LED_PORT = (LED_PORT & ~(LED_BITMASK_BOTH)) | mask;
	if (t > 0) {
		delaymilli(t);
		LED_PORT = (LED_PORT & ~(LED_BITMASK_BOTH));
		delaymilli(t);
	}
}

static unsigned char rfm70_SPI_RW(unsigned char value) {
	unsigned char i;
	for(i = 0 ;i < 8;i++) {
		delaymicro();
		if ((value & 0x80) == 0x00)
			RFM70_PORT &= ~(1 << RFM70_BIT_MOSI);
		else
			RFM70_PORT |= (1 << RFM70_BIT_MOSI);
		value = (value << 1);    // shift next bit into MSB..
		delaymicro();
		RFM70_PORT |= (1 << RFM70_BIT_SCLK);
		value |= ((RFM70_PIN >> RFM70_BIT_MISO) & 0x01);     // capture current MISO bit
		delaymicro();
		RFM70_PORT &= ~(1 << RFM70_BIT_SCLK);
		delaymicro();
	}
	return value;
}

static void rfm70_register_write(unsigned char reg, unsigned char value) {
	if(reg < RFM70_CMD_WRITE_REG)
		reg |= RFM70_CMD_WRITE_REG;
	RFM70_PORT &= ~(1 << RFM70_BIT_CSN);
	rfm70_SPI_RW(reg);
	rfm70_SPI_RW(value);
	RFM70_PORT |= (1 << RFM70_BIT_CSN);
}

static unsigned char rfm70_register_read(unsigned char reg) {
	unsigned char value;
	if(reg < RFM70_CMD_WRITE_REG)
		reg |= RFM70_CMD_READ_REG;       
	RFM70_PORT &= ~(1 << RFM70_BIT_CSN);
	rfm70_SPI_RW(reg);
	value = rfm70_SPI_RW(0);
	RFM70_PORT |= (1 << RFM70_BIT_CSN);
	return value;
}

static void rfm70_bank(unsigned char b) {
	unsigned char st = 0x80 & rfm70_register_read(RFM70_REG_STATUS);
	if((st && (b == 0)) || ((st == 0) && b)) {
		rfm70_register_write(RFM70_CMD_ACTIVATE, 0x53);
	}
}

static void rfm70_mode_receive() {
	unsigned char value;
	rfm70_register_write(RFM70_CMD_FLUSH_RX, 0);
	value = rfm70_register_read(RFM70_REG_STATUS);
	rfm70_register_write(RFM70_REG_STATUS, value);
	RFM70_PORT &= ~(1 << RFM70_BIT_CE);
	value = rfm70_register_read(RFM70_REG_CONFIG);
	value |= 0x01;
	value |= 0x02;
	rfm70_register_write(RFM70_REG_CONFIG, value);
	RFM70_PORT |= (1 << RFM70_BIT_CE);
}

static void rfm70_mode_transmit() {
	unsigned char value;
	rfm70_register_write(RFM70_CMD_FLUSH_TX, 0);
	value = rfm70_register_read(RFM70_REG_STATUS);
	rfm70_register_write(RFM70_REG_STATUS, value);
	RFM70_PORT &= ~(1 << RFM70_BIT_CE);
	value = rfm70_register_read(RFM70_REG_CONFIG);
	value &= 0xFE;
	value |= 0x02;
	rfm70_register_write(RFM70_REG_CONFIG, value);
	RFM70_PORT |= (1 << RFM70_BIT_CE);
}

static void rfm70_buffer_write(char reg, const unsigned char *pBuf, unsigned char length) {
	unsigned char i;
	if (reg < RFM70_CMD_WRITE_REG)
		reg |= RFM70_CMD_WRITE_REG;      
	RFM70_PORT &= ~(1 << RFM70_BIT_CSN);
	rfm70_SPI_RW(reg);
	for(i = 0; i < length; i++)
		rfm70_SPI_RW(*pBuf++);
	RFM70_PORT |= (1 << RFM70_BIT_CSN);
}

static void rfm70_buffer_read(unsigned char reg, unsigned char *pBuf, unsigned char length) {
	unsigned char i;
	if(reg < RFM70_CMD_WRITE_REG)
		reg |= RFM70_CMD_READ_REG;       
	RFM70_PORT &= ~(1 << RFM70_BIT_CSN);
	rfm70_SPI_RW(reg);
	for(i = 0; i < length; i++)
		*pBuf++ = rfm70_SPI_RW(0);
	RFM70_PORT |= (1 << RFM70_BIT_CSN);
}

static meshMessage RawToMesh(rawMessageRfm70 rm) {
	meshMessage mm;
	if (rm.len >= RF_MESH_MESH_MESSAGE_LEN_HEADER) {
		mm.to = rm.to;
		mm.from = (rm.data[1] << 8) + rm.data[0];
		mm.source = (rm.data[3] << 8) + rm.data[2];
		mm.dest = (rm.data[5] << 8) + rm.data[4];
		mm.pid = rm.data[6];
		mm.ptype = rm.data[7];
		mm.cost = rm.data[8];
		mm.len = rm.len - RF_MESH_MESH_MESSAGE_LEN_HEADER;
		for (uint8_t i = RF_MESH_MESH_MESSAGE_LEN_HEADER; i < rm.len; i++)
			mm.data[i - RF_MESH_MESH_MESSAGE_LEN_HEADER] = rm.data[i];
	}
	else
		mm.from = RF_MESH_BROADCAST;
	return mm;
}

static rawMessageRfm70 MeshToRaw(meshMessage mm) {
	rawMessageRfm70 rm;
	rm.to = mm.to;
	rm.len = mm.len + RF_MESH_MESH_MESSAGE_LEN_HEADER;
	rm.data[0] = (uint8_t)(mm.from & 0x00FF);
	rm.data[1] = (uint8_t)((mm.from & 0xFF00) >> 8);
	rm.data[2] = (uint8_t)(mm.source & 0x00FF);
	rm.data[3] = (uint8_t)((mm.source & 0xFF00) >> 8);
	rm.data[4] = (uint8_t)(mm.dest & 0x00FF);
	rm.data[5] = (uint8_t)((mm.dest & 0xFF00) >> 8);
	rm.data[6] = mm.pid;
	rm.data[7] = mm.ptype;
	rm.data[8] = mm.cost;
	for (uint8_t i = 0; i < mm.len; i++)
		rm.data[i + RF_MESH_MESH_MESSAGE_LEN_HEADER] = mm.data[i];
	return rm;
}
// private functions for rfm70 end

static uint16_t TransceiverAddress = RF_MESH_NOADDRESS;

const unsigned long Bank1_Reg0_13[] = {
   0xE2014B40,
   0x00004BC0,
   0x028CFCD0,
   0x41390099,
   0x0B869Ef9, 
   0xA67F0624,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00127300,
   0x36B48000 };   
   
const unsigned char Bank1_Reg14[] = {
   0x41, 0x20, 0x08, 0x04, 0x81, 0x20, 0xCF, 0xF7, 0xFE, 0xFF, 0xFF };   

static void TransceiverInit() {
	// init pins
	RFM70_DDR |= (1 << RFM70_BIT_MOSI) + (1 << RFM70_BIT_SCLK) + (1 << RFM70_BIT_CE) + (1 << RFM70_BIT_CSN);
	RFM70_DDR &= ~(1 << RFM70_BIT_MISO);
	RFM70_PORT |= (1 << RFM70_BIT_CSN);
	RFM70_PORT &= ~((1 << RFM70_BIT_MOSI) + (1 << RFM70_BIT_SCLK) + (1 << RFM70_BIT_CE));
	delaymilli(50);
	// init register bank 0
	rfm70_bank(0);
	rfm70_register_write( 0, 0x0F ); 
	rfm70_register_write( 1, 0x00 ); 
	rfm70_register_write( 5, RF_MESH_RFM70_CHANNEL );
	uint8_t addrarr[5];
	addrarr[4] = RF_MESH_RFM70_NET3;
	addrarr[3] = RF_MESH_RFM70_NET2;
	addrarr[2] = RF_MESH_RFM70_NET1;
    addrarr[1] = (uint8_t)((RF_MESH_BROADCAST & 0xFF00) >> 8);
    addrarr[0] = (uint8_t)(RF_MESH_BROADCAST & 0x00FF);
	rfm70_buffer_write( RFM70_REG_RX_ADDR_P1, addrarr, 5 );  
    addrarr[1] = (uint8_t)((RF_MESH_NOADDRESS & 0xFF00) >> 8);
    addrarr[0] = (uint8_t)(RF_MESH_NOADDRESS & 0x00FF);
	rfm70_buffer_write( RFM70_REG_RX_ADDR_P0, addrarr, 5 );  
	rfm70_buffer_write( RFM70_REG_TX_ADDR, addrarr, 5 );   
	if (rfm70_register_read(29) == 0)
		rfm70_register_write( RFM70_CMD_ACTIVATE, 0x73 );
	rfm70_register_write( 28, 0x3F ); 
	rfm70_register_write( 29, 0x07 );  
	// init register bank 1
	unsigned char i, j;
	unsigned char WriteArr[ 12 ];
	rfm70_bank(1);
	for( i = 0; i <= 8; i++ ){ //reverse!
		for( j = 0; j < 4; j++ ){
			WriteArr[ j ]=( Bank1_Reg0_13[ i ]>>(8*(j) ) )&0xff;
		}  
		rfm70_buffer_write( i,WriteArr, 4 );
	}
	for( i = 9; i <= 13; i++ ){
		for( j = 0; j < 4; j++ ){
			WriteArr[ j ]=( Bank1_Reg0_13[ i ]>>(8*(3-j) ) )&0xff;
		}
		rfm70_buffer_write( i, WriteArr, 4 );
	}
	rfm70_buffer_write( 14, Bank1_Reg14, 11 );
	//toggle REG4<25,26>
	for(j = 0; j < 4; j++){
		WriteArr[ j ]=( Bank1_Reg0_13[ 4 ]>>(8*(j) ) )&0xff;
	} 
	WriteArr[ 0 ] = WriteArr[ 0 ] | 0x06;
	rfm70_buffer_write( 4, WriteArr, 4);
	WriteArr[ 0 ] = WriteArr[ 0 ] & 0xf9;
	rfm70_buffer_write( 4, WriteArr,4);
	// finish init
	rfm70_bank( 0 );
	delaymilli(50); 
	rfm70_mode_receive();
}

static void TransceiverSetAddress(uint16_t addr) {
	uint8_t addrarr[5];
	addrarr[4] = RF_MESH_RFM70_NET3;
	addrarr[3] = RF_MESH_RFM70_NET2;
	addrarr[2] = RF_MESH_RFM70_NET1;
	addrarr[1] = (uint8_t)((addr & 0xFF00) >> 8);
	addrarr[0] = (uint8_t)(addr & 0x00FF);
	rfm70_buffer_write(RFM70_REG_RX_ADDR_P0, addrarr, 5);   
	TransceiverAddress = addr;
}

static void TransceiverSend(meshMessage mm) {
	rawMessageRfm70 rm = MeshToRaw(mm);
	uint8_t toarr[5];
	toarr[4] = RF_MESH_RFM70_NET3;
	toarr[3] = RF_MESH_RFM70_NET2;
	toarr[2] = RF_MESH_RFM70_NET1;
	toarr[1] = (uint8_t)((rm.to & 0xFF00) >> 8);
	toarr[0] = (uint8_t)(rm.to & 0x00FF);
	rfm70_buffer_write(RFM70_REG_TX_ADDR, toarr, 5);   
	rfm70_mode_transmit();
	rfm70_buffer_write(RFM70_CMD_W_TX_PAYLOAD_NOACK, rm.data, rm.len);
	delaymilli(1);
	rfm70_mode_receive();
}

static uint8_t TransceiverHasData() {
	return ((rfm70_register_read( RFM70_REG_FIFO_STATUS ) & FIFO_STATUS_RX_EMPTY ) == 0);
}

static meshMessage TransceiverReceive() {
	rawMessageRfm70 rm;
	if (TransceiverHasData()) {
		if (((rfm70_register_read( RFM70_REG_STATUS ) >> 1 ) & 0x07) == 0)
			rm.to = TransceiverAddress;
		else
			rm.to = RF_MESH_BROADCAST;
		rm.len = rfm70_register_read(RFM70_CMD_R_RX_PL_WID);
		rfm70_buffer_read(RFM70_CMD_R_RX_PAYLOAD, rm.data, rm.len);
		rfm70_register_write(RFM70_CMD_FLUSH_RX, 0);
	}
	meshMessage mm = RawToMesh(rm);
	return mm;
}

#endif