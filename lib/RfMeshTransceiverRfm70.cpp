#include "RfMeshTransceiverRfm70.h"

RfMeshTransceiverRfm70::RfMeshTransceiverRfm70(uint8_t sclk, uint8_t mosi, uint8_t miso, uint8_t csn, uint8_t ce)
	: rfm70(*(new pins::uc_pin(sclk)), *(new pins::uc_pin(mosi)), *(new pins::uc_pin(miso)), *(new pins::uc_pin(csn)), *(new pins::uc_pin(ce))) {
}

bool RfMeshTransceiverRfm70::Init(uint16_t Address) {
	pins::uc_init();
	rfm70::init();

	SetAddress(Address, false);
	SetAddress(RF_MESH_BROADCAST, true);
    rfm70::air_data_rate(2);
    rfm70::channel(RF_MESH_RFM70_CHANNEL);
    //rfm70::retransmit_delay_attempts(8, 15);
    rfm70::pipe_autoack(0, false);
    rfm70::pipe_autoack(1, false);
    if(!rfm70::is_present())
		return false;
	delay(50); 
    rfm70::mode_receive();
	return true;
}

void RfMeshTransceiverRfm70::SetAddress(uint16_t Address, bool brdcst) {
	uint8_t addrarr[5];
	addrarr[4] = RF_MESH_RFM70_NET3;
	addrarr[3] = RF_MESH_RFM70_NET2;
	addrarr[2] = RF_MESH_RFM70_NET1;
    addrarr[1] = (uint8_t)((Address & 0xFF00) >> 8);
    addrarr[0] = (uint8_t)(Address & 0x00FF);
	if (brdcst) {
		rfm70::receive_address_p1(addrarr);
	}
	else {
//		rfm70::transmit_address(addrarr);
		rfm70::receive_address_p0(addrarr);
		addr = Address;
	}
}

void RfMeshTransceiverRfm70::Send(meshMessage mm) {
	rawMessageRfm70 rm = MeshToRaw(mm);
	uint8_t toarr[5];
	toarr[4] = RF_MESH_RFM70_NET3;
	toarr[3] = RF_MESH_RFM70_NET2;
	toarr[2] = RF_MESH_RFM70_NET1;
    toarr[1] = (uint8_t)((rm.to & 0xFF00) >> 8);
    toarr[0] = (uint8_t)(rm.to & 0x00FF);
	rfm70::transmit_address(toarr);
	rfm70::mode_transmit();
	rfm70::buffer_write(RFM70_CMD_W_TX_PAYLOAD_NOACK, rm.data, rm.len);
	delay(1);
	rfm70::mode_receive();
}

bool RfMeshTransceiverRfm70::HasData() {
    return (!rfm70::receive_fifo_empty());
}

meshMessage RfMeshTransceiverRfm70::Receive() {
	rawMessageRfm70 rm;
    if (!receive_fifo_empty()) {
		// ToDo : get from address
		if (rfm70::receive_next_pipe() == 0)
			rm.to = addr;
		else
			rm.to = RF_MESH_BROADCAST;
		rm.len = rfm70::register_read(RFM70_CMD_R_RX_PL_WID);
		rfm70::buffer_read(RFM70_CMD_R_RX_PAYLOAD, rm.data, rm.len);
        rfm70::register_write(RFM70_CMD_FLUSH_RX, 0);
	}
    return RawToMesh(rm);
}

meshMessage RfMeshTransceiverRfm70::RawToMesh(rawMessageRfm70 rm) {
	meshMessage mm;
	if (rm.len >= RF_MESH_MESH_MESSAGE_LEN_HEADER) {
		mm.tc = index;
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

rawMessageRfm70 RfMeshTransceiverRfm70::MeshToRaw(meshMessage mm) {
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
