/**
 * @file    ROAdmin.cpp
 * @brief
 */

#include <vector>

#include <boost/unordered_map.hpp>
#include <boost/thread.hpp>
#include "AbstractAntennaOperation.h"
#include "ReaderOperation.h"
#include "LLRPCore.h"
#include "ReaderOperation.h"
#include "Scheduler.h"
#include "GPIEventListener.h"
#include "ROAdmin.h"

ELFIN::ROAdmin::ROAdmin(ELFIN::LLRPCore* __LLRPCore)
:GPIEventListener() {
	this->_LLRPCore = __LLRPCore;
}

ELFIN::ROAdmin::~ROAdmin() {
	READER_LOG (LOGTYPE_TRACE, "Destroying ROAdmin...\n");
	for (ROMap::iterator roIter = _ROMap.begin();
			roIter != _ROMap.end();roIter++) {
		delete (*roIter).second;
	}
	for (CROMap::iterator croIter = _CROMap.begin();
			croIter != _CROMap.end();croIter++) {
		delete (*croIter).second;
	}
}

/* When GPIEvent is invoked, start the predefined ROSpec for the event, only one time.
 * Because the invocation of ROSpec is based on the ROSpecID,
 * we can substitute it to other ROSpec if it is needed.
 */
void ELFIN::ROAdmin::onGPIEvent(LLRP::llrp_u16_t GPIPortNum, enum LLRP::EGPIPortState GPIEvent) {
	READER_LOG(LOGTYPE_INFO, "ROAdmin received GPIEvent (GPIPortNum %d, Event %d)\n", GPIPortNum, GPIEvent);
	boost::mutex::scoped_lock lock(_pGPIRegLock);
	for (GPIEventRegistry::iterator iter = _GPIEventRegistry.begin();
			iter != _GPIEventRegistry.end();iter++) {
		if ((*iter).get<0>() == GPIPortNum && (*iter).get<1>() == GPIEvent) {
			int ROSpecID = (*iter).get<2>();
			int trigType = (*iter).get<3>();
			const char *trigTypeStr = (trigType == ELFIN::GPI_StartTrigger) ? "StartTrigger" : "StopTrigger";
			READER_LOG(LOGTYPE_INFO, "GPIEventListener: (GPIPortNum %d, Event %d, TriggerType %s) invoked ROSpec %d.\n", GPIPortNum, GPIEvent, trigTypeStr, ROSpecID);
			if (trigType == ELFIN::GPI_StartTrigger) {
				boost::thread pThread(boost::bind(&ELFIN::ROAdmin::startROSpec, _LLRPCore->m_ROAdmin, ROSpecID));
				//_LLRPCore->_ROAdmin->startROSpec(ROSpecID);
			}
			else if (trigType == ELFIN::GPI_StopTrigger) {
				boost::thread pThread(boost::bind(&ELFIN::ROAdmin::stopROSpec, _LLRPCore->m_ROAdmin, ROSpecID));
				//_LLRPCore->_ROAdmin->stopROSpec(ROSpecID);
			}
			else {
				// not reached
				throw "error";
			}
		}
	}
}

int ELFIN::ROAdmin::addROSpec(LLRP::CROSpec *aRoSpec) {
	LLRP::llrp_u32_t pROSpecID = aRoSpec->getROSpecID();
	if (_CROMap.find(pROSpecID) == _CROMap.end()) {
		_CROMap.insert(CROMap_t(pROSpecID, aRoSpec));
		READER_LOG(LOGTYPE_INFO, "ROSpec ID %d is added\n", aRoSpec->getROSpecID());
		return 0;
	}
	else {
		READER_LOG(LOGTYPE_ERROR, "ROSpec ID %d is already exists!\n", aRoSpec->getROSpecID());
		return -1;
	}
}

// Endable the target ROSpec
int ELFIN::ROAdmin::enableROSpec(LLRP::llrp_u32_t aROSpecID) {
	std::vector<LLRP::llrp_u32_t> keyVector;
	// If ROSpecID is 0, then enable all disabled ROSpecs.
	int targetAllROSpecs = (aROSpecID == 0) ? TRUE : FALSE;
	if (targetAllROSpecs == TRUE) {
		for (CROMap::iterator croIter = _CROMap.begin();
				croIter != _CROMap.end();croIter++) {
			keyVector.push_back((*croIter).first);
		}
	}
	else {
		keyVector.push_back(aROSpecID);
	}

	for (std::vector<LLRP::llrp_u32_t>::iterator keyIter = keyVector.begin();
			keyIter != keyVector.end();keyIter++) {
		LLRP::llrp_u32_t targetROSpecID = (*keyIter);
		CROMap::iterator cro_iter = _CROMap.find(targetROSpecID);
		if (cro_iter == _CROMap.end()) {
			if (targetAllROSpecs == FALSE) {
				READER_LOG(LOGTYPE_ERROR, "enableROSpec: ROSpec ID %d does not exist.\n", targetROSpecID);
				return -1;
			}
			else
				continue;
		}
		LLRP::CROSpec *pROSpec =  (*cro_iter).second;
		if (pROSpec->getCurrentState() != LLRP::ROSpecState_Disabled) {
			if (targetAllROSpecs == FALSE) {
				READER_LOG(LOGTYPE_ERROR, "enableROSpec: ROSpec ID %d is not in Disabled state.\n", targetROSpecID);
				return -1;
			}
			else
				continue;
		}
		else if (pROSpec->getCurrentState() == LLRP::ROSpecState_Disabled) {
			pROSpec->setCurrentState(LLRP::ROSpecState_Inactive);
			ReaderOperation *ro = new ReaderOperation(pROSpec, _LLRPCore);
			if (_ROMap.find(targetROSpecID) != _ROMap.end()) {
				//ReaderOperation *pRO = (*_ROMap.find(targetROSpecID)).second;
				_ROMap.erase(targetROSpecID);
				ReaderOperation::deleteRO(ro);
				//boost::thread(boost::bind(&ELFIN::ReaderOperation::deleteRO, pRO)).join();
			}
			_ROMap.insert(ROMap_t(targetROSpecID, ro));
			// If there is GPI triggers, then register them.
			this->regGPIEvents(ro);
			int ROSpecStartTriggerType = ro->_CROSpec->getROBoundarySpec()->getROSpecStartTrigger()->getROSpecStartTriggerType();
			// Periodic and immediate start trigger is started automatically.
			if (ROSpecStartTriggerType == LLRP::ROSpecStartTriggerType_Periodic ||
					ROSpecStartTriggerType == LLRP::ROSpecStartTriggerType_Immediate) {
				ro->schedule();
			}
		}
		else {
			// not reached
			throw "error";
		}
	}

	return 0;
}

// Start the target ROSpec
int ELFIN::ROAdmin::startROSpec(LLRP::llrp_u32_t aROSpecID) {
	if (aROSpecID == 0) {
		READER_LOG(LOGTYPE_ERROR, "startROSpec: ROSpec ID cannot be 0 in startROSpec.\n", aROSpecID);
		return -1;
	}
	CROMap::iterator cro_iter = _CROMap.find(aROSpecID);
	if (cro_iter == _CROMap.end()) {
		READER_LOG(LOGTYPE_ERROR, "startROSpec: ROSpec ID %d does not exist.\n", aROSpecID);
		return -1;
	}
	LLRP::CROSpec *pROSpec = (*cro_iter).second;
	if (pROSpec->getCurrentState() == LLRP::ROSpecState_Inactive) {
		ReaderOperation *pRO = _ROMap.at(aROSpecID);
		if (pRO->_pStopping == 0) {
			READER_LOG (LOGTYPE_TRACE, "Running ROSpec %d\n", aROSpecID);
			pRO->schedule();
			pRO->run();
		} else {
			READER_LOG (LOGTYPE_TRACE, "ROSpec %d is already being stopped. Ignore the StartROSpec.\n", aROSpecID);
			return -1;
		}
	}
	else if (pROSpec->getCurrentState() == LLRP::ROSpecState_Active) {
		READER_LOG(LOGTYPE_ERROR, "startROSpec: ROSpec ID %d is already in Active state.\n", aROSpecID);
		return -1;
	}
	else {
		READER_LOG(LOGTYPE_ERROR, "startROSpec: ROSpec ID %d is not in Inactive state.\n", aROSpecID);
		return -1;
	}

	return 0;
}

// Stop the target ROSpec
int ELFIN::ROAdmin::stopROSpec(LLRP::llrp_u32_t aROSpecID) {
	if (aROSpecID == 0) {
		READER_LOG(LOGTYPE_ERROR, "stopROSpec: ROSpec ID cannot be 0 in stopROSpec.\n", aROSpecID);
		return -1;
	}

	CROMap::iterator cro_iter = _CROMap.find(aROSpecID);
	if (cro_iter == _CROMap.end()) {
		READER_LOG(LOGTYPE_ERROR, "stopROSpec: ROSpec ID %d does not exist.\n", aROSpecID);
		return -1;
	}
	LLRP::CROSpec *pROSpec = (*cro_iter).second;
	if (pROSpec->getCurrentState() == LLRP::ROSpecState_Active) {
		// stopROSpec just changes the state of ROSpec to inactive.
		// So, if the start trigger type is periodic, then it automatically goes back to active state,
		// after the timeout is over.
		if (pROSpec->getROBoundarySpec()->getROSpecStartTrigger()->getROSpecStartTriggerType() != LLRP::ROSpecStartTriggerType_Periodic) {
			ROMap::iterator ro_iter = _ROMap.find(aROSpecID);
			if (ro_iter == _ROMap.end()) {
				return -1; // ROSpec is active but RO does not exist, so error
			}
			ReaderOperation *pRO = (*ro_iter).second;
			if (pRO->_pStopping == 0) {
				pROSpec->setCurrentState(LLRP::ROSpecState_Inactive);
				READER_LOG (LOGTYPE_TRACE, "Stopping ROSpec %d\n", aROSpecID);
				pRO->stopRO();
			} else {
				READER_LOG (LOGTYPE_TRACE, "ROSpec %d is already being stopped. Ignore the StopROSpec.\n", aROSpecID);
				return -1;
			}
			return 0;
		}
		else {
			return 0;
		}
	}
	else {
		READER_LOG(LOGTYPE_ERROR, "stopROSpec: ROSpec ID %d is not in Active state.\n", aROSpecID);
		return -1;
	}
}

// Disable the target ROSpec
int ELFIN::ROAdmin::disableROSpec(LLRP::llrp_u32_t aROSpecID) {
	std::vector<LLRP::llrp_u32_t> keyVector;
	// If ROSpecID is 0, then disable all ROSpecs.
	int targetAllROSpecs = (aROSpecID == 0) ? TRUE : FALSE;
	if (targetAllROSpecs == TRUE) {
		for (CROMap::iterator croIter = _CROMap.begin();
				croIter != _CROMap.end();croIter++) {
			keyVector.push_back((*croIter).first);
		}
	}
	else {
		keyVector.push_back(aROSpecID);
	}

	// keyVector contains ROSpec IDs to disable.
	for (std::vector<LLRP::llrp_u32_t>::iterator keyIter = keyVector.begin();
			keyIter != keyVector.end();keyIter++) {
		LLRP::llrp_u32_t targetROSpecID = (*keyIter);
		CROMap::iterator cro_iter = _CROMap.find(targetROSpecID);
		if (cro_iter == _CROMap.end()) {
			if (targetAllROSpecs == FALSE) {
				READER_LOG(LOGTYPE_ERROR, "disableROSpec: ROSpec ID %d does not exist.\n", targetROSpecID);
				return -1;
			}
			else
				continue;
		}
		LLRP::CROSpec *pROSpec = (*cro_iter).second;
		pROSpec->setCurrentState(LLRP::ROSpecState_Disabled);

		ROMap::iterator ro_iter = _ROMap.find(targetROSpecID);
		if (ro_iter != _ROMap.end()) {
			ReaderOperation *pRO = (*ro_iter).second;
			_ROMap.erase(targetROSpecID);
			this->unregGPIEvents(pRO);
			ReaderOperation::deleteRO(pRO);
		}
	}
	return 0;
}

// Disable and delete the target ROSpec
int ELFIN::ROAdmin::deleteROSpec(LLRP::llrp_u32_t aROSpecID) {
	std::vector<LLRP::llrp_u32_t> keyVector;
	// If ROSpecID is 0, then delete all ROSpecs.
	int targetAllROSpecs = (aROSpecID == 0) ? TRUE : FALSE;
	if (targetAllROSpecs == TRUE) {
		for (CROMap::iterator croIter = _CROMap.begin();
				croIter != _CROMap.end();croIter++) {
			keyVector.push_back((*croIter).first);
		}
	}
	else {
		keyVector.push_back(aROSpecID);
	}

	int result = 0;
	for (std::vector<LLRP::llrp_u32_t>::iterator keyIter = keyVector.begin();
			keyIter != keyVector.end();keyIter++) {
		LLRP::llrp_u32_t targetROSpecID = (*keyIter);
		int disableResult = disableROSpec(targetROSpecID);
		if (disableResult == 0) {
			CROMap::iterator cro_iter = _CROMap.find(targetROSpecID);
			if (cro_iter == _CROMap.end()) {
				READER_LOG(LOGTYPE_ERROR, "deleteROSpec: ROSpec ID %d does not exist.\n", targetROSpecID);
				return -1;
			}
			LLRP::CROSpec *pROSpec = (*cro_iter).second;
			_CROMap.erase(targetROSpecID);
			delete pROSpec;
			pROSpec = NULL;
		}
		else {
			// Some error happened!
			result = disableResult;
		}
	}
	return result;
}

/**
 * @warning You should delete the returned vector to avoid memory leakage.\n
 * And you must not delete the ROSpecs in the vector, because they are original ones.
 */
std::vector<LLRP::CROSpec*> *ELFIN::ROAdmin::getROSpecs() {
	std::vector<LLRP::CROSpec*> *ROSpecs = new std::vector<LLRP::CROSpec*>();
	for (CROMap::iterator iter = _CROMap.begin(); iter != _CROMap.end(); ++iter) {
		ROSpecs->push_back(iter->second);
	}
	return ROSpecs;
}

int ELFIN::ROAdmin::regGPIEvents(ReaderOperation* ro) {
	int ROSpecStartTriggerType = ro->_CROSpec->getROBoundarySpec()->getROSpecStartTrigger()->getROSpecStartTriggerType();
	if (ROSpecStartTriggerType == LLRP::ROSpecStartTriggerType_GPI) {
		LLRP::CGPITriggerValue *pGPIValue = ro->_CROSpec->getROBoundarySpec()->getROSpecStartTrigger()->getGPITriggerValue();
		if (pGPIValue == NULL)
			return -1;
		this->regGPIEvent(pGPIValue->getGPIPortNum(), pGPIValue->getGPIEvent(),
				ro->_CROSpec->getROSpecID(), ELFIN::GPI_StartTrigger);
	}
	int ROSpecStopTriggerType = ro->_CROSpec->getROBoundarySpec()->getROSpecStopTrigger()->getROSpecStopTriggerType();
	if (ROSpecStopTriggerType == LLRP::ROSpecStopTriggerType_GPI_With_Timeout) {
		LLRP::CGPITriggerValue *pGPIValue = ro->_CROSpec->getROBoundarySpec()->getROSpecStopTrigger()->getGPITriggerValue();
		if (pGPIValue == NULL)
			return -1;
		this->regGPIEvent(pGPIValue->getGPIPortNum(), pGPIValue->getGPIEvent(),
				ro->_CROSpec->getROSpecID(), ELFIN::GPI_StopTrigger);
	}
	return 0;
}

int ELFIN::ROAdmin::unregGPIEvents(ReaderOperation* ro) {
	int ROSpecStartTriggerType = ro->_CROSpec->getROBoundarySpec()->getROSpecStartTrigger()->getROSpecStartTriggerType();
	if (ROSpecStartTriggerType == LLRP::ROSpecStartTriggerType_GPI) {
		LLRP::CGPITriggerValue *pGPIValue = ro->_CROSpec->getROBoundarySpec()->getROSpecStartTrigger()->getGPITriggerValue();
		if (pGPIValue == NULL)
			return -1;
		this->unregGPIEvent(pGPIValue->getGPIPortNum(), pGPIValue->getGPIEvent());
	}
	int ROSpecStopTriggerType = ro->_CROSpec->getROBoundarySpec()->getROSpecStopTrigger()->getROSpecStopTriggerType();
	if (ROSpecStopTriggerType == LLRP::ROSpecStopTriggerType_GPI_With_Timeout) {
		LLRP::CGPITriggerValue *pGPIValue = ro->_CROSpec->getROBoundarySpec()->getROSpecStopTrigger()->getGPITriggerValue();
		if (pGPIValue == NULL)
			return -1;
		this->unregGPIEvent(pGPIValue->getGPIPortNum(), pGPIValue->getGPIEvent());
	}
	return 0;
}
