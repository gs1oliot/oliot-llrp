/**
 * @file    ReaderApp.cpp
 * @brief
 */

#include <boost/thread.hpp>
#include <stdlib.h>
#include <signal.h>
#include "ReaderApp.h"

ReaderApp *_pApp;

// This is for test. Shut down the StubApp
void
signal_callback_handler(int signum)
{
	if (_pApp->m_shutdownFlag == 0) {
		_pApp->m_shutdownFlag = 1;
		_pApp->stop();
	}
	else {
		exit(0);
	}
}


int main(int argc, char * argv[]) {
	// ignore 'Broken Pipe' signal that happens when it is disconnected from FnC
	signal(SIGPIPE, SIG_IGN);
	// SIGINT lets the reader app to shutdown
	signal(SIGINT, signal_callback_handler);

	printf("INFO: Gathering options from reader.cfg....\n");
	ELFIN::ProgramOptions* progOpt = new ELFIN::ProgramOptions();
	try {
		progOpt->processCmdlineOptions(argc, argv);

		if (progOpt->isHelpOption()) {
			progOpt->printHelp();
		} else {
			//start the work
			progOpt->printOptions();

			printf("INFO: Starting ReaderApp....\n");
			_pApp = new ReaderApp(progOpt);
			// Register signal and signal handler
			_pApp->run();
		}
	} catch (std::exception& e) {
		printf("ERROR: %s\n", e.what());
		return 1;
	} catch (...) {
		printf("ERROR: Exception of unknown type!\n");
	}
	delete _pApp;

	return 0;
}

ReaderApp::ReaderApp(ELFIN::ProgramOptions* __progOpt)
: m_ProgOpt(__progOpt), m_LLRPReaderThread(NULL), m_ELFINReader(NULL), m_shutdownFlag(0) {

}

ReaderApp::~ReaderApp() {
	printf ("DEBUG: Destroying ReaderApp...\n");
	this->stop();
	if (m_ProgOpt != NULL) {
		delete m_ProgOpt;
	}
}

int ReaderApp::run() {
	printf("INFO: Starting LLRP Reader....\n");

	m_ELFINReader = new ELFIN::ELFINReader(m_ProgOpt);
	m_ELFINReader->init();
	m_ELFINReader->connectReaderApp(this);
	m_LLRPReaderThread = new boost::thread(
			boost::bind(&ELFIN::ELFINReader::run, m_ELFINReader));

	// Wait until the reader thread returns
	if (m_LLRPReaderThread != NULL)
		m_LLRPReaderThread->join();

	return 0;
}

int ReaderApp::stop() {
	if (m_ELFINReader != NULL) {
		m_ELFINReader->setShutdownFlag();
		printf ("Waiting for LLRP Wrapper to finish current job...\n");
		m_LLRPReaderThread->join();
		printf ("LLRP Wrapper is closed...\n");
		delete m_LLRPReaderThread;
		delete m_ELFINReader;
		m_LLRPReaderThread = NULL;
		m_ELFINReader = NULL;
	}
	return 0;
}

void ReaderApp::initializaionFinishedCallback() {
	printf ("INFO: initializaionFinishedCallback is invoked.\n");
}

void ReaderApp::GPITriggeredInventoryFinishedCallback(int pROSpecID, ELFIN::TagVector *pReportedTagVector) {
	printf ("INFO: GPITriggeredInventoryFinishedCallback is invoked.\n");
	//int antennaID = 0;
	//ELFIN::TagVector *tagVector = _pReader->getCurrentTags(antennaID);
	int tagCount = pReportedTagVector->size();
	printf ("INFO: GPITriggeredInventoryFinishedCallback: %d tags are reported by ROSpec ID %d\n", tagCount, pROSpecID);
}

void ReaderApp::InventoryOperationFinishedCallback(int pROSpecID, ELFIN::TagVector *pReportedTagVector) {
	printf ("INFO: InventoryOperationFinishedCallback is invoked.\n");
	//int antennaID = 0;
	//ELFIN::TagVector *tagVector = _pReader->getCurrentTags(antennaID);
	int tagCount = pReportedTagVector->size();
	printf ("INFO: InventoryOperationFinishedCallback: %d tags are reported by ROSpec ID %d\n", tagCount, pROSpecID);
}

void ReaderApp::notifyConnectionResultCallback(int connResult) {
	printf ("INFO: notifyConnectionResultCallback is invoked with status %d\n", connResult);
}

void ReaderApp::readyToConnectCallback() {
	printf ("INFO: readyToConnectCallback is invoked.\n");
	if (this->m_ELFINReader->getShutdownFlag() != 1)
		this->m_ELFINReader->startConnection();
}

void ReaderApp::connectionClosedCallback() {
	printf ("INFO: connectionClosedCallback is invoked.\n");
	if (this->m_ELFINReader->getShutdownFlag() != 1)
		this->m_ELFINReader->startConnection();
}

void ReaderApp::GPOEventCallback(int portNum, int state) {
	printf ("INFO: GPOEventCallback is invoked for port %d and state %d\n", portNum, state);
}

