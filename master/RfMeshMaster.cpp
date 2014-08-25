#include "RfMeshMaster.h"

LoggerPtr logger(Logger::getLogger("ToSa.Mesh"));

int main(int argc, const char * argv[]) {
	ConsoleAppender * consoleAppender = new ConsoleAppender(LayoutPtr(new SimpleLayout()));
	consoleAppender->setImmediateFlush(true);
	BasicConfigurator::configure(AppenderPtr(consoleAppender));

	LOG4CXX_INFO(logger, "RfMeshMaster starting...");

	RfMeshDatabase::init();
	
	MessageQueue mqSubscribe;
	MessageQueue mqTx;
	MessageQueue mqRx;
	MessageQueue mqToUi;
	MessageQueue mqFromUi;
	
	RfMeshMessageDispatcher disp(&mqSubscribe, &mqTx, &mqRx, &mqToUi, &mqFromUi);

	RfMeshThreadNode tNode;
	RfMeshThreadSocket tSocket;
	RfMeshThreadTime tTime;
	RfMeshThreadDataLogger tDataLogger;

	tNode.setName("Node");
	tSocket.setName("Socket");
	tTime.setName("Time");
	tDataLogger.setName("DataLogger");
	
	tNode.start(&disp);
	tSocket.start(&disp);
	tTime.start(&disp);
	tDataLogger.start(&disp);

//	system("cd ../web; nohup nodejs RfMeshWeb.js >> ../log/RfMeshWeb.log 2>&1 &");
	
	LOG4CXX_INFO(logger, "RfMeshMaster running...");

	tDataLogger.join();	
	tTime.join();
	tSocket.join();
	tNode.join();
}
