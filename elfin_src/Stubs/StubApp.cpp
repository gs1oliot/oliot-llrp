/**
 * @file    StubApp.cpp
 * @brief
 */

//#include "../LLRPCore.h"
#include "StubApp.h"
#include "StubReader.h"
#include "StubTag.h"
#include "../ELFIN_Platform.h"

ELFIN::StubReaderApp::StubReaderApp()
:_pReader(NULL), _GPICallBack(NULL), _isEmulator(1), _isConnectedToReader(0) {

}

ELFIN::StubReaderApp::StubReaderApp(StubReader *__pReader)
:_pReader(__pReader), _GPICallBack(NULL), _isEmulator(1), _isConnectedToReader(0) {

}
ELFIN::StubReaderApp::~StubReaderApp() {
	READER_LOG (LOGTYPE_TRACE, "Destroying StubApp...\n");

}

int ELFIN::StubReaderApp::regGPICallback(
		boost::function<int(StubReader*, uint16_t, int)> callback) {
	_GPICallBack = callback;
	return 0;
}

void ELFIN::StubReaderApp::initializaionFinishedCallback() {

}

void ELFIN::StubReaderApp::GPITriggeredInventoryFinishedCallback(int pROSpecID, TagVector *pReportedTagVector) {
}

void ELFIN::StubReaderApp::InventoryOperationFinishedCallback(int pROSpecID, TagVector *pReportedTagVector) {
}

void ELFIN::StubReaderApp::notifyConnectionResultCallback(int connResult) {

}

int ELFIN::StubReaderApp::invokeGPIEvent(unsigned short int GPIPortNum, int state) {
	if (_isConnectedToReader == 0)
		return -1;
	if (_GPICallBack != NULL)
		return _GPICallBack (_pReader, GPIPortNum, (LLRP::EGPIPortState) state);
	else
		return -1;
}

void ELFIN::StubReaderApp::GPOEventCallback(int portNum, int state) {
}

void ELFIN::StubReaderApp::readyToConnectCallback() {
}


void ELFIN::StubReaderApp::connectionClosedCallback() {
}


/**
 * @warning EPCID should be a char array of 24 chars and one last null char.\n
 * Example: this->addVirtualTagToReader(1, "35123456789ABCDEF0123456");
 */
ELFIN::StubTag* ELFIN::StubReaderApp::addVirtualTagToReader(
		int pAntennaID, const char* EPCID) {
	READER_LOG (LOGTYPE_TRACE, "Gaining lock of the reader to add virtual tag %s\n", EPCID);
	boost::recursive_mutex::scoped_lock lock(this->_pReader->m_StubReaderLock);
	READER_LOG (LOGTYPE_TRACE, "Gained lock of the reader to add virtual tag %s\n", EPCID);
	StubTag *pTag = new StubTag(pAntennaID);
	pTag->setEPC(EPCID);
	this->_pReader->m_VirtualTagVector.push_back(pTag);
	READER_LOG (LOGTYPE_TRACE, "Successfully added virtual tag %s\n", pTag->_logEpcStr);
	return pTag;
}

/**
 * @warning EPCID should be a char array of 24 chars and one last null char.\n
 * Example: this->addVirtualTagToReader(1, "35123456789ABCDEF0123456");
 */
void ELFIN::StubReaderApp::removeVirtualTagFromReader(const char* EPCID) {
	READER_LOG (LOGTYPE_TRACE, "Gaining lock of the reader to remove virtual tag %s\n", EPCID);
	boost::recursive_mutex::scoped_lock lock(this->_pReader->m_StubReaderLock);
	READER_LOG (LOGTYPE_TRACE, "Gained lock of the reader to remove virtual tag %s\n", EPCID);
	TagVector *pVTagVector = &this->_pReader->m_VirtualTagVector;
	// EPC string is stored as unsigned char, so convert it.
	unsigned char epcStr[25];
	memcpy(epcStr, EPCID, sizeof(epcStr));
	epcStr[24] = '\0';

	for (TagVector::iterator iter = pVTagVector->begin();
			iter != pVTagVector->end();iter++) {
		if (memcmp(epcStr, (*iter)->_logEpcStr, 24) == 0) {
			StubTag *pTag = (*iter);
			pVTagVector->erase(iter);
			delete pTag;
			READER_LOG (LOGTYPE_TRACE, "Successfully removed virtual tag %s\n", EPCID);
		}
	}
}

void ELFIN::StubReaderApp::addRandomVirtualTags(int numOfTagsPerAntenna) {
	READER_LOG (LOGTYPE_TRACE, "Gaining lock of the reader to add %d randomly generated virtual tags\n", numOfTagsPerAntenna);
	boost::recursive_mutex::scoped_lock lock(this->_pReader->m_StubReaderLock);
	READER_LOG (LOGTYPE_TRACE, "Gained lock of the reader to add %d randomly generated virtual tags\n", numOfTagsPerAntenna);
	srand(time(NULL));
	for (unsigned int i = 1;i <= this->_pReader->m_AntennaMap.size();i++) {
		for (int j = 0;j < numOfTagsPerAntenna;j++) {
			// Always use same number for seed(to generate EPCID),
			// to maintain same tags during the single execution.
			StubTag *pTag = new StubTag(i);
			pTag->setRandEPC(i + j * 100 + rand());
			this->_pReader->m_VirtualTagVector.push_back(pTag);
		}
	}
	READER_LOG (LOGTYPE_TRACE, "Successfully added %d randomly generated virtual tags\n", numOfTagsPerAntenna);
}




/* Removes all virtual tags from the reader.
 */
void ELFIN::StubReaderApp::clearVirtualTagOfReader() {
	READER_LOG (LOGTYPE_TRACE, "Gaining lock of the reader to clear virtual tags\n");
	boost::recursive_mutex::scoped_lock lock(this->_pReader->m_StubReaderLock);
	for (TagVector::iterator tagIter = this->_pReader->m_VirtualTagVector.begin();
			tagIter != this->_pReader->m_VirtualTagVector.end(); tagIter++) {
		delete (*tagIter);
	}
	this->_pReader->m_VirtualTagVector.clear();
	READER_LOG (LOGTYPE_TRACE, "Successfully cleared virtual tags\n");
}
