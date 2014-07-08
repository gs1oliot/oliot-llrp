/**
 * @file    StubGPOPort.h
 * @brief
 */

#ifndef __LLRP_READER__STUBGPOPORT_H__
#define __LLRP_READER__STUBGPOPORT_H__
#include "StubReader.h"
#include "../ELFIN_Platform.h"

namespace ELFIN
{
class StubReader;
class StubGPOPort;
}

namespace ELFIN
{
/** @class StubGPOPort
 * @brief Abstraction for the GPO Port.
 */
class StubGPOPort
{
public:
	/// Constructor of StubGPOPort class
	StubGPOPort(StubReader *__pReader, LLRP::llrp_u16_t __GPOPortNum, LLRP::llrp_u1_t __data);
	/// Returns port number of this port
	LLRP::llrp_u16_t getPortNum();
	/// Returns current data
	LLRP::llrp_u1_t getData();
	/// Set data and send GPO event to Reader App
	int setData(LLRP::llrp_u1_t _data);

private:
	StubReader *m_Reader;
	LLRP::llrp_u16_t m_GPOPortNum;
	/// GPI HIGH/LOW/unknown data.
	LLRP::llrp_u1_t m_data;
};
}

#endif /* __LLRP_READER__STUBGPOPORT_H__ */
