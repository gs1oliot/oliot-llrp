/*
 * ROSpecTrigger.cpp
 *
 *  Created on: Apr 14, 2014
 *      Author: iot-team
 */

#include "ROSpecTrigger.h"

ELFIN::ROSpecTrigger::ROSpecTrigger(LLRP::CROSpec* a_ROSpec)
: m_ROSpec(a_ROSpec){
	if (this->m_ROSpec->getROBoundarySpec()->getROSpecStopTrigger()->getROSpecStopTriggerType()
			== LLRP::ROSpecStopTriggerType_Null) {
		// DO NOTHING
	} else if (this->m_ROSpec->getROBoundarySpec()->getROSpecStopTrigger()->getROSpecStopTriggerType()
			== LLRP::ROSpecStopTriggerType_Duration) {
		m_timeOut = this->m_ROSpec->getROBoundarySpec()->getROSpecStopTrigger()->getDurationTriggerValue();
	} else if (this->m_ROSpec->getROBoundarySpec()->getROSpecStopTrigger()->getROSpecStopTriggerType()
			== LLRP::ROSpecStopTriggerType_GPI_With_Timeout) {
		LLRP::CGPITriggerValue *pGPIValue = this->m_ROSpec->getROBoundarySpec()->getROSpecStopTrigger()->getGPITriggerValue();
		m_timeOut = pGPIValue->getTimeout();
	} else {
		throw "error";
	}
}

ELFIN::ROSpecTrigger::~ROSpecTrigger() {
}
