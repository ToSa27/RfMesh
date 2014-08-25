#ifndef __RfMeshThreadTime_h__
#define __RfMeshThreadTime_h__

#include "RfMeshThreading.h"

#define RF_MESH_TIME_SLEEP			10
#define RF_MESH_TIME_RESPONSE_DELAY	60
#define RF_MESH_TIME_INTERVAL			15*60		// alle 15 minuten zeit senden
#define RF_MESH_HOLIDAY_INTERVAL		12*60*60	// alle 12 stunden feiertage senden

class RfMeshThreadTime : public RfMeshThreadBase {
	public:
		bool setup();
		bool loop();
	private:
		uint16_t timeToInterval;
		uint16_t timeToResponse;
		uint16_t timeToIntervalHoliday;
		void SendTime();
		void SendHoliday();
		appMessage getTimeMessage();
		appMessage getHolidayMessage();
		MessageQueue mqTimeRequests;
		Datestamp getNextBankHoliday();
		Datestamp getNextVacationStart();
		Datestamp getNextVacationEnd();
};

#endif