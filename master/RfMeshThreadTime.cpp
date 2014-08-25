#include "RfMeshThreadTime.h"

bool RfMeshThreadTime::setup() {
	timeToInterval = 0;
	timeToResponse = 0;
	timeToIntervalHoliday = 0;
	disp->mqSubscribe->enqueue(*this, new RfMeshThreadMessageSubscribe(&mqTimeRequests, RF_MESH_PTYPE_TIME_REQUEST, RF_MESH_NOADDRESS));
	return true;
}

bool RfMeshThreadTime::loop() {
	sleep(RF_MESH_TIME_SLEEP);
	while (!mqTimeRequests.empty()) {
//		RfMeshThreadMessageAppPtr mp = (RfMeshThreadMessageAppPtr)disp->mqRx->dequeue(*this);
		RfMeshThreadMessageAppPtr mp = (RfMeshThreadMessageAppPtr)mqTimeRequests.dequeue(*this);
		delete mp;
		if (timeToResponse == 0xFFFF)
			timeToResponse = RF_MESH_TIME_RESPONSE_DELAY;
	}
	if ((timeToInterval < RF_MESH_TIME_SLEEP) || (timeToResponse < RF_MESH_TIME_SLEEP)) {
		timeToInterval = RF_MESH_TIME_INTERVAL;
		timeToResponse = 0xFFFF;
		SendTime();
	} else {
		timeToInterval -= RF_MESH_TIME_SLEEP;
		if (timeToResponse != 0xFFFF)
			timeToResponse -= RF_MESH_TIME_SLEEP;
	}
	if (timeToIntervalHoliday < RF_MESH_TIME_SLEEP) {
		timeToIntervalHoliday = RF_MESH_HOLIDAY_INTERVAL;
		SendHoliday();
	} else {
		timeToIntervalHoliday -= RF_MESH_TIME_SLEEP;
	}
	return true;
}

void RfMeshThreadTime::SendTime() {
	disp->mqTx->enqueue(*this, new RfMeshThreadMessageApp(getTimeMessage()));
}

void RfMeshThreadTime::SendHoliday() {
	disp->mqTx->enqueue(*this, new RfMeshThreadMessageApp(getHolidayMessage()));
}

appMessage RfMeshThreadTime::getTimeMessage() {
	time_t raw;
	struct tm * local;
	time(&raw);
	local = localtime(&raw);
	appMessage am;
	am.source = RF_MESH_NOADDRESS;
	am.dest = RF_MESH_BROADCAST;
	am.pid = 0;
	am.ptype = RF_MESH_PTYPE_TIME_BROADCAST;
	am.len = 8; // = sizeof Timestamp;
	Timestamp *req = (Timestamp *)am.data;
	req->year = local->tm_year + 1900;
	req->month = local->tm_mon + 1;
	req->day = local->tm_mday;
	req->hour = local->tm_hour;
	req->minute = local->tm_min;
	req->second = local->tm_sec;
	req->weekday = local->tm_wday;
	return am;
}

appMessage RfMeshThreadTime::getHolidayMessage() {
	Datestamp nextBankHoliday = getNextBankHoliday();
	Datestamp nextVacationStart = getNextVacationStart();
	Datestamp nextVacationEnd = getNextVacationEnd();
	appMessage am;
	am.source = RF_MESH_NOADDRESS;
	am.dest = RF_MESH_BROADCAST;
	am.pid = 0;
	am.ptype = RF_MESH_PTYPE_HOLIDAY_BROADCAST;
	am.len = 12; // = 3 * sizeof Datestamp;
	am.data[0] = (uint8_t)(nextBankHoliday.year & 0x00FF);
	am.data[1] = (uint8_t)((nextBankHoliday.year & 0xFF00) >> 8);
	am.data[2] = nextBankHoliday.month;
	am.data[3] = nextBankHoliday.day;
	am.data[4] = (uint8_t)(nextVacationStart.year & 0x00FF);
	am.data[5] = (uint8_t)((nextVacationStart.year & 0xFF00) >> 8);
	am.data[6] = nextVacationStart.month;
	am.data[7] = nextVacationStart.day;
	am.data[8] = (uint8_t)(nextVacationEnd.year & 0x00FF);
	am.data[9] = (uint8_t)((nextVacationEnd.year & 0xFF00) >> 8);
	am.data[10] = nextVacationEnd.month;
	am.data[11] = nextVacationEnd.day;
	return am;
}

Datestamp RfMeshThreadTime::getNextBankHoliday() {
	Datestamp d;
	// ToDo : get from database
	d.year = 2013;
	d.month = 11;
	d.day = 20;
	return d;
}

Datestamp RfMeshThreadTime::getNextVacationStart() {
	Datestamp d;
	// ToDo : get from database
	d.year = 2013;
	d.month = 10;
	d.day = 14;
	return d;
}

Datestamp RfMeshThreadTime::getNextVacationEnd() {
	Datestamp d;
	// ToDo : get from database
	d.year = 2013;
	d.month = 10;
	d.day = 18;
	return d;
}
