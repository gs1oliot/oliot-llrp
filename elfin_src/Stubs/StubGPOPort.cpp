/**
 * @file    StubGPOPort.cpp
 * @brief
 */

#include "StubGPOPort.h"

ELFIN::StubGPOPort::StubGPOPort(StubReader *__pReader, LLRP::llrp_u16_t __GPOPortNum, LLRP::llrp_u1_t __data)
:m_Reader(__pReader), m_GPOPortNum(__GPOPortNum), m_data(__data) {

}

LLRP::llrp_u16_t ELFIN::StubGPOPort::getPortNum() {
	return this->m_GPOPortNum;
}

LLRP::llrp_u1_t ELFIN::StubGPOPort::getData() {
	return this->m_data;
}

int ELFIN::StubGPOPort::setData(LLRP::llrp_u1_t _data) {
	m_Reader->m_StubApp->GPOEventCallback(this->m_GPOPortNum, _data);
	this->m_data = _data;
	return 0;
}
