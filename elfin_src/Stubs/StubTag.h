/**
 * @file    StubTag.h
 * @brief
 */

#ifndef __LLRP_READER__RFIDTAG_H__
#define __LLRP_READER__RFIDTAG_H__
#include <vector>
#include <boost/unordered_map.hpp>
#include "../ELFIN_Defs.h"

typedef unsigned char byte;

namespace ELFIN
{
	class StubTag;
}

namespace ELFIN
{
	/** @class StubTag
	 *  @brief Abstraction for the RFID Tag.
	 */
	class StubTag
	{
	public:
		/// Constructor of StubTag class. EPCID is not predefined. This is for real use.
		StubTag(int __pAntennaID);
		/// Constructor of StubTag class. EPCID is predefined with given EPC string. This is for virtual tags.
		int setEPC(const char *__epcStr);
		/// Constructor of StubTag class. EPCID is predefined randomly, with given seed. This is for virtual tags.
		int setRandEPC(int seed);
		/// Destructor of StubTag class.
		~StubTag();
		/// Returns clone of this StubTag object.
		StubTag *clone();
		/// With the contents of EPC bank, make EPC string and store to _logEpcStr. This string is usually used for printing log.
		void setLogEPCStr();

		int _pAntennaID;
		/// Memory banks of the RFID tag.
		boost::unordered_map<int, MemoryBank* > _MB;
		/// The EPC string converted by setLogEPCStr(). This is usually used for printing log.
		unsigned char _logEpcStr[25];
		/// First seen timestamp. This is set when this StubTag object is firstly instantiated.
		uint64_t _pFirstSeenTimestamp;
		/// Last seen timestamp. This is updated on every round of tag singulation.
		uint64_t _pLastSeenTimestamp;
		/// This variable is increased by 1 for each round of tag singulation.
		int _pTagSeenCount;
		int8_t _pPeakRSSI;
		int _pChannelIndex;
		/**
		 * @brief For every round of tag singulation, LLRP Wrapper
		 * checks whether the singulated tag exists in the StubAntenna.\n
		 * If exists, just update information and set this variable to true.
		 */
		bool isUpdated;

	private:
		/// Initialize StubTag by allocating MemoryBanks and several variables.
		int init();
		unsigned char hatoi(unsigned char a);
	};
}


#endif /* __LLRP_READER__RFIDTAG_H__ */
