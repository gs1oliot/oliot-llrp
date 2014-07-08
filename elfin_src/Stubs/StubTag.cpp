/**
 * @file    StubTag.cpp
 * @brief
 */

#include <stdlib.h>
#include "StubTag.h"
#include "../ELFIN_Platform.h"

// Just create tag and allocate MB. This is for real use.
ELFIN::StubTag::StubTag(int __pAntennaID)
: _pAntennaID(__pAntennaID), _pFirstSeenTimestamp(0),
  _pLastSeenTimestamp(0), _pTagSeenCount(1),
  _pPeakRSSI(0), _pChannelIndex(0), isUpdated(true) {
	this->init();
}

int ELFIN::StubTag::setEPC(const char* __epcStr) {
	_MB[0]->resize(4, 0x00); // 32-bit Kill Password, 32-bit Access Password
	_MB[1]->resize(16, 0x00); // 16-bit StoredCRC, 16-bit StoredPC, 96-bit EPC
	_MB[2]->resize(4, 0xCC); // 12-bit Tag-mask, 12-bit Tag model number, etc
	_MB[2]->at(0) = 0xE2; // 8-bit ISO/IEC 15963 class-identifier value (0xE0 or 0xE2)
	_MB[3]->resize(16, 0xAB); // 256-bit user memory
	_MB[3]->at(0) = 0x12;
	_MB[3]->at(1) = 0x34;
	_MB[3]->at(2) = 0x56;
	_MB[3]->at(3) = 0x78;

	int i;
	// Assign EPC number with given EPC string
	// (Note: The format of EPC is ignored. Just for test.)
	this->_MB[1]->at(0) = 0x00; // fake CRC
	this->_MB[1]->at(1) = 0xE2; // fake CRC
	this->_MB[1]->at(2) = 0x00; // fake PC
	this->_MB[1]->at(3) = 0xBC; // fake PC
	unsigned char epcStr[24];
	memcpy(epcStr, __epcStr, sizeof(epcStr));

	for (i = 4;i < 16;i++) {
		this->_MB[1]->at(i) = (LLRP::llrp_u8_t)(
				hatoi(epcStr[(i - 4) * 2]) * 0x10 +
				hatoi(epcStr[(i - 4) * 2 + 1]) );
	}

	this->setLogEPCStr();

	return 0;
}

int ELFIN::StubTag::setRandEPC(int seed) {
	_MB[0]->resize(4, 0x00); // 32-bit Kill Password, 32-bit Access Password
	_MB[1]->resize(16, 0x00); // 16-bit StoredCRC, 16-bit StoredPC, 96-bit EPC
	_MB[2]->resize(4, 0xCC); // 12-bit Tag-mask, 12-bit Tag model number, etc
	_MB[2]->at(0) = 0xE2; // 8-bit ISO/IEC 15963 class-identifier value (0xE0 or 0xE2)
	_MB[3]->resize(16, 0xAB); // 256-bit user memory
	_MB[3]->at(0) = 0x12;
	_MB[3]->at(1) = 0x34;
	_MB[3]->at(2) = 0x56;
	_MB[3]->at(3) = 0x78;

	int i;
	srand(seed);
	// Assign EPC number randomly
	// (Note: The format of EPC is ignored. Just for test.)
	this->_MB[1]->at(0) = 0xAC; // fake CRC
	this->_MB[1]->at(1) = 0xBC; // fake CRC
	this->_MB[1]->at(2) = 0xCC; // fake PC
	this->_MB[1]->at(3) = 0xDC; // fake PC
	this->_MB[1]->at(4) = 0x35; //GID
	for (i = 5;i < 16;i++) {
		this->_MB[1]->at(i) = ((LLRP::llrp_u8_t)rand() % 0xFF);
	}

	this->setLogEPCStr();

	return 0;
}

ELFIN::StubTag::~StubTag()
{
	READER_LOG (LOGTYPE_TRACE, "Destroying StubTag...\n");
	for (int i = 0;i < 4;i++) {
		if (this->_MB[i] != NULL)
			delete this->_MB[i];
	}
}


// This is supposed to create a backup of original StubTag.
ELFIN::StubTag* ELFIN::StubTag::clone() {
	StubTag *pStubTag = new StubTag(_pAntennaID);
	for (int i = 0;i < 4;i++) {
		delete pStubTag->_MB[i];
		pStubTag->_MB[i] = new MemoryBank(this->_MB[i]->begin(), this->_MB[i]->end());
	}
	pStubTag->setLogEPCStr();
	pStubTag->_pFirstSeenTimestamp-= this->_pFirstSeenTimestamp;
	pStubTag->_pLastSeenTimestamp = this->_pLastSeenTimestamp;
	pStubTag->_pTagSeenCount = this->_pTagSeenCount;
	pStubTag->_pPeakRSSI = this->_pPeakRSSI;
	pStubTag->_pChannelIndex = this->_pChannelIndex;
	pStubTag->isUpdated = this->isUpdated;
	return pStubTag;
}

void ELFIN::StubTag::setLogEPCStr() {
	unsigned char hexchars[] = { '0', '1', '2', '3', '4',
			'5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	for (unsigned int i = 0; i < 12; i++) {
		// Start from 2nd memory space. 0~1 spaces are for CRC bits,
		// but cannot be obtained from PR9200 using auto_read2 API, so leave them as 0x00.
		int ch = this->_MB[1]->at(i + 4);
		_logEpcStr[i * 2] = hexchars[(ch & 0xf0) >> 4];
		_logEpcStr[i * 2 + 1] = hexchars[ch & 0x0f];
	}
	_logEpcStr[24] = '\0';
}


unsigned char ELFIN::StubTag::hatoi(unsigned char a) {
	if (a >= '0' && a <= '9')
		return a - '0';
	if (a >= 'a' && a <= 'f')
		return a - 'a' + 10;
	if (a >= 'A' && a <= 'F')
		return a - 'A' + 10;
	return 0;
}

int ELFIN::StubTag::init() {
	for (int i = 0;i < 4;i++) {
		this->_MB[i] = new MemoryBank();
	}
	_pFirstSeenTimestamp = Utils::getCurrentTimeMilliseconds() * 1000;
	_pLastSeenTimestamp = Utils::getCurrentTimeMilliseconds() * 1000;
	_pTagSeenCount = 0;

	return 0;
}

