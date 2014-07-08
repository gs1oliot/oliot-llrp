/**
 * @file    StubAntenna.h
 * @brief
 */

#ifndef __LLRP_READER__STUBANTENNA_H__
#define __LLRP_READER__STUBANTENNA_H__
#include <vector>
#include "StubReader.h"
#include "StubTag.h"

namespace ELFIN
{
class StubReader;
class StubAntenna;
}

namespace ELFIN
{
/** @class StubAntenna
 * @brief Abstraction for the Antenna.
 */
class StubAntenna
{
public:
	/// Constructor of StubAntenna class
	StubAntenna(StubReader *__physicalReader, LLRP::llrp_u16_t __pAntennaID,
			LLRP::llrp_u16_t __pAntennaConnected, LLRP::llrp_u16_t __pAntennaGain);
	/// Destructor of StubAntenna class
	~StubAntenna();
	StubReader *_pReader;
	/// @todo  Add getter and setter of these variables and make these private.
	LLRP::llrp_u16_t _pAntennaID;
	LLRP::llrp_u16_t _pAntennaConnected;
	LLRP::llrp_u16_t _pAntennaGain;
	LLRP::llrp_u16_t _ReceiverSensitivity;
	LLRP::llrp_u16_t _ChannelIndex;
	LLRP::llrp_u16_t _HopTableID;
	LLRP::llrp_u16_t _pTransmitPower;

private:
	/// StubTag in the FoV of antenna are stored in this map.
	AntennaFoVMap _AntennaFoV;

public:
	/// Delete all StubTag in the FoV of this antenna
	int clearAntennaFoV();
	/// Delete all StubTag which was not sensed this time, by checking isUpdated variable of each StubTag.
	int clearUnsensedTagsInFoV();
	/// Executes updateExistingStubTag(). If the tag does not exist, add it.
	int addStubTag(StubTag *pTag);
	/// Matches the existing tag with the EPC bank contents. If exists, update the information of the tag and return TRUE. If not, return FALSE.
	bool updateExistingStubTag(MemoryBank *pEPCBank, int pRSSI);
	/// Remove StubTag from the Antenna FoV, if exists.
	int removeStubTag(StubTag *pTag);

	/**@{*/
	/// Iterator access method
	AntennaFoVMap::iterator beginAntennaFoV();
	AntennaFoVMap::iterator endAntennaFoV();
	int countAntennaFoV();
	/**@}*/

	/// Get transmit power of the RFID reader, update _pTransmitPower, and return it. If LLRP Wrapper is in emulator mode, just return _pTransmitPower.
	int getTransmitPower();
	/// Set transmit power of the RFID reader, update _pTransmitPower, and return result. If LLRP Wrapper is in emulator mode, just update _pTransmitPower.
	int setTransmitPower(LLRP::llrp_u16_t pTransmitPower);
};
}


#endif /* __LLRP_READER__STUBANTENNA_H__ */
