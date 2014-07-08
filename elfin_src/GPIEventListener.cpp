/**
 * @file    GPIEventListener.cpp
 * @brief
 */

#include <exception>

#include "ROAdmin.h"
#include "GPIEventListener.h"

ELFIN::GPIEventListener::GPIEventListener() {

}

ELFIN::GPIEventListener::~GPIEventListener() {

}


int ELFIN::GPIEventListener::regGPIEvent(int GPIPortNum, int GPIEvent,
		int ROSpecID, enum EGPITriggerType trigType) {
	boost::mutex::scoped_lock lock(_pGPIRegLock);
	for (GPIEventRegistry::iterator iter = _GPIEventRegistry.begin();
			iter != _GPIEventRegistry.end();iter++) {
		if ((*iter).get<0>() == GPIPortNum && (*iter).get<1>() == GPIEvent) {
			READER_LOG(LOGTYPE_ERROR, "GPIEventListener: (GPIPortNum %d, Event %d) is already registered.\n", GPIPortNum, GPIEvent);
			return -1;
		}
	}
	const char *trigTypeStr = (trigType == ELFIN::GPI_StartTrigger) ? "StartTrigger" : "StopTrigger";
	READER_LOG(LOGTYPE_INFO, "GPIEventListener: (GPIPortNum %d, Event %d, TriggerType %s) is registered for ROSpecID %d.\n", GPIPortNum, GPIEvent, trigTypeStr, ROSpecID);
	_GPIEventRegistry.push_back(boost::tuple<int, int, int, int>(GPIPortNum, GPIEvent, ROSpecID, trigType));
	return 0;
}

int ELFIN::GPIEventListener::unregGPIEvent(int GPIPortNum, int GPIEvent) {
	// Because GPI port number starts from 1, 0 means nothing.
	// 0 comes as argument, when the ROSpec does not have GPITrigger.
	boost::mutex::scoped_lock lock(_pGPIRegLock);
	if (GPIPortNum == 0)
		return 0;
	for (GPIEventRegistry::iterator iter = _GPIEventRegistry.begin();
			iter != _GPIEventRegistry.end();iter++) {
		if ((*iter).get<0>() == GPIPortNum && (*iter).get<1>() == GPIEvent) {
			READER_LOG(LOGTYPE_INFO, "GPIEventListener: (GPIPortNum %d, Event %d) is unregistered.\n", GPIPortNum, GPIEvent);
			_GPIEventRegistry.erase(iter);
			return 0;
		}
	}
	READER_LOG(LOGTYPE_ERROR, "GPIEventListener: (GPIPortNum %d, Event %d) is not registered.\n", GPIPortNum, GPIEvent);
	return -1;
}
