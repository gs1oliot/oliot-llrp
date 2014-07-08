/**
 * @file    AOAdmin.cpp
 * @brief
 */

#include <vector>

//#include <boost/thread/recursive_mutex.hpp>
#include "LLRPCore.h"
#include "AOAdmin.h"
#include "ROAdmin.h"
#include "ELFIN_Platform.h"
#include "AccessOperation.h"

int ELFIN::AOAdmin::enableAccessSpec(LLRP::llrp_u32_t aAccessSpecID) {
	boost::recursive_mutex::scoped_lock lock(this->_pLLRPCore->m_ROAdmin->_pROLock);
	std::vector<LLRP::llrp_u32_t> keyVector;
	// If AccessSpecID is 0, then disable all AccessSpecs.
	int targetAllAccessSpecs = (aAccessSpecID == 0) ? TRUE : FALSE;
	if (targetAllAccessSpecs == TRUE) {
		for (CAOMap::iterator caoIter = _CAOMap.begin();
				caoIter != _CAOMap.end();caoIter++) {
			keyVector.push_back((*caoIter).first);
		}
	}
	else {
		keyVector.push_back(aAccessSpecID);
	}

	for (std::vector<LLRP::llrp_u32_t>::iterator keyIter = keyVector.begin();
			keyIter != keyVector.end();keyIter++) {
		LLRP::llrp_u32_t targetAccessSpecID = (*keyIter);
		CAOMap::iterator cao_iter = _CAOMap.find(targetAccessSpecID);
		if (cao_iter == _CAOMap.end()) {
			if (targetAccessSpecID == FALSE) {
				READER_LOG(LOGTYPE_ERROR, "enableAccessSpec: AccessSpec ID %d does not exist.\n", targetAccessSpecID);
				return -1;
			}
			else
				continue;
		}
		LLRP::CAccessSpec *pAccessSpec = (*cao_iter).second;
		if (pAccessSpec->getCurrentState() != LLRP::AccessSpecState_Disabled) {
			if (targetAccessSpecID == FALSE) {
				READER_LOG(LOGTYPE_ERROR, "enableAccessSpec: AccessSpec ID %d is not in Disabled state.\n", targetAccessSpecID);
				return -1;
			}
			else
				continue;
		}
		else if (pAccessSpec->getCurrentState() == LLRP::AccessSpecState_Disabled) {
			pAccessSpec->setCurrentState(LLRP::AccessSpecState_Active);
			AccessOperation *ao = new AccessOperation(this->_pLLRPCore, pAccessSpec);
			_AOMap.insert(AOMap_t(targetAccessSpecID, ao));
		}
		else {
			// not reached
			throw "error";
		}
	}

	return 0;
}

int ELFIN::AOAdmin::addAccessSpec(LLRP::CAccessSpec *aAccessSpec) {
	/** @fixme  In case there are multiple AccessSpecs that get matched during a TagSpec lookup, the
	 * Reader SHALL only execute the first enabled AccessSpec that matches, where the
	 * ordering of the AccessSpecs is the order in which the AccessSpecs were created by the
	 * Client. (LLRP Spec 12.2.1.2)
	 */
	int pAccessSpecID = aAccessSpec->getAccessSpecID();
	if (_CAOMap.find(pAccessSpecID) == _CAOMap.end()) {
		_CAOMap.insert(CAOMap_t(pAccessSpecID, aAccessSpec));
		READER_LOG(LOGTYPE_INFO, "AccessSpec ID %d is added\n", pAccessSpecID);
		return 0;
	}
	else {
		READER_LOG(LOGTYPE_ERROR, "AccessSpec ID %d is already exists!\n", pAccessSpecID);
		return -1;
	}
}

int ELFIN::AOAdmin::disableAccessSpec(LLRP::llrp_u32_t aAccessSpecID) {
	boost::recursive_mutex::scoped_lock lock(this->_pLLRPCore->m_ROAdmin->_pROLock);
	std::vector<LLRP::llrp_u32_t> keyVector;
	// If AccessSpecID is 0, then disable all AccessSpecs.
	int targetAllAccessSpecs = (aAccessSpecID == 0) ? TRUE : FALSE;
	if (targetAllAccessSpecs == TRUE) {
		for (CAOMap::iterator caoIter = _CAOMap.begin();
				caoIter != _CAOMap.end();caoIter++) {
			keyVector.push_back((*caoIter).first);
		}
	}
	else {
		keyVector.push_back(aAccessSpecID);
	}

	for (std::vector<LLRP::llrp_u32_t>::iterator keyIter = keyVector.begin();
			keyIter != keyVector.end();keyIter++) {
		LLRP::llrp_u32_t targetAccessSpecID = (*keyIter);
		CAOMap::iterator cao_iter = _CAOMap.find(targetAccessSpecID);
		if (cao_iter == _CAOMap.end()) {
			if (targetAccessSpecID == FALSE) {
				READER_LOG(LOGTYPE_ERROR, "disableAccessSpec: AccessSpec ID %d does not exist.\n", targetAccessSpecID);
				return -1;
			}
			else
				continue;
		}
		LLRP::CAccessSpec *pAccessSpec = (*cao_iter).second;

		pAccessSpec->setCurrentState(LLRP::AccessSpecState_Disabled);

		AOMap::iterator ao_iter = _AOMap.find(targetAccessSpecID);
		if (ao_iter != _AOMap.end()) {
			AccessOperation *ao = (*ao_iter).second;
			_AOMap.erase(targetAccessSpecID);
			delete ao;
		}
	}
	return 0;
}

int ELFIN::AOAdmin::deleteAccessSpec(LLRP::llrp_u32_t aAccessSpecID) {
	boost::recursive_mutex::scoped_lock lock(this->_pLLRPCore->m_ROAdmin->_pROLock);
	std::vector<LLRP::llrp_u32_t> keyVector;
	// If AccessSpecID is 0, then delete all AccessSpecs.
	int targetAllAccessSpecs = (aAccessSpecID == 0) ? TRUE : FALSE;
	if (targetAllAccessSpecs == TRUE) {
		for (CAOMap::iterator caoIter = _CAOMap.begin();
				caoIter != _CAOMap.end();caoIter++) {
			keyVector.push_back((*caoIter).first);
		}
	}
	else {
		keyVector.push_back(aAccessSpecID);
	}
	int result = 0;
	for (std::vector<LLRP::llrp_u32_t>::iterator keyIter = keyVector.begin();
			keyIter != keyVector.end();keyIter++) {
		LLRP::llrp_u32_t targetAccessSpecID = (*keyIter);
		int disableResult = disableAccessSpec(targetAccessSpecID);
		if (disableResult == 0) {
			CAOMap::iterator cao_iter = _CAOMap.find(targetAccessSpecID);
			if (cao_iter == _CAOMap.end()) {
				READER_LOG(LOGTYPE_ERROR, "deleteAccessSpec: AccessSpec ID %d does not exist.\n", targetAccessSpecID);
				if (targetAllAccessSpecs == FALSE) {
					return -1;
				}
				else
					continue;
			}
			LLRP::CAccessSpec *pAccessSpec = (*cao_iter).second;
			_CAOMap.erase(targetAccessSpecID);
			delete pAccessSpec;
			pAccessSpec = NULL;
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
 * And you must not delete the AccessSpecs in the vector, because they are original ones.
 */
std::vector<LLRP::CAccessSpec*> *ELFIN::AOAdmin::getAccessSpecs() {
	std::vector<LLRP::CAccessSpec*> *AccessSpecs = new std::vector<LLRP::CAccessSpec*>();
	for (CAOMap::iterator it = _CAOMap.begin(); it != _CAOMap.end(); ++it) {
		AccessSpecs->push_back(it->second);
	}
	return AccessSpecs;
}

ELFIN::AOMap::iterator ELFIN::AOAdmin::beginAOMap() {
	return _AOMap.begin();
}

ELFIN::AOMap::iterator ELFIN::AOAdmin::endAOMap() {
	return _AOMap.end();
}

int ELFIN::AOAdmin::countAOMap() {
	return _AOMap.size();
}

ELFIN::AOAdmin::AOAdmin(LLRPCore *__pLLRPCore) {
	this->_pLLRPCore = __pLLRPCore;
}

ELFIN::AOAdmin::~AOAdmin() {
	READER_LOG (LOGTYPE_TRACE, "Destroying AOAdmin...\n");
	for (AOMap::iterator aoIter = _AOMap.begin();
			aoIter != _AOMap.end();aoIter++) {
		delete (*aoIter).second;
	}
	for (CAOMap::iterator caoIter = _CAOMap.begin();
			caoIter != _CAOMap.end();caoIter++) {
		delete (*caoIter).second;
	}
}
