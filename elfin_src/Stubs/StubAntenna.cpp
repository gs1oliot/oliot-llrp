/**
 * @file    StubAntenna.cpp
 * @brief
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "StubAntenna.h"

ELFIN::StubAntenna::StubAntenna(StubReader *__pReader, LLRP::llrp_u16_t __pAntennaID, LLRP::llrp_u16_t __pAntennaConnected, LLRP::llrp_u16_t __pAntennaGain)
:_pReader(__pReader), _pAntennaID(__pAntennaID), _pAntennaConnected(__pAntennaConnected), _pAntennaGain(__pAntennaGain), _ReceiverSensitivity(0), _ChannelIndex(0), _HopTableID(0), _pTransmitPower(0) {

}

ELFIN::StubAntenna::~StubAntenna() {
	READER_LOG (LOGTYPE_TRACE, "Destroying StubAntenna...\n");
	this->clearAntennaFoV();
}

int ELFIN::StubAntenna::clearAntennaFoV() {
	for (AntennaFoVMap::iterator iter = this->_AntennaFoV.begin();
			iter != this->_AntennaFoV.end();iter++) {
		delete (*iter).second;
	}
	this->_AntennaFoV.clear();
	return 0;
}

int ELFIN::StubAntenna::clearUnsensedTagsInFoV() {
	for (AntennaFoVMap::iterator iter = this->_AntennaFoV.begin();
			iter != this->_AntennaFoV.end();iter++) {
		if ((*iter).second->isUpdated == false) {
			delete (*iter).second;
			this->_AntennaFoV.erase(iter);
		}
		else {
			(*iter).second->isUpdated = false;
		}
	}
	return 0;
}

/**
 * @remark This method executes updateExistingStubTag() to check whether the tag exists or not.\n
 * So if the tag already exists, the information of the tag is updated.
 */
int ELFIN::StubAntenna::addStubTag(StubTag* pTag) {
	if (this->updateExistingStubTag(pTag->_MB[1], pTag->_pPeakRSSI) == false) {
		this->_AntennaFoV[*(pTag->_MB[1])] = pTag;
		READER_LOG(LOGTYPE_TRACE, "RFID Tag(%s) is added to the Antenna ID %d\n",
				pTag->_logEpcStr, _pAntennaID);
		return 0;
	}
	else {
		READER_LOG(LOGTYPE_ERROR, "RFID Tag(%s) is already added to the Antenna ID %d\n",
				pTag->_logEpcStr, _pAntennaID);
		return 1;
	}
}

bool ELFIN::StubAntenna::updateExistingStubTag(MemoryBank *pEPCBank, int pRSSI) {
	AntennaFoVMap::iterator iter = this->_AntennaFoV.find(*pEPCBank);
	if (iter == this->_AntennaFoV.end()) {
		return false;
	}
	else {
		StubTag *pTag = (*iter).second;
		pTag->_pLastSeenTimestamp = Utils::getCurrentTimeMilliseconds() * 1000;
		pTag->_pTagSeenCount++;
		if (pRSSI > pTag->_pTagSeenCount) {
			pTag->_pPeakRSSI = pRSSI;
		}
		pTag->isUpdated = true;
		return true;
	}
}

int ELFIN::StubAntenna::removeStubTag(StubTag* pTag) {
	if (this->_AntennaFoV[*(pTag->_MB[1])] != NULL) {
		delete this->_AntennaFoV[*(pTag->_MB[1])];
		_AntennaFoV.erase(*(pTag->_MB[1]));
		READER_LOG(LOGTYPE_TRACE, "RFID Tag(%s) is removed from the Antenna ID %d\n",
				pTag->_logEpcStr, _pAntennaID);
		return 0;
	}
	else {
		READER_LOG(LOGTYPE_ERROR, "RFID Tag(%s) is not found from the Antenna ID %d\n",
				pTag->_logEpcStr, _pAntennaID);
		return 1;
	}
}

ELFIN::AntennaFoVMap::iterator ELFIN::StubAntenna::beginAntennaFoV() {
	return _AntennaFoV.begin();
}

ELFIN::AntennaFoVMap::iterator ELFIN::StubAntenna::endAntennaFoV() {
	return _AntennaFoV.end();
}

int ELFIN::StubAntenna::countAntennaFoV() {
	return _AntennaFoV.size();
}

int ELFIN::StubAntenna::getTransmitPower() {
	if (_pReader->m_ProgOpt->getIsEmulatorMode() == 0) {
#ifdef ENABLE_PR9200
		float pwr = _pReader->reader_getTransmitPower();
		if (pwr == -1)
			this->_pTransmitPower = -1;
		else
			this->_pTransmitPower = pwr * 100;
#endif
	}
	return this->_pTransmitPower;
}

int ELFIN::StubAntenna::setTransmitPower(LLRP::llrp_u16_t pTransmitPower) {
	if (_pReader->m_ProgOpt->getIsEmulatorMode() == 0) {
#ifdef ENABLE_PR9200
		float pwr = pTransmitPower / 100;
		int result = _pReader->reader_setTransmitPower(pwr);
		if (result == -1)
			this->_pTransmitPower = -1;
		else
			this->_pTransmitPower = pTransmitPower;
#endif
	}
	else {
		this->_pTransmitPower = pTransmitPower;
	}
	return this->_pTransmitPower;
}
