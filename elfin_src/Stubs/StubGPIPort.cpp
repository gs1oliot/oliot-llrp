/**
 * @file    StubGPIPort.cpp
 * @brief
 */
#include "StubGPIPort.h"

ELFIN::StubGPIPort::StubGPIPort(StubReader *__pReader, LLRP::llrp_u16_t __GPIPortNum, LLRP::llrp_u1_t __config,
		enum LLRP::EGPIPortState __state)
:m_Reader(__pReader), m_PortNum(__GPIPortNum), m_config(__config), m_state(__state) {

}

int ELFIN::StubGPIPort::setState(enum LLRP::EGPIPortState state) {
	if (this->m_config == 1) {
		m_state = state;
		m_Reader->m_LLRPCore->m_ROAdmin->onGPIEvent(this->m_PortNum, this->m_state);
		return 0;
	} else {
		return -1;
	}
}

enum LLRP::EGPIPortState ELFIN::StubGPIPort::getState() {
	return this->m_state;
}

LLRP::llrp_u16_t ELFIN::StubGPIPort::getPortNum() {
	return this->m_PortNum;
}

LLRP::llrp_u1_t ELFIN::StubGPIPort::getConfig() {
	return this->m_config;
}

int ELFIN::StubGPIPort::setConfig(LLRP::llrp_u1_t _config) {
	this->m_config = _config;
	return 0;
}
