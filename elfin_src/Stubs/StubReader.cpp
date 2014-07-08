/**
 * @file    StubReader.cpp
 * @brief
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <boost/thread/thread.hpp>
#include <boost/chrono.hpp>
#include "StubReader.h"
#include "../ReaderHandler/SampleReader.h"

ELFIN::AbstractReader *_pAbstractReader;

ELFIN::StubReader::StubReader(LLRPCore *__pLLRPCore)
: m_LLRPCore(__pLLRPCore), m_ProgOpt(__pLLRPCore->m_progOpt) {
	std::string readerEPCStr = m_ProgOpt->getEPC();
	m_ReaderEPC = new LLRP::llrp_u96_t();
	READER_LOG(LOGTYPE_INFO, "EPC: %s\n", readerEPCStr.c_str());

	for (int j = 0; j < 12; j++) {
		m_ReaderEPC->m_aValue[j] = ha2i_str(
				readerEPCStr.substr(j * 2, 2).c_str());
	}
	this->m_ReadCycle = m_ProgOpt->getReadCycle();
	this->m_NumberOfStubTagsPerAntenna = m_ProgOpt->getVirtualTagCount();

	LLRP::llrp_u16_t antennaCount = m_ProgOpt->getAntennaCount();
	LLRP::llrp_u16_t GPIPortCount = m_ProgOpt->getGPIPortCount();
	LLRP::llrp_u16_t GPOPortCount = m_ProgOpt->getGPOPortCount();
	// Generate Antennas and store them
	for (LLRP::llrp_u16_t i = 1;i <= antennaCount;i++) { // AntennaID starts from 1
		StubAntenna *pAntenna = new StubAntenna(this, i, 1, 0);
		if (this->addAntenna(pAntenna) != 0) {
			delete pAntenna;
		}
	}
	// Generate GPI/GPO ports and store them
	for (LLRP::llrp_u16_t i = 1;i <= GPIPortCount;i++) { // GPI/GPO Port ID starts from 1
		StubGPIPort *pGPIPort = new StubGPIPort(this, i, 1, LLRP::GPIPortState_Low);
		if (this->addGPIPort(pGPIPort) != 0) {
			delete pGPIPort;
		}
	}
	for (LLRP::llrp_u16_t i = 1;i <= GPOPortCount;i++) { // GPI/GPO Port ID starts from 1
		StubGPOPort *pGPOPort = new StubGPOPort(this, i, 1);
		if (this->addGPOPort(pGPOPort) != 0) {
			delete pGPOPort;
		}
	}
	// Generate virtual tags and store them to _pVirtualTagVector
	srand(time(NULL));
	for (unsigned int i = 1;i <= m_AntennaMap.size();i++) {
		for (int j = 0;j < m_NumberOfStubTagsPerAntenna;j++) {
			StubTag *pTag = new StubTag(i);
			pTag->setRandEPC(rand());
			this->m_VirtualTagVector.push_back(pTag);
		}
	}

	// Generate StubApp object and store it. Then register the GPI callback
	m_StubApp = new StubReaderApp(this);
	m_StubApp->regGPICallback(&StubReader::invokeGPIEventCallback);
	if (m_ProgOpt->getIsEmulatorMode() == 1) {
		// RealReader leave as NULL in emulator mode.
		_pAbstractReader = NULL;
	}
	else {
		_pAbstractReader = new SampleReader(this);
		_pAbstractReader->init();
		_pAbstractReader->regTagCallback(&StubReader::inventoryTag);
	}
}


unsigned char ELFIN::StubReader::hatoi(unsigned char a) {
	if (a >= '0' && a <= '9')
		return a - '0';
	if (a >= 'a' && a <= 'f')
		return a - 'a' + 10;
	if (a >= 'A' && a <= 'F')
		return a - 'A' + 10;
	return 0;
}

LLRP::llrp_u8_t ELFIN::StubReader::ha2i_str(const char *str) {
	LLRP::llrp_u8_t val;
	int i;
	int len;
	val = 0;
	len = strlen(str);
	if (len == 0)
		return 0;

	if (len > 1)
		for (i = len - 2; i >= 0; i--)
			val += hatoi(str[i]) * (16 << (4 * (len - 2 - i)));
	val += hatoi(str[len - 1]);
	return val;
}


ELFIN::StubReader::~StubReader() {
	READER_LOG (LOGTYPE_TRACE, "Destroying StubReader...\n");
	boost::recursive_mutex::scoped_lock lock(this->m_StubReaderLock);
	if (m_ReaderEPC != NULL)
		delete m_ReaderEPC;
	for (AntennaMap::iterator iter = m_AntennaMap.begin();
			iter != m_AntennaMap.end();iter++) {
		(*iter).second->clearAntennaFoV();
	}
	clearAntennaMap();
	clearGPIPortMap();
	clearGPOPortMap();
	this->m_StubApp->clearVirtualTagOfReader();
	if (this->m_StubApp->_isEmulator == 1)
		delete m_StubApp;

	if (_pAbstractReader != NULL)
		delete _pAbstractReader;
}

int ELFIN::StubReader::StartTagSingulation(int __pAntennaID) {
	//_pReaderLock.lock();
	int result;
	READER_LOG (LOGTYPE_DEBUG, "Tag singulation started: tagread()\n");
	result = this->tagRead();
	READER_LOG (LOGTYPE_DEBUG, "Tag singulation started: tagread() finished\n");
	return result;
}

void ELFIN::StubReader::FinishTagSingulation() {
	READER_LOG (LOGTYPE_DEBUG, "Tag singulation finished\n");
	//_pReaderLock.unlock();
}

void ELFIN::StubReader::InterruptTagSingulation() {
	if (this->m_ProgOpt->getIsEmulatorMode() == 0) {
		_pAbstractReader->_stopTagRead();
	}
	READER_LOG (LOGTYPE_DEBUG, "Tag singulation is inturrpted\n");
	//_pReaderLock.unlock();
}

/**
 * @return Pointer to vector of StubTag for the given antenna ID. If antenna ID is 0,
 * return all tags in all antennas.
 * @warning You should delete the returned vector to avoid memory leakage.
 */
ELFIN::TagVector *ELFIN::StubReader::getCurrentTags(int __pAntennaID) {
	boost::recursive_mutex::scoped_lock lock(m_StubReaderLock);
	TagVector *tagVector = new TagVector();
	if (__pAntennaID != 0) {
		StubAntenna *_pAntenna = m_AntennaMap[__pAntennaID];
		tagVector->resize(_pAntenna->countAntennaFoV(), NULL);
		int i = 0;
		for (AntennaFoVMap::iterator iter = _pAntenna->beginAntennaFoV();
				iter != _pAntenna->endAntennaFoV();iter++) {
			tagVector->at(i) = ((*iter).second);
			i++;
		}
	}
	else {
		for (AntennaMap::iterator iter = this->beginAntennaMap();
				iter != this->endAntennaMap();iter++) {
			StubAntenna *_pAntenna = (*iter).second;
			for (AntennaFoVMap::iterator iter = _pAntenna->beginAntennaFoV();
					iter != _pAntenna->endAntennaFoV();iter++) {
				tagVector->push_back((*iter).second);
			}
		}
	}
	return tagVector;
}

ELFIN::StubAntenna* ELFIN::StubReader::getAntenna(int __pAntennaID) {
	AntennaMap::iterator ant_iter = this->m_AntennaMap.find(__pAntennaID);
	if (ant_iter == this->m_AntennaMap.end()) {
		return NULL;
	} else {
		return (*ant_iter).second;
	}
}

ELFIN::StubGPIPort* ELFIN::StubReader::getGPIPort(int __pGPIPortNum) {
	GPIPortMap::iterator gpi_iter = this->m_GPIPortMap.find(__pGPIPortNum);
	if (gpi_iter == this->m_GPIPortMap.end()) {
		return NULL;
	} else {
		return (*gpi_iter).second;
	}
}

ELFIN::StubGPOPort* ELFIN::StubReader::getGPOPort(int __pGPOPortNum) {
	GPOPortMap::iterator gpo_iter = this->m_GPOPortMap.find(__pGPOPortNum);
	if (gpo_iter == this->m_GPOPortMap.end()) {
		return NULL;
	} else {
		return (*gpo_iter).second;
	}
}

/**
 * @param pAntennaID Antenna id to check
 * @param pEPCBank EPC bank of the tag to check
 * @warning If the matching tag exists, this method updates the tag with current information using StubAntenna::updateExistingStubTag()
 * @return If the antenna exists and the tag does not exists, returns TRUE. Otherwise, returns FALSE.
 */
bool ELFIN::StubReader::checkStubTagIsAddable(int pAntennaID, MemoryBank *pEPCBank, int pRSSI) {
	if (this->m_AntennaMap.find(pAntennaID) == this->m_AntennaMap.end()) {
		// If the antenna id does not exists, return false.
		return false;
	}
	else {
		if (m_AntennaMap[pAntennaID]->updateExistingStubTag(pEPCBank, pRSSI) == true) {
			// If the tag already exists in the antenna, update timestamp and return false.
			return false;
		}
		else {
			return true;
		}
	}
}

/**
 * @remark Because there is no error checking procedure,
 * need to use checkStubTagIsAddable() before using this method.
 */
int ELFIN::StubReader::inventoryTag(StubTag* pTag) {
	return m_AntennaMap[pTag->_pAntennaID]->addStubTag(pTag);
}

int ELFIN::StubReader::tagRead() {
	boost::recursive_mutex::scoped_lock lock(m_StubReaderLock);
	ELFIN::Utils::printCountedTime("Recv-TagReadStart");

	// If is emulator mode, then generate tags per each antenna.
	if (this->m_ProgOpt->getIsEmulatorMode() == 1) {
		for (TagVector::iterator tagIter = m_VirtualTagVector.begin();
				tagIter != m_VirtualTagVector.end(); tagIter++) {
			if (this->checkStubTagIsAddable((*tagIter)->_pAntennaID, (*tagIter)->_MB[1], (*tagIter)->_pPeakRSSI) == true) {
				this->inventoryTag((*tagIter)->clone());
			}
		}
		// Because tagRead loops before AISpec or ROSpec is finished,
		// give delay for emulator mode.
		boost::this_thread::sleep_for(boost::chrono::milliseconds(m_ReadCycle * 50 + 1));
	}

	else {
		// Add virtual tag to tagvector
		for (TagVector::iterator tagIter = m_VirtualTagVector.begin();
				tagIter != m_VirtualTagVector.end(); tagIter++) {
			if (this->checkStubTagIsAddable((*tagIter)->_pAntennaID, (*tagIter)->_MB[1], 0) == true) {
				this->inventoryTag((*tagIter)->clone());
			}
		}
		int readResult = 0;
		READER_LOG (LOGTYPE_DEBUG, "Starting tag_read of the abstract reader\n");
		readResult = _pAbstractReader->_startTagRead(m_ReadCycle);
		if (readResult != 0) {
			READER_LOG (LOGTYPE_ERROR, "tag_read returned error\n");
			return readResult;
		}
		//boost::recursive_mutex::scoped_lock abs_lock(_pAbstractReader->m_AbstractReaderLock.lock());
		//READER_LOG (LOGTYPE_DEBUG, "Finished tag_read. Found %d tags\n", _pAbstractReader->tagID_list.cnt);
		boost::this_thread::sleep_for(boost::chrono::milliseconds(m_ReadCycle * 50 + 1));
	}

	for (AntennaMap::iterator iter = m_AntennaMap.begin();
			iter != m_AntennaMap.end();iter++) {
		(*iter).second->clearUnsensedTagsInFoV();
	}
	ELFIN::Utils::printCountedTime("TagReadStart-TagReadStop");

	return 0;
}

int ELFIN::StubReader::readTagMemoryBank(
		uint16_t accessPassword, StubTag *pStubTag, uint16_t memoryBank,
		uint16_t startAddress, uint16_t dataLength) {
	int readResult;
	uint16_t epcLength = 12;
	char epc[12];
	for (unsigned int tag_i = 0; tag_i < 12; tag_i++) {
		epc[tag_i] = pStubTag->_MB[1]->at(4 + tag_i);
	}
	readResult = _pAbstractReader->_readTagMemory(accessPassword, epcLength,
				epc, memoryBank, startAddress, dataLength);
	if (readResult != 0) {
		READER_LOG (LOGTYPE_ERROR, "read_tag_memory returned error\n");
		return readResult;
	}
	READER_LOG (LOGTYPE_DEBUG, "Finished read_tag_memory of the real reader\n");
	/*
	pStubTag->_MB[memoryBank]->resize(_pAbstractReader->_pReceivedTagMemorySize);
	for (unsigned int i = 0;i < _pAbstractReader->_pReceivedTagMemorySize;i++) {
		pStubTag->_MB[memoryBank]->at(i) = _pAbstractReader->_pReceivedTagMemory[i];
	}
	if (memoryBank == 1) {
		pStubTag->setLogEPCStr();
	}
	*/
	return readResult;
}

int ELFIN::StubReader::writeTagMemoryBank(
		uint16_t accessPassword, StubTag *pStubTag, uint16_t memoryBank,
		uint16_t startAddress, LLRP::llrp_u16v_t dataToWrite) {
	int writeResult;
	uint16_t epcLength = 12;
	char epc[12];
	for (unsigned int tag_i = 0; tag_i < 12; tag_i++) {
		epc[tag_i] = pStubTag->_MB[1]->at(4 + tag_i);
	}
	char pDataToWrite[dataToWrite.m_nValue * 2];
	for (int i = 0;i < dataToWrite.m_nValue;i++) {
		int targetCharAddr = i * 2;
		pDataToWrite[targetCharAddr] = dataToWrite.m_pValue[i] / 0x100;
		pDataToWrite[targetCharAddr + 1] = dataToWrite.m_pValue[i] % 0x100;
	}
	writeResult = _pAbstractReader->_writeTagMemory(accessPassword, epcLength,
			epc, memoryBank, startAddress, (uint16_t) dataToWrite.m_nValue, pDataToWrite);
	if (writeResult != 0) {
		READER_LOG (LOGTYPE_ERROR, "write_tag_memory returned error\n");
		return writeResult;
	}
	READER_LOG (LOGTYPE_DEBUG, "Finished write_tag_memory of the real reader\n");
	/*
	pStubTag->_MB[memoryBank]->resize(_pAbstractReader->_pReceivedTagMemorySize);
	for (unsigned int i = 0;i < _pAbstractReader->_pReceivedTagMemorySize;i++) {
		pStubTag->_MB[memoryBank]->at(i) = _pAbstractReader->_pReceivedTagMemory[i];
	}
	*/
	return writeResult;
}

int ELFIN::StubReader::invokeGPIEventCallback(uint16_t GPIPortNum,
		int state) {
	// If we send GPIEvent to ROAdmin during the initialization of LLRPCore,
	// the segfault may happen. To avoid it, check whether it is connected to FnC.
	if (this->m_LLRPCore->isConnected() == 1)
		return m_GPIPortMap[GPIPortNum]->setState((enum LLRP::EGPIPortState) state);
	else
		return -1;
}

int ELFIN::StubReader::clearAntennaMap() {
	for (AntennaMap::iterator iter = m_AntennaMap.begin();
			iter != m_AntennaMap.end();iter++) {
		delete (*iter).second;
	}
	this->m_AntennaMap.clear();
	return 0;
}

int ELFIN::StubReader::clearGPIPortMap() {
	for (GPIPortMap::iterator iter = m_GPIPortMap.begin();
			iter != m_GPIPortMap.end();iter++) {
		delete (*iter).second;
	}
	this->m_GPIPortMap.clear();
	return 0;
}

int ELFIN::StubReader::clearGPOPortMap() {
	for (GPOPortMap::iterator iter = m_GPOPortMap.begin();
			iter != m_GPOPortMap.end();iter++) {
		delete (*iter).second;
	}
	this->m_GPOPortMap.clear();
	return 0;
}

int ELFIN::StubReader::addAntenna(StubAntenna* pAntenna) {
	if (this->m_AntennaMap[pAntenna->_pAntennaID] == NULL) {
		this->m_AntennaMap[pAntenna->_pAntennaID] = pAntenna;
		READER_LOG(LOGTYPE_INFO, "Antenna ID %d is added to the Reader\n", pAntenna->_pAntennaID);
		return 0;
	}
	else {
		READER_LOG(LOGTYPE_ERROR, "Antenna ID %d is already added to the Reader\n", pAntenna->_pAntennaID);
		return 1;
	}
}

int ELFIN::StubReader::addGPIPort(StubGPIPort* pGPIPort) {
	if (this->m_GPIPortMap[pGPIPort->getPortNum()] == NULL) {
		this->m_GPIPortMap[pGPIPort->getPortNum()] = pGPIPort;
		READER_LOG(LOGTYPE_INFO, "GPI Port ID %d is added to the Reader\n", pGPIPort->getPortNum());
		return 0;
	}
	else {
		READER_LOG(LOGTYPE_ERROR, "GPI Port ID %d is already added to the Reader\n", pGPIPort->getPortNum());
		return 1;
	}
}

int ELFIN::StubReader::addGPOPort(StubGPOPort* pGPOPort) {
	if (this->m_GPOPortMap[pGPOPort->getPortNum()] == NULL) {
		this->m_GPOPortMap[pGPOPort->getPortNum()] = pGPOPort;
		READER_LOG(LOGTYPE_INFO, "GPO Port ID %d is added to the Reader\n", pGPOPort->getPortNum());
		return 0;
	}
	else {
		READER_LOG(LOGTYPE_ERROR, "GPO Port ID %d is already added to the Reader\n", pGPOPort->getPortNum());
		return 1;
	}
}

int ELFIN::StubReader::removeAntenna(LLRP::llrp_u16_t pAntennaID) {
	if (this->m_AntennaMap[pAntennaID] != NULL) {
		delete this->m_AntennaMap[pAntennaID];
		m_AntennaMap.erase(pAntennaID);
		READER_LOG(LOGTYPE_INFO, "Antenna ID %d is removed from the Reader\n", pAntennaID);
		return 0;
	}
	else {
		READER_LOG(LOGTYPE_ERROR, "Antenna ID %d is not found from the Reader\n", pAntennaID);
		return 1;
	}
}

int ELFIN::StubReader::removeGPIPort(LLRP::llrp_u16_t GPIPortNum) {
	if (this->m_AntennaMap[GPIPortNum] != NULL) {
		delete this->m_AntennaMap[GPIPortNum];
		m_AntennaMap.erase(GPIPortNum);
		READER_LOG(LOGTYPE_INFO, "GPI Port ID %d is removed from the Reader\n", GPIPortNum);
		return 0;
	}
	else {
		READER_LOG(LOGTYPE_ERROR, "GPI Port ID %d is not found from the Reader\n", GPIPortNum);
		return 1;
	}
}

int ELFIN::StubReader::removeGPOPort(LLRP::llrp_u16_t GPOPortNum) {
	if (this->m_AntennaMap[GPOPortNum] != NULL) {
		delete this->m_AntennaMap[GPOPortNum];
		m_AntennaMap.erase(GPOPortNum);
		READER_LOG(LOGTYPE_INFO, "GPO Port ID %d is removed from the Reader\n", GPOPortNum);
		return 0;
	}
	else {
		READER_LOG(LOGTYPE_ERROR, "GPO Port ID %d is not found from the Reader\n", GPOPortNum);
		return 1;
	}
}


ELFIN::AntennaMap::iterator ELFIN::StubReader::beginAntennaMap() {
	return this->m_AntennaMap.begin();
}

ELFIN::GPIPortMap::iterator ELFIN::StubReader::beginGPIPortMap() {
	return this->m_GPIPortMap.begin();
}

ELFIN::GPOPortMap::iterator ELFIN::StubReader::beginGPOPortMap() {
	return this->m_GPOPortMap.begin();
}

ELFIN::AntennaMap::iterator ELFIN::StubReader::endAntennaMap() {
	return this->m_AntennaMap.end();
}

ELFIN::GPIPortMap::iterator ELFIN::StubReader::endGPIPortMap() {
	return this->m_GPIPortMap.end();
}

ELFIN::GPOPortMap::iterator ELFIN::StubReader::endGPOPortMap() {
	return this->m_GPOPortMap.end();
}

int ELFIN::StubReader::countAntennaMap() {
	return this->m_AntennaMap.size();
}

int ELFIN::StubReader::countGPIPortMap() {
	return this->m_GPIPortMap.size();
}

int ELFIN::StubReader::countGPOPortMap() {
	return this->m_GPOPortMap.size();
}

int ELFIN::StubReader::connectStubApp(StubReaderApp* __pStubApp) {
	if (this->m_StubApp != NULL) {
		if (this->m_StubApp->_GPICallBack != NULL) {
			__pStubApp->_GPICallBack = this->m_StubApp->_GPICallBack;
		}
		delete this->m_StubApp;
	}
	this->m_StubApp = __pStubApp;
	__pStubApp->_pReader = this;
	return 0;
}

int ELFIN::StubReader::getStubAppIsEmulator() {
	return this->m_StubApp->_isEmulator;
}
