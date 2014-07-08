/**
 * @file    StubReader.h
 * @brief
 */

#ifndef __LLRP_READER__PHYSICALREADER_H__
#define __LLRP_READER__PHYSICALREADER_H__

#include <boost/unordered_map.hpp>
#include "../LLRPCore.h"
#include "StubAntenna.h"
#include "StubGPIPort.h"
#include "StubGPOPort.h"
#include "StubApp.h"
#include "StubTag.h"
#include "../ELFIN_Platform.h"

namespace ELFIN
{
class StubGPIPort;
class StubGPOPort;
class StubAntenna;
class StubReaderApp;
class StubTag;
class StubReader;
class CConnectionFnCMgr;
}

namespace ELFIN
{
/** @class StubReader
 * @brief Abstraction for physical RFID reader
 * @remark The StubReader is designed to run on single thread.
 * So every methods that utilize StubReader object should gain lock of _pReaderLock.
 */
class StubReader
{
public:
	/// Constructor of StubReader class
	StubReader(LLRPCore *__pLLRPCore);
	/// Destructor of StubReader class
	~StubReader();
	/// With given antenna id, creates vector of tags in the antenna and returns the pointer to vector.
	std::vector<StubTag *> *getCurrentTags(int __pAntennaID);
	/// Start of tag singulation. Perform tagRead() and return the result.
	int StartTagSingulation(int __pAntennaID);
	/// Finish of tag singulation. But actually, this method does nothing.
	void FinishTagSingulation();
	/// Interrupt tag singulation. If LLRP Wrapper is in wrapper mode, let the RFID reader to stop tag read. If not, does nothing.
	void InterruptTagSingulation();
	/// Let the RFID reader to read tag memory bank. After the reading is finished, result is stored in pStubTag
	int readTagMemoryBank(
			uint16_t accessPassword, StubTag *pStubTag, uint16_t memoryBank,
			uint16_t startAddress, uint16_t dataLength);
	/// Let the RFID reader to write tag memory bank. After the writing is finished, result is stored in pStubTag
	int writeTagMemoryBank(
			uint16_t accessPassword, StubTag *pStubTag, uint16_t memoryBank,
			uint16_t startAddress, LLRP::llrp_u16v_t dataToWrite);
	/// Let the RFID reader to start tag read operation, and store the tags to each StubAntenna, in format of StubTag
	int tagRead();

public:
	LLRPCore *m_LLRPCore;
	ProgramOptions *m_ProgOpt;
	AntennaMap m_AntennaMap;
	GPIPortMap m_GPIPortMap;
	GPOPortMap m_GPOPortMap;
	TagVector m_VirtualTagVector;
	StubReaderApp *m_StubApp;

public:
	boost::recursive_mutex m_StubReaderLock;
	LLRP::llrp_u96_t *m_ReaderEPC;
	int m_NumberOfStubTagsPerAntenna;
	int m_ReadCycle;

public:
	/// Checks whether the antenna of given id exists, and whether the tag of given EPC exists in that antenna.
	bool checkStubTagIsAddable(int pAntennaID, MemoryBank *pEPCBank, int pRSSI);
	/// Add the given StubTag to the antenna. Antenna id should be defined in the StubTag object.
	int inventoryTag(StubTag *pTag);
	/// This callback is registered to StubReaderApp, so StubReaderApp can send GPI event to StubReader.
	int invokeGPIEventCallback(uint16_t GPIPortNum, int state);
	/// Destroy currently connected StubReaderApp, and connect the StubReaderApp(or the class which inherits StubReaderApp) to StubReader
	int connectStubApp(StubReaderApp *__pStubApp);
	/// Check whether the current StubReaderApp is just stub one, or the inherited reader app which executed LLRP Wrapper.
	int getStubAppIsEmulator();

public:
	/// Get the transmit power from RFID reader. This should be used in wrapper mode.
	float reader_getTransmitPower();
	/// Set the transmit power of RFID reader. This should be used in wrapper mode.
	int reader_setTransmitPower(float pTransmitPower);

public:
	/// Find StubAntenna object with given antenna id and return it. If it does not exist, return NULL.
	StubAntenna *getAntenna(int __pAntennaID);
	/// Find StubGPIPort object with given GPI port number and return it. If it does not exist, return NULL.
	StubGPIPort *getGPIPort(int __pGPIPortNum);
	/// Find StubGPOPort object with given GPO port number and return it. If it does not exist, return NULL.
	StubGPOPort *getGPOPort(int __pGPOPortNum);
	/**@{*/
	/// Iterator access method
	AntennaMap::iterator beginAntennaMap();
	GPIPortMap::iterator beginGPIPortMap();
	GPOPortMap::iterator beginGPOPortMap();
	AntennaMap::iterator endAntennaMap();
	GPIPortMap::iterator endGPIPortMap();
	GPOPortMap::iterator endGPOPortMap();
	int countAntennaMap();
	int countGPIPortMap();
	int countGPOPortMap();
	/**@}*/

private:
	int clearAntennaMap();
	int clearGPIPortMap();
	int clearGPOPortMap();
	int addAntenna(StubAntenna *pAntenna);
	int addGPIPort(StubGPIPort *pGPIPort);
	int addGPOPort(StubGPOPort *pGPOPort);
	int removeAntenna(LLRP::llrp_u16_t pAntennaID);
	int removeGPIPort(LLRP::llrp_u16_t GPIPortNum);
	int removeGPOPort(LLRP::llrp_u16_t GPOPortNum);

	/// Method for converting single HEX char to integer
	unsigned char hatoi(unsigned char a);
	/// Method for converting single byte HEX string to integer
	LLRP::llrp_u8_t ha2i_str(const char *str);


public: // Added for testing non-polling method
	void tagDataCallback(StubTag *pTag);
};
}

#endif /* __LLRP_READER__PHYSICALREADER_H__ */
