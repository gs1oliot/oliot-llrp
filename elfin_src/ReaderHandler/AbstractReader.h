/*
 * AbstractReader.h
 *
 *  Created on: Apr 4, 2014
 *      Author: iot-team
 */

#ifndef ABSTRACTREADER_H_
#define ABSTRACTREADER_H_

#include "../ELFIN_Platform.h"
#include "../Stubs/StubReader.h"

namespace ELFIN {
class AbstractReader {
protected:
	StubReader *m_StubReader;
public:
	boost::recursive_mutex m_AbstractReaderLock;
	int m_ReadCommandCount;

	AbstractReader(StubReader *a_StubReader)
	: m_StubReader(a_StubReader), m_ReadCommandCount(0) {};
	virtual ~AbstractReader() {};
	virtual ELFIN::TagVector *getCurrentTags(bool RSSIEnabled) = 0;

	// Need to override in actual reader implementation
	// startTagRead should be blocked until the current read is finished
	int _startTagRead(int readCycle) {
		if (this->m_ReadCommandCount == 0) {
			READER_LOG (LOGTYPE_TRACE, "Starting read from abstract reader\n");
			this->startTagRead(10);
			// Send start read command in actual implementation
		}
		// Increase m_ReadCommandCount to store the number of ROSpecs currently
		// using this AbstractReader (similar to read lock)
		this->m_ReadCommandCount++;
		return 0; }
	int _stopTagRead() {
		this->m_ReadCommandCount--;
		if (this->m_ReadCommandCount == 0) {
			// Send stop read command in actual implementation
		} else if (this->m_ReadCommandCount < 0) {
			return -1;
		}
		return 0; }
	int _readTagMemory(
			uint16_t accessPassword,
			uint16_t epcLength,
			char *epc,
			uint16_t memoryBank,
			uint16_t startAddress,
			uint16_t dataLength) {return 0;}
	int _writeTagMemory(
			uint16_t accessPassword,
			uint16_t epcLength,
			char *epc,
			uint16_t memoryBank,
			uint16_t startAddress,
			uint16_t dataLength,
			char *dataToWrite) {return 0;}

	// Not used currently, so returns -1 when called.
	int _lockTagMemory() {return -1;}
	int _killTagMemory() {return -1;}

	// TODO: Add settings for antenna configurations.

	boost::function<int(StubReader*, StubTag *)> m_tagCallBack;
	int regTagCallback(boost::function<int(StubReader*, StubTag *)> pTagCallBack) {
		m_tagCallBack = pTagCallBack;
		return 0;
	}
	// Pure virtual methods
	virtual int init() = 0;
	virtual int startTagRead(int readCycle) = 0;
	virtual int stopTagRead() = 0;
	virtual int readTagMemory(
			uint16_t accessPassword,
			uint16_t epcLength,
			char *epc,
			uint16_t memoryBank,
			uint16_t startAddress,
			uint16_t dataLength) = 0;
	virtual int writeTagMemory(
			uint16_t accessPassword,
			uint16_t epcLength,
			char *epc,
			uint16_t memoryBank,
			uint16_t startAddress,
			uint16_t dataLength,
			char *dataToWrite) = 0;
};
}

#endif /* ABSTRACTREADER_H_ */
