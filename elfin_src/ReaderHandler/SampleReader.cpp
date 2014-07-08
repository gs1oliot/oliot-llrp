/*
 * SampleReader.cpp
 *
 *  Created on: Apr 4, 2014
 *      Author: iot-team
 */

#include "SampleReader.h"

ELFIN::SampleReader::SampleReader(StubReader *a_StubReader)
: AbstractReader(a_StubReader) {
}

ELFIN::SampleReader::~SampleReader() {
}

int ELFIN::SampleReader::init() {
	// Initialize emulator reader data structure
	memset(&pReader, 0, sizeof (reader));
	for (int i = 1;i < EMUL_ANT_COUNT + 1;i++) {
		pReader.antennas[i].ant_id = i;
		for (int j = 0;j < EMUL_TAG_COUNT;j++) {
			pReader.antennas[i].ant_fov[j].MB_1[0] = 0xCC; // Fake CRC
			pReader.antennas[i].ant_fov[j].MB_1[2] = 0xBC; // Fake PC
			pReader.antennas[i].ant_fov[j].MB_1[4] = 0x35; // Heading of EPC
			pReader.antennas[i].ant_fov[j].MB_1[15] = i * EMUL_ANT_COUNT * EMUL_TAG_COUNT + j;
		}
	}
	return 0;
}

ELFIN::TagVector* ELFIN::SampleReader::getCurrentTags(
		bool RSSIEnabled) {
	return NULL;
}

int ELFIN::SampleReader::startTagRead(int readCycle) {
	readerThread = boost::thread(boost::bind(&ELFIN::SampleReader::reader_run, this));
	boost::this_thread::sleep_for(boost::chrono::milliseconds(4000));
	return 0;
}

int ELFIN::SampleReader::reader_run() {
	for (int antID = 1;antID < EMUL_ANT_COUNT + 1;antID++) {
		for (int tagNum = 0;tagNum < EMUL_TAG_COUNT;tagNum++) {
			StubTag *pTag = new StubTag(antID);
			// 16-bit StoredCRC, 16-bit StoredPC, 96-bit EPC
			pTag->_MB[1]->resize(16, 0x00);
			for (int mb_i = 0;mb_i < 16;mb_i++) {
				pTag->_MB[1]->at(mb_i) = pReader.antennas[antID].ant_fov[tagNum].MB_1[mb_i];
			}
			pTag->setLogEPCStr();
			READER_LOG (LOGTYPE_TRACE, "AntID: %d, TagCallback: %s\n", antID, pTag->_logEpcStr);
			m_tagCallBack(m_StubReader, pTag);
			boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
		}
	}
	return 0;
}

int ELFIN::SampleReader::stopTagRead() {
	return 0;
}

int ELFIN::SampleReader::readTagMemory(
		uint16_t accessPassword,
		uint16_t epcLength,
		char *epc,
		uint16_t memoryBank,
		uint16_t startAddress,
		uint16_t dataLength) {
	return 0;
}

int ELFIN::SampleReader::writeTagMemory(
		uint16_t accessPassword,
		uint16_t epcLength,
		char *epc,
		uint16_t memoryBank,
		uint16_t startAddress,
		uint16_t dataLength,
		char *dataToWrite) {
	return 0;
}




