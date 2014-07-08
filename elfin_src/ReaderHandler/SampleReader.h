/*
 * SampleReader.h
 *
 *  Created on: Apr 4, 2014
 *      Author: iot-team
 */

#ifndef SAMPLEREADER_H_
#define SAMPLEREADER_H_

#include "AbstractReader.h"

namespace ELFIN {
class SampleReader : public AbstractReader {
public:
	SampleReader(StubReader *a_StubReader);
	~SampleReader();
	const static int EMUL_ANT_COUNT = 4;
	const static int EMUL_TAG_COUNT = 10;
	typedef struct {
		uint16_t MB_0[4]; // 32-bit Kill Password, 32-bit Access Password
		uint16_t MB_1[16]; // 16-bit StoredCRC, 16-bit StoredPC, 96-bit EPC
		uint16_t MB_2[4]; // 12-bit Tag-mask, 12-bit Tag model number, etc
		uint16_t MB_3[16]; // 256-bit user memory
	} tag;
	typedef struct {
		int ant_id;
		tag ant_fov[EMUL_TAG_COUNT]; // antennaID should start from 1
	} antenna;
	typedef struct {
		antenna antennas[EMUL_ANT_COUNT + 1];
	} reader;

	reader pReader;
	boost::thread readerThread;
	int reader_run();

	int init();
	ELFIN::TagVector *getCurrentTags(bool RSSIEnabled);
	int startTagRead(int readCycle);
	int stopTagRead();
	int readTagMemory(
			uint16_t accessPassword,
			uint16_t epcLength,
			char *epc,
			uint16_t memoryBank,
			uint16_t startAddress,
			uint16_t dataLength);
	int writeTagMemory(
			uint16_t accessPassword,
			uint16_t epcLength,
			char *epc,
			uint16_t memoryBank,
			uint16_t startAddress,
			uint16_t dataLength,
			char *dataToWrite);
};
}



#endif /* SAMPLEREADER_H_ */
