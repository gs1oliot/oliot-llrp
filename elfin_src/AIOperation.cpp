/**
 * @file    AIOperation.cpp
 * @brief
 */

#include <boost/unordered_map.hpp>
#include "AccessOperation.h"
#include "Stubs/StubTag.h"
#include "ReaderOperation.h"
#include "Stubs/StubReader.h"
#include "AOAdmin.h"
#include "ELFIN_Platform.h"
#include "AIOperation.h"

ELFIN::AIOperation::AIOperation(LLRP::CROSpec *__pROSpec, LLRP::CAISpec *__pAISpec, StubReader *__pReader,
		ReaderOperation *__pRO, AOAdmin *__AOAdmin, int __pSpecIndex)
: ELFIN::AbstractAntennaOperation(__pReader, __pRO, &LLRP::CAISpec::s_typeDescriptor, __pSpecIndex),
  _pROSpec(__pROSpec), _pAISpec(__pAISpec), _pTagReportSet(new TagReportSet()), isOnSingulation(0),
  _pAOAdmin(__AOAdmin) {
	if (__pROSpec->getROReportSpec() != NULL) {
		_pROReportSpec = __pROSpec->getROReportSpec();
	}
	else {
		_pROReportSpec = this->_pRO->_pConfig->_pDefaultROReportSpec;
	}
	if (_pROReportSpec->getTagReportContentSelector()->countAirProtocolEPCMemorySelector() == 0) {
		_pMemSelector = NULL;
	} else if (_pROReportSpec->getTagReportContentSelector()->countAirProtocolEPCMemorySelector() == 1) {
		_pMemSelector = (LLRP::CC1G2EPCMemorySelector *) *(_pROReportSpec->getTagReportContentSelector()->beginAirProtocolEPCMemorySelector());
	} else {
		READER_LOG (LOGTYPE_ERROR, "AirProtocolEPCMemorySelector exists more than 1. Reading only the first one\n");
		_pMemSelector = (LLRP::CC1G2EPCMemorySelector *) *(_pROReportSpec->getTagReportContentSelector()->beginAirProtocolEPCMemorySelector());
	}
}


ELFIN::AIOperation::~AIOperation() {
	READER_LOG(LOGTYPE_TRACE, "Destroying AIOperation...\n");
	delete _pTagReportSet;
}


ELFIN::TagReportSet *ELFIN::AIOperation::run() {
	int antennaCount = _pAISpec->getAntennaIDs().m_nValue;
	bool invAllAntennas = false;
	AntennaMap::iterator iter;
	for (int i = 0;i < antennaCount;i++) {
		if (_pAISpec->getAntennaIDs().m_pValue[i] == 0) {
			// If there is 0 in antenna ID, then inventory from all antennas
			READER_LOG(LOGTYPE_TRACE, "AIOperation: antennaID is 0\n");
			antennaCount = _pReader->countAntennaMap();
			invAllAntennas = true;
			break;
		}
		else {
			READER_LOG(LOGTYPE_TRACE, "AIOperation: antennaID is %d\n", _pAISpec->getAntennaIDs().m_pValue[i]);
		}
	}
	READER_LOG(LOGTYPE_TRACE, "AIOperation: Number of antennas to inventory: %d\n", antennaCount);

	std::vector<StubTag *> *tagVector = NULL;
	TagReportSet *pNewTagReportSet = new TagReportSet();

	uint64_t pAISpecStopTime = Utils::getCurrentTimeMilliseconds();
	uint64_t pAISpecNoMoreNewTagStopTime = 0;
	uint64_t pAISpecNoMoreNewTagTimeout = 0;
	int pAISpecNoMoreNewTagPreviousNumberOfTags = -1;
	int pAISpecNumberOfAttempts = -1;
	int pAISpecNumberOfTags = -1;

	if (this->_pAISpec->getAISpecStopTrigger()->getAISpecStopTriggerType() == LLRP::AISpecStopTriggerType_Null) {
		pAISpecStopTime = 0;
	} else if (this->_pAISpec->getAISpecStopTrigger()->getAISpecStopTriggerType() == LLRP::AISpecStopTriggerType_Duration) {
		pAISpecStopTime += this->_pAISpec->getAISpecStopTrigger()->getDurationTrigger();
	} else if (this->_pAISpec->getAISpecStopTrigger()->getAISpecStopTriggerType() == LLRP::AISpecStopTriggerType_GPI_With_Timeout) {
		assert (this->_pAISpec->getAISpecStopTrigger()->getGPITriggerValue());
		pAISpecStopTime += this->_pAISpec->getAISpecStopTrigger()->getGPITriggerValue()->getTimeout();
		/// @todo  add GPI event listener
	} else if (this->_pAISpec->getAISpecStopTrigger()->getAISpecStopTriggerType() == LLRP::AISpecStopTriggerType_Tag_Observation) {
		LLRP::CTagObservationTrigger *pTrigger = this->_pAISpec->getAISpecStopTrigger()->getTagObservationTrigger();
		assert (pTrigger);
		pAISpecStopTime += pTrigger->getTimeout();

		if (pTrigger->getTriggerType()
				== LLRP::TagObservationTriggerType_N_Attempts_To_See_All_Tags_In_FOV_Or_Timeout) {
			pAISpecNumberOfAttempts = pTrigger->getNumberOfAttempts();
		} else if (pTrigger->getTriggerType()
				== LLRP::TagObservationTriggerType_Upon_Seeing_N_Tags_Or_Timeout ||
				pTrigger->getTriggerType()
				== LLRP::TagObservationTriggerType_Upon_Seeing_N_Unique_Tags_Or_Timeout) {
			pAISpecNumberOfTags = pTrigger->getNumberOfTags();
		} else if (pTrigger->getTriggerType()
				== LLRP::TagObservationTriggerType_Upon_Seeing_No_More_New_Tags_For_Tms_Or_Timeout ||
				pTrigger->getTriggerType()
				== LLRP::TagObservationTriggerType_Upon_Seeing_No_More_New_Unique_Tags_For_Tms_Or_Timeout) {
			pAISpecNoMoreNewTagPreviousNumberOfTags = 0;
			pAISpecNoMoreNewTagTimeout = pTrigger->getT();
		} else {
			throw "error";
		}
	}

	if (pAISpecStopTime != 0) {
		_pAISpecStopTriggerThread = boost::thread(
				boost::bind(&ELFIN::AIOperation::run_AISpecStopTriggerThread, this, pAISpecStopTime));
	}

	int AISpecLoopEnabled = 1;
	boost::unordered_set<MemoryBank, container_hash<MemoryBank> > pAlreadyAccessedEPCSet;
	while (AISpecLoopEnabled == 1) {
		for (std::list<LLRP::CInventoryParameterSpec *>::iterator inv_iter = this->_pAISpec->beginInventoryParameterSpec();
				inv_iter != this->_pAISpec->endInventoryParameterSpec(); inv_iter++) {
			LLRP::CInventoryParameterSpec *pInvSpec = (*inv_iter);
			/// @fixme  Add handling of AntennaConfigurations of InventoryParameterSpec
			READER_LOG (LOGTYPE_DEBUG, "Starting tag singulation for InventoryParameterSpec %d.\n", pInvSpec->getInventoryParameterSpecID());
			if (pInvSpec->getProtocolID() != LLRP::AirProtocols_EPCGlobalClass1Gen2) {
				READER_LOG (LOGTYPE_ERROR, "Protocol ID %d is not supported. Ignoring this InventoryParameterSpec\n", pInvSpec->getProtocolID());
				continue;
			}
			isOnSingulation = 1;
			this->_pReader->m_StubReaderLock.lock();
			if (_pReader->StartTagSingulation(0) != 0) {
				READER_LOG (LOGTYPE_ERROR, "AIOperation: Tag singulation error. Ignoring this AI round.\n");
				_pReader->FinishTagSingulation();
				isOnSingulation = 0;
				this->_pReader->m_StubReaderLock.unlock();
				continue;
			}

			for (AntennaMap::iterator ant_iter = _pReader->beginAntennaMap();
					ant_iter != _pReader->endAntennaMap();ant_iter++) {
				if (invAllAntennas == false) {
					int matchAntennaExists = FALSE;
					for (int i = 0;i < antennaCount;i++) {
						// If there is any matching in the antenna list, then break.
						if (_pAISpec->getAntennaIDs().m_pValue[i] == (*ant_iter).first) {
							matchAntennaExists = TRUE;
							break;
						}
					}
					if (matchAntennaExists == FALSE) {
						continue;
					}
				}

				READER_LOG (LOGTYPE_TRACE, "AIOperation:: Reading antenna %d\n", (*ant_iter).first);
				tagVector = _pReader->getCurrentTags((*ant_iter).first);
				for (std::vector<StubTag *>::iterator tagIter = tagVector->begin();
						tagIter != tagVector->end();tagIter++) {
					StubTag *pStubTag = (*tagIter);
					const MemoryBank pBank = MemoryBank(pStubTag->_MB[1]->begin() + 4, pStubTag->_MB[1]->end());
					// If the StubTag is already applied by AccessSpec, skip it.
					if (pAlreadyAccessedEPCSet.find(pBank) != pAlreadyAccessedEPCSet.end()) {
						continue;
					}
					LLRP::CTagReportData *pTagRpt = createTagReportData (pStubTag, pInvSpec->getInventoryParameterSpecID());
					/// @fixme  iterate AccessSpec in the sequence of enable time, and execute only the firstly matched one.
					// Refer LLRP Spec 1.1 12.2.1.2.
					for (AOMap::iterator aoIter = _pAOAdmin->beginAOMap();
							aoIter != _pAOAdmin->endAOMap();aoIter++) {
						AccessOperation *ao = (*aoIter).second;
						LLRP::CAccessSpec *_pCAccessSpec = ao->_pCAccessSpec;
						READER_LOG (LOGTYPE_TRACE, "Executing Access operation %d\n", _pCAccessSpec->getAccessSpecID());
						// Check the target ROSpec ID, Antenna ID, and Protocol ID of AccessSpec. Then match it.
						if ((_pCAccessSpec->getROSpecID() != 0 && _pCAccessSpec->getROSpecID() != this->_pROSpec->getROSpecID()) ||
								(_pCAccessSpec->getAntennaID() != 0 && pStubTag->_pAntennaID != _pCAccessSpec->getAntennaID()) ||
								(_pCAccessSpec->getProtocolID() != LLRP::AirProtocols_EPCGlobalClass1Gen2)) {
							// This AccessSpec does not target the current tag, so do nothing.
						}
						else {
							if (pStubTag->_logEpcStr != NULL)
								READER_LOG (LOGTYPE_TRACE, "Start matching TagSpec to the RFID tag (%s)\n", pStubTag->_logEpcStr);
							if (ao->matchTagSpec(pStubTag, _pRO->_CROSpec->getROSpecID()) == TRUE) {
								READER_LOG (LOGTYPE_DEBUG, "Finished matching TagSpec to the RFID tag: Matched, (%s)\n", pStubTag->_logEpcStr);
								bool isOpCountLimited = (ao->_pCAccessSpec->getAccessSpecStopTrigger()->getAccessSpecStopTrigger() == LLRP::AccessSpecStopTriggerType_Operation_Count
										&& ao->_pCAccessSpec->getAccessSpecStopTrigger()->getOperationCountValue() != 0);
								int opCountLeft = ao->_pCAccessSpec->getAccessSpecStopTrigger()->getOperationCountValue() - ao->_pOperationCount;
								if (isOpCountLimited == TRUE && opCountLeft <= 0) {
									continue;
								}
								if (this->_pROReportSpec->getTagReportContentSelector()->getEnableAccessSpecID() == 1) {
									LLRP::CAccessSpecID *pAccessSpecID = new LLRP::CAccessSpecID();
									pAccessSpecID->setAccessSpecID(ao->_pCAccessSpec->getAccessSpecID());
									pTagRpt->setAccessSpecID(pAccessSpecID);
								}
								ao->executeOpSpecs(pStubTag, pTagRpt);
								// If AccessSpecStopTrigger is AccessSpecStopTriggerType_Operation_Count and
								// the operation count is out, then delete the AccessSpec.
								// Because this loop would stop right after the single execution of AccessSpec,
								// there is no problem of the invalidated iterator.
								if (isOpCountLimited == TRUE) {
									ao->_pOperationCount++;
									opCountLeft--;
									READER_LOG (LOGTYPE_DEBUG, "Access Operation #d: operation count left: %d\n", ao->_pCAccessSpec->getAccessSpecID(), opCountLeft);
								}
								if (ao->_pAccessReportSpec->getAccessReportTrigger() == LLRP::AccessReportTriggerType_End_Of_AccessSpec) {
									LLRP::CRO_ACCESS_REPORT *pACReport = new LLRP::CRO_ACCESS_REPORT();
									pACReport->addTagReportData(pTagRpt);
									pACReport->removeSubParameterFromAllList(pTagRpt);
									this->_pRO->_pLLRPCore->sendMessage(pACReport);
									delete pACReport;
								}
								pAlreadyAccessedEPCSet.insert(MemoryBank(pStubTag->_MB[1]->begin() + 4, pStubTag->_MB[1]->end()));
								break;
							}
							else {
								READER_LOG (LOGTYPE_DEBUG, "Finished matching TagSpec to the RFID tag: Not matched\n");
							}
						}
					}
					// Find AccessSpecs to delete
					// Used two loops to avoid the invalidation of iterator
					std::vector<LLRP::llrp_u32_t> specsToDeleteVector;
					for (AOMap::iterator aoIter = _pAOAdmin->beginAOMap();
							aoIter != _pAOAdmin->endAOMap();aoIter++) {
						AccessOperation *ao = (*aoIter).second;
						int opCountLeft = ao->_pCAccessSpec->getAccessSpecStopTrigger()->getOperationCountValue() - ao->_pOperationCount;
						bool isOpCountLimited = (ao->_pCAccessSpec->getAccessSpecStopTrigger()->getAccessSpecStopTrigger() == LLRP::AccessSpecStopTriggerType_Operation_Count
								&& ao->_pCAccessSpec->getAccessSpecStopTrigger()->getOperationCountValue() != 0);
						if (isOpCountLimited == true && opCountLeft <= 0) {
							READER_LOG (LOGTYPE_INFO, "Operation count is out. Deleting AccessSpec %d\n", ao->_pCAccessSpec->getAccessSpecID());
							specsToDeleteVector.push_back(ao->_pCAccessSpec->getAccessSpecID());
						}
					}
					// Delete AccessSpecs
					for (std::vector<LLRP::llrp_u32_t>::iterator del_iter = specsToDeleteVector.begin();
							del_iter != specsToDeleteVector.end(); del_iter++) {
						this->_pAOAdmin->deleteAccessSpec(*del_iter);
					}
					// Access operation finished
					this->addTagReportUnique(pNewTagReportSet,pTagRpt);
					/*
				if (insResult == 0) {
					READER_LOG (LOGTYPE_TRACE, "loopAI_Report is added\n");
				}
				else {
					READER_LOG (LOGTYPE_TRACE, "loopAI_Report is overwritten to existing one\n");
				}
					 */
				}
				delete tagVector;
				tagVector = NULL;
			}
			_pReader->FinishTagSingulation();
			READER_LOG (LOGTYPE_DEBUG, "Finished tag singulation.\n");
			isOnSingulation = 0;
			this->_pReader->m_StubReaderLock.unlock();
			// sleep 1ms to yield for other operations that use _pReaderLock
			boost::this_thread::sleep_for(boost::chrono::milliseconds(1));

			// Check the timeout.
			// If it matches stop condition, then stop the loop.
			uint64_t pCurrentTime = Utils::getCurrentTimeMilliseconds();
			if (pAISpecStopTime != 0 && pCurrentTime > pAISpecStopTime) {
				// Timeout.
				AISpecLoopEnabled = 0;
				continue;
			}

			// Check the number of attempts
			// If it matches stop condition, then stop the loop.
			if (pAISpecNumberOfAttempts != -1) {
				pAISpecNumberOfAttempts--;
				if (pAISpecNumberOfAttempts <= 0) {
					AISpecLoopEnabled = 0;
					continue;
				}
			}

			// Check the number of tags (uniquely) observed
			// If it matches stop condition, then stop the loop.
			if (pAISpecNumberOfTags != -1) {
				if (static_cast<int>(pNewTagReportSet->size()) >= pAISpecNumberOfTags) {
					AISpecLoopEnabled = 0;
					continue;
				}
			}

			// Check the number of tags (uniquely) observed and compare with previous attempt.
			// If there is no difference (no more tages observed), then set stop time with timeout.
			if (pAISpecNoMoreNewTagPreviousNumberOfTags != -1) {
				if (static_cast<int>(pNewTagReportSet->size()) != pAISpecNoMoreNewTagPreviousNumberOfTags) {
					pAISpecNoMoreNewTagStopTime =
							Utils::getCurrentTimeMilliseconds() + pAISpecNoMoreNewTagTimeout;
				}

				pAISpecNoMoreNewTagPreviousNumberOfTags = static_cast<int>(pNewTagReportSet->size());
			}

			// If the time is out while observing no more new tags, stop the loop.
			if (pAISpecNoMoreNewTagStopTime != 0
					&& pCurrentTime > pAISpecNoMoreNewTagStopTime) {
				AISpecLoopEnabled = 0;
				continue;
			}

			// Check whether the ROSpecStopTrigger is invoked.
			if (_pRO->_pROSpecStopTriggerInvoked == 1) {
				AISpecLoopEnabled = 0;
				continue;
			}
		}

	}
	_pAISpecStopTriggerThread.interrupt();
	_pAISpecStopTriggerThread.join();

	// Replace reports of _pTagReportVector that contains same EPCID, with new reports in pTagReportVector.

	for (TagReportSet::iterator tsIter = pNewTagReportSet->begin();
			tsIter != pNewTagReportSet->end(); tsIter++) {
		this->addTagReportUnique(_pTagReportSet, (*tsIter));
	}

	delete pNewTagReportSet;

	READER_LOG (LOGTYPE_DEBUG, "AIOperation:: Finished: %d tags are currently stored.\n", _pTagReportSet->size());

	return _pTagReportSet;
}

LLRP::CTagReportData* ELFIN::AIOperation::createTagReportData(
		StubTag* pStubTag, int pInvSpecID) {
	LLRP::CTagReportData *pTagRpt = new LLRP::CTagReportData();
	LLRP::CEPC_96 *pEPCData = new LLRP::CEPC_96();
	LLRP::llrp_u96_t EPCID;

	for (int j = 0;j < 12;j++) { // first 16-bits are PC and CRC
		EPCID.m_aValue[j] = pStubTag->_MB[1]->at(j + 4);
	}
	pEPCData->setEPC(EPCID);
	pTagRpt->setEPCParameter(pEPCData);

	if (_pROReportSpec->getTagReportContentSelector()->getEnableROSpecID() == 1) {
		LLRP::CROSpecID *pROSpecID = new LLRP::CROSpecID();
		pROSpecID->setROSpecID(this->_pROSpec->getROSpecID());
		pTagRpt->setROSpecID(pROSpecID);
	}
	if (_pROReportSpec->getTagReportContentSelector()->getEnableSpecIndex() == 1) {
		LLRP::CSpecIndex *pSpecIndex = new LLRP::CSpecIndex();
		pSpecIndex->setSpecIndex(this->_pSpecIndex);
		pTagRpt->setSpecIndex(pSpecIndex);
	}
	if (_pROReportSpec->getTagReportContentSelector()->getEnableInventoryParameterSpecID() == 1) {
		LLRP::CInventoryParameterSpecID *pInveSpecID = new LLRP::CInventoryParameterSpecID();
		pInveSpecID->setInventoryParameterSpecID(pInvSpecID);
		pTagRpt->setInventoryParameterSpecID(pInveSpecID);
	}
	if (_pROReportSpec->getTagReportContentSelector()->getEnableAntennaID() == 1) {
		LLRP::CAntennaID *antID = new LLRP::CAntennaID();
		antID->setAntennaID(pStubTag->_pAntennaID);
		pTagRpt->setAntennaID(antID);
	}
	if (_pROReportSpec->getTagReportContentSelector()->getEnablePeakRSSI() == 1) {
		LLRP::CPeakRSSI *pPeakRSSI = new LLRP::CPeakRSSI();
		pPeakRSSI->setPeakRSSI(pStubTag->_pPeakRSSI);
		pTagRpt->setPeakRSSI(pPeakRSSI);
	}
	if (_pROReportSpec->getTagReportContentSelector()->getEnableChannelIndex() == 1) {
		LLRP::CChannelIndex *pChannelIndex = new LLRP::CChannelIndex();
		pChannelIndex->setChannelIndex(pStubTag->_pChannelIndex);
		pTagRpt->setChannelIndex(pChannelIndex);
	}
	if (_pROReportSpec->getTagReportContentSelector()->getEnableFirstSeenTimestamp() == 1) {
		LLRP::CFirstSeenTimestampUTC *pFirstSeenTimestamp = new LLRP::CFirstSeenTimestampUTC();
		pFirstSeenTimestamp->setMicroseconds(pStubTag->_pFirstSeenTimestamp);
		pTagRpt->setFirstSeenTimestampUTC(pFirstSeenTimestamp);
	}
	if (_pROReportSpec->getTagReportContentSelector()->getEnableLastSeenTimestamp() == 1) {
		LLRP::CLastSeenTimestampUTC *pLastSeenTimestamp = new LLRP::CLastSeenTimestampUTC();
		pLastSeenTimestamp->setMicroseconds(pStubTag->_pLastSeenTimestamp);
		pTagRpt->setLastSeenTimestampUTC(pLastSeenTimestamp);
	}
	if (_pROReportSpec->getTagReportContentSelector()->getEnableTagSeenCount() == 1) {
		LLRP::CTagSeenCount *pTagSeenCount = new LLRP::CTagSeenCount();
		pTagSeenCount->setTagCount(pStubTag->_pTagSeenCount);
		pTagRpt->setTagSeenCount(pTagSeenCount);
	}
	if (_pMemSelector != NULL) {
		if (_pMemSelector->getEnableCRC() == 1) {
			LLRP::CC1G2_CRC *pC1G2CRC = new LLRP::CC1G2_CRC();
			pC1G2CRC->setCRC(pStubTag->_MB[1]->at(0) * 0x100 + pStubTag->_MB[1]->at(1));
			pTagRpt->addAirProtocolTagData(pC1G2CRC);
		}
		if (_pMemSelector->getEnablePCBits() == 1) {
			LLRP::CC1G2_PC *pC1G2PC = new LLRP::CC1G2_PC();
			pC1G2PC->setPC_Bits(pStubTag->_MB[1]->at(2) * 0x100 + pStubTag->_MB[1]->at(3));
			pTagRpt->addAirProtocolTagData(pC1G2PC);
		}
	}

	// AccessSpecID and OpSpecResultList are included after AccessSpecs are applied.

	return pTagRpt;
}

// If there is same TagReport with same EPCID, then replace it.
// If there is no such report, then just add it.
int ELFIN::AIOperation::addTagReportUnique(TagReportSet* pSet,
		LLRP::CTagReportData* pRpt) {
	std::pair<TagReportSet::iterator, bool> result = pSet->insert(pRpt);
	if (result.second == true) {
		return 0;
	}
	else {
		LLRP::CTagReportData *pOldRpt = *(result.first);
		if (pOldRpt->countAccessCommandOpSpecResult() != 0) {
			std::vector<LLRP::CAccessCommandOpSpecResult *> pStayingReportVector;
			// LLRP Spec does not specifies that how many times to apply AccessSpec to each tags.
			// So in this implementation, just apply AccessSpec every round in AIOperation loop,
			// and if there exists same reports or same OpSpecID, then overwrite
			int stayingReport = 1;
			for (std::list<LLRP::CParameter *>::iterator old_iter = pOldRpt->beginAccessCommandOpSpecResult();
					old_iter != pOldRpt->endAccessCommandOpSpecResult(); old_iter++) {
				LLRP::CAccessCommandOpSpecResult *pOldACOpSpecResult = (LLRP::CAccessCommandOpSpecResult *) (*old_iter);
				for (std::list<LLRP::CParameter *>::iterator new_iter = pRpt->beginAccessCommandOpSpecResult();
						new_iter != pRpt->endAccessCommandOpSpecResult(); new_iter++) {
					LLRP::CAccessCommandOpSpecResult *pNewACOpSpecResult = (LLRP::CAccessCommandOpSpecResult *) (*new_iter);
					// If the AccessCommandOpSpecResults in old report are different from existing ones, then add them.
					// But if OpSpecID is same, then ignore them.
					int compareResult = AccessOperation::compareACOpSpecResult(pOldACOpSpecResult, pNewACOpSpecResult);
					if (compareResult == 0) {
						// Two ACOpSpecResults are same, so ignore.
						stayingReport = 0;
					} else if (compareResult != 0 &&
							AccessOperation::getACOpSpecResultOPSpecID(pOldACOpSpecResult) ==
									AccessOperation::getACOpSpecResultOPSpecID(pNewACOpSpecResult)) {
						// Two ACOpSpecResults are different, but OpSpecID is same, so ignore.
						stayingReport = 0;
					}
				}
				if (stayingReport == 1) {
					pOldRpt->removeSubParameterFromAllList((LLRP::CParameter *) pOldACOpSpecResult);
					pStayingReportVector.push_back(pOldACOpSpecResult);
				}
			}
			for (std::vector<LLRP::CAccessCommandOpSpecResult *>::iterator stay_iter = pStayingReportVector.begin();
					stay_iter != pStayingReportVector.end();stay_iter++) {
				pRpt->addAccessCommandOpSpecResult((LLRP::CParameter *) (*stay_iter));
			}
		}

		pSet->erase(result.first);
		delete pOldRpt;
		std::pair<TagReportSet::iterator, bool> result2 = pSet->insert(pRpt);
		if (result2.second == true) {
			return 1;
		}
		else {
			// NOT_REACHED
			return -1;
		}
	}

	return -1;
}

void ELFIN::AIOperation::run_AISpecStopTriggerThread(int stopMS) {
	stopMS -= Utils::getCurrentTimeMilliseconds();
	boost::this_thread::sleep_for(boost::chrono::milliseconds(stopMS));
	READER_LOG (LOGTYPE_DEBUG, "AISpecStopTriggerThread of AISpec in ROSpec %d timeout. StopTrigger is invoked.\n", this->_pROSpec->getROSpecID());
	if (isOnSingulation == 1)
		this->_pReader->InterruptTagSingulation();

}
