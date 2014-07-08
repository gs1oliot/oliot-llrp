/**
 * @file    ReaderApp.h
 * @brief
 */

#ifndef STUBAPPEXAMPLE_H_
#define STUBAPPEXAMPLE_H_

#include <Stubs/StubApp.h>
#include <ELFINReader.h>
#include <ELFIN_ProgOpts.h>

/** @class ReaderApp
 * @brief Reader application example class. Developers can refer to this class to implement their own reader application.
 */
class ReaderApp : public ELFIN::StubReaderApp {
public:
	/// Constructor of ReaderApp class
	ReaderApp(ELFIN::ProgramOptions* __progOpt);
	/// Destructor of ReaderApp class
	virtual ~ReaderApp();
	/// Instantiate LLRP Wrapper and start it. And then, wait for the LLRP Wrapper thread finishes.
	int run();
	/// Send stop signal to LLRP Wrapper and wait until it actually finishes. And then, delete LLRP Wrapper.
	int stop();
	void initializaionFinishedCallback();
	void GPITriggeredInventoryFinishedCallback(int pROSpecID, ELFIN::TagVector *pReportedTagVector);
	void InventoryOperationFinishedCallback(int pROSpecID, ELFIN::TagVector *pReportedTagVector);
	void notifyConnectionResultCallback(int connResult);
	void readyToConnectCallback();
	void connectionClosedCallback();
	void GPOEventCallback(int portNum, int state);

	ELFIN::ProgramOptions *m_ProgOpt;
	boost::thread *m_LLRPReaderThread;
	ELFIN::ELFINReader *m_ELFINReader;
	int m_shutdownFlag;
};

#endif /* STUBAPPEXAMPLE_H_ */
