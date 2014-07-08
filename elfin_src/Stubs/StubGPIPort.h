/**
 * @file    StubGPIPort.h
 * @brief
 */

#ifndef __LLRP_READER__STUBGPIPORT_H__
#define __LLRP_READER__STUBGPIPORT_H__
#include "StubReader.h"
#include "../ELFIN_Platform.h"

namespace ELFIN
{
class StubReader;
class StubAntenna;
}

namespace ELFIN
{
/** @class StubGPIPort
 * @brief Abstraction for the GPI Port.
 */
class StubGPIPort
{
public:
	/// Constructor of StubGPIPort class
	StubGPIPort(StubReader *__pReader, LLRP::llrp_u16_t __GPIPortNum,
			LLRP::llrp_u1_t __config, enum LLRP::EGPIPortState __state);
	/// Set the current state of GPI port. This method also delivers GPI event to ROAdmin.
	int setState(enum LLRP::EGPIPortState state);
	/// Returns port number of this GPI port
	LLRP::llrp_u16_t getPortNum();
	/// Returns current configuration of this GPI port
	LLRP::llrp_u1_t getConfig();
	/// Set the configuration of this GPI port
	int setConfig(LLRP::llrp_u1_t _config);
	/// Returns current state of this GPI port
	enum LLRP::EGPIPortState getState();

private:
	StubReader *m_Reader;
	LLRP::llrp_u16_t m_PortNum;
	/// State of GPI port. 0 means this GPI port is disabled, 1 is enabled.
	LLRP::llrp_u1_t m_config;
	/// GPI HIGH/LOW/unknown state.
	enum LLRP::EGPIPortState m_state;

};
}

#endif /* __LLRP_READER__STUBGPIPORT_H__ */
