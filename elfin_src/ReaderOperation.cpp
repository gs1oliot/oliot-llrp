/**
 * @file    ReaderOperation.cpp
 * @brief
 */

#include "RFSurveyOperation.h"
#include "AIOperation.h"
#include "LLRPReader_Configurations.h"
#include "EventNotifier.h"
#include "ReaderOperation.h"


ELFIN::ReaderOperation::ReaderOperation(LLRP::CROSpec *aROSpec, LLRPCore *__pLLRPCore)
:  _pReader(__pLLRPCore->m_physicalReader), _CROSpec(aROSpec), _pLLRPCore(__pLLRPCore),
   _pConfig(__pLLRPCore->m_LLRPConfig), _pROAdmin(__pLLRPCore->m_ROAdmin), _pROSpecStopTriggerInvoked(0),
   _pLoopSpecInfinity(0), _pStopping(0), _pLoopSpecLoopCount(0), _pPendingReport(NULL) {
	_pScheduler = new Scheduler(_ios);


	if (aROSpec->getROReportSpec() != NULL) {
		_pROReportSpec = aROSpec->getROReportSpec();
	}
	else {
		_pROReportSpec = _pConfig->_pDefaultROReportSpec;
	}

	int specIndex = 1;
	int loopSpecExists = 0;
	int antSpecExists = 0;
	LLRP::CLoopSpec *pLoopSpec = NULL;
	for (std::list<LLRP::CParameter *>::iterator iter = aROSpec->beginSpecParameter();
			iter != aROSpec->endSpecParameter();iter++) {
		if ((*iter)->m_pType == &LLRP::CRFSurveySpec::s_typeDescriptor) {
			antSpecExists = 1;
			CRFSurveySpec *pSpec = (CRFSurveySpec *) (*iter);
			RFSurveyOperation *pOper = new RFSurveyOperation (pSpec, _pReader, this, specIndex);
			this->_absAntennaOps[specIndex] = ((AbstractAntennaOperation*) pOper);
		}
		else if ((*iter)->m_pType == &LLRP::CAISpec::s_typeDescriptor) {
			antSpecExists = 1;
			LLRP::CAISpec *pSpec = (LLRP::CAISpec *) (*iter);
			AIOperation *pOper = new AIOperation (aROSpec, pSpec, _pReader, this, _pLLRPCore->m_AOAdmin, specIndex);
			this->_absAntennaOps[specIndex] = (AbstractAntennaOperation*) pOper;
		}
		else if ((*iter)->m_pType == &LLRP::CLoopSpec::s_typeDescriptor) {
			loopSpecExists++;
			pLoopSpec = (LLRP::CLoopSpec *) (*iter);
			specIndex--; // LoopSpec should be added to the last position of the specs.
		}
		else {
			//Not reached
			throw "error";
		}
		specIndex++;
	}

	_pLoopSpecInfinity = 0;
	_pLoopSpecLoopCount = 0;
	/// @todo  Handle the errors related to illegal LoopSpec
	if (antSpecExists == 0 && loopSpecExists == 1) { // This is illegal
		READER_LOG (LOGTYPE_ERROR, "LoopSpec cannot exist without other specs.\n");
	} else if (loopSpecExists > 1) { // LoopSpec cannot be exist more than two.
		READER_LOG (LOGTYPE_ERROR, "Only one or no loopSpec can exist in a single ROSpec. There are %d currently.\n", loopSpecExists);
	} else if (antSpecExists != 0 && loopSpecExists == 1) {
		if (pLoopSpec->getLoopCount() == 0) {
			_pLoopSpecInfinity = 1;
			_pLoopSpecLoopCount = 0;
		}
		else {
			_pLoopSpecInfinity = 0;
			_pLoopSpecLoopCount = pLoopSpec->getLoopCount();
		}
	} else {

	}
}

ELFIN::ReaderOperation::~ReaderOperation() {
	READER_LOG (LOGTYPE_TRACE, "Destroying ReaderOperation...\n");
	if (_pStopping == 0) {
		stopRO();
	}
	else {
		if (_pThread.joinable())
			_pThread.join();
		if (_pROSpecStopTriggerThread.joinable())
			_pROSpecStopTriggerThread.join();
	}
	for (AbsAntennaOpsMap::iterator iter = _absAntennaOps.begin();
			iter != _absAntennaOps.end();iter++) {
		if ((*iter).second->_pTypeDescriptor == &LLRP::CAISpec::s_typeDescriptor) {
			AIOperation *pOper = (AIOperation *) ((*iter).second);
			delete pOper;
		} else if ((*iter).second->_pTypeDescriptor == &LLRP::CRFSurveySpec::s_typeDescriptor) {
			RFSurveyOperation *pOper = (RFSurveyOperation *) ((*iter).second);
			delete pOper;
		}
	}

	if (_pPendingReport != NULL)
		delete _pPendingReport;

	_ios.stop();
	delete _pScheduler;
}


void ELFIN::ReaderOperation::run() {
	READER_LOG(LOGTYPE_INFO, "ROSpec is going active state\n");
	_CROSpec->setCurrentState(LLRP::ROSpecState_Active);
	_pROSpecStopTriggerInvoked = 0;
	_pThread = boost::thread(boost::bind(&ELFIN::ReaderOperation::runRO, this));
}

void ELFIN::ReaderOperation::runRO() {
	_pScheduler->run();
	READER_LOG(LOGTYPE_INFO, "ROSpec is going inactive state\n");
	_CROSpec->setCurrentState(LLRP::ROSpecState_Inactive);
}

void ELFIN::ReaderOperation::schedule() {
	READER_LOG(LOGTYPE_INFO, "Scheduling ROSpec %d. %d specs are currently in the absAntennaOps\n", this->_CROSpec->getROSpecID(), _absAntennaOps.size());
	int ROSpecStartTriggerType = _CROSpec->getROBoundarySpec()->getROSpecStartTrigger()->getROSpecStartTriggerType();
	if (ROSpecStartTriggerType == LLRP::ROSpecStartTriggerType_Null) {
		/* ROSpecStartTriggerType_Null is only be executed when START_ROSPEC in invoked.
		 * So, schedule as follows and run the thread when START_ROSPEC message is received.
		 */
		_pScheduler->schedule(this, 0, 1);
	} else if (ROSpecStartTriggerType == LLRP::ROSpecStartTriggerType_Periodic) {
		LLRP::llrp_u32_t period = _CROSpec->getROBoundarySpec()->getROSpecStartTrigger()->getPeriodicTriggerValue()->getPeriod();
		LLRP::llrp_u32_t offset = _CROSpec->getROBoundarySpec()->getROSpecStartTrigger()->getPeriodicTriggerValue()->getOffset();
		//READER_LOG (LOGTYPE_DEBUG, "Offset is set to %d\n", offset);
		LLRP::CUTCTimestamp *pUTCTimestamp = _CROSpec->getROBoundarySpec()->getROSpecStartTrigger()->getPeriodicTriggerValue()->getUTCTimestamp();
		if (pUTCTimestamp != NULL) {
			LLRP::CUTCTimestamp *pUTCTimestamp_current = ELFIN::Utils::getCurrentUTCTimestamp();
			LLRP::llrp_u64_t pUTCMicro = pUTCTimestamp->getMicroseconds();
			LLRP::llrp_u64_t pUTCMicro_current = pUTCTimestamp_current->getMicroseconds();
			offset += (pUTCMicro - pUTCMicro_current) / 1000;
			//READER_LOG ("LOGTYPE_DEBUG, "Offset is set to %d, by UTCTimestamp\n", offset);
			delete pUTCTimestamp_current;
		}
		if (period == 0) {
			_pScheduler->schedule(this, 0, 1, offset);
		}
		else {
			/// @fixme  change the sequence of parameters, to remove TimerTask::COUNT_INFINITY
			_pScheduler->schedule(this, period, TimerTask::COUNT_INFINITY, offset);
		}
		// Periodic trigger ROSpec starts immediately after the enable.
		this->run();
	} else if (ROSpecStartTriggerType == LLRP::ROSpecStartTriggerType_Immediate) {
		_pScheduler->schedule(this, 0, 1);
		// Immediate trigger ROSpec starts immediately after the enable.
		this->run();
	} else if (ROSpecStartTriggerType == LLRP::ROSpecStartTriggerType_GPI) {
		_pScheduler->schedule(this, 0, 1);
	}
}

void ELFIN::ReaderOperation::handle_timeout(const boost::system::error_code& e) {
	Utils::startTimeCount(this->_pReader->m_NumberOfStubTagsPerAntenna, this->_pReader->m_AntennaMap.size());
	READER_LOG (LOGTYPE_DEBUG, "Scheduler invoked ReaderOperation %d.\n", this->_CROSpec->getROSpecID());
	if (e != boost::asio::error::operation_aborted) {
		// Because the start trigger is invoked, send ROSpecEvent as Start_Of_ROSpec.
		_pLLRPCore->m_EventNotifier->sendROSpecEvent(
				_CROSpec->getROSpecID(), LLRP::ROSpecEventType_Start_Of_ROSpec, 0);
		// Set the stop trigger
		if (this->_CROSpec->getROBoundarySpec()->getROSpecStopTrigger()->getROSpecStopTriggerType()
				== LLRP::ROSpecStopTriggerType_Null) {
			// DO NOTHING
		} else if (this->_CROSpec->getROBoundarySpec()->getROSpecStopTrigger()->getROSpecStopTriggerType()
				== LLRP::ROSpecStopTriggerType_Duration) {
			int stopMS = this->_CROSpec->getROBoundarySpec()->getROSpecStopTrigger()->getDurationTriggerValue();
			_pROSpecStopTriggerThread = boost::thread(
					boost::bind(&ELFIN::ReaderOperation::run_ROSpecStopTriggerThread, this, stopMS));
		} else if (this->_CROSpec->getROBoundarySpec()->getROSpecStopTrigger()->getROSpecStopTriggerType()
				== LLRP::ROSpecStopTriggerType_GPI_With_Timeout) {
			assert (this->_CROSpec->getROBoundarySpec()->getROSpecStopTrigger()->getGPITriggerValue());
			LLRP::CGPITriggerValue *pGPIValue = this->_CROSpec->getROBoundarySpec()->getROSpecStopTrigger()->getGPITriggerValue();
			int stopMS = pGPIValue->getTimeout();
			if (stopMS != 0) {
				_pROSpecStopTriggerThread = boost::thread(
						boost::bind(&ELFIN::ReaderOperation::run_ROSpecStopTriggerThread, this, stopMS));
			}
		} else {
			throw "error";
		}

		// Start reader operation
		LLRP::CRO_ACCESS_REPORT *pRpt = new LLRP::CRO_ACCESS_REPORT();
		unsigned int SpecIndex = 1;
		LLRP::llrp_u32_t LoopCountLeft = _pLoopSpecLoopCount;
		LLRP::llrp_u32_t LoopCount = 0;
		TagReportSet *pROTagReportSet = new TagReportSet();
		// Because specIndex starts from 1, loop until size + 1
		while (SpecIndex < this->_absAntennaOps.size() + 1) {
			if (this->_absAntennaOps.at(SpecIndex)->_pTypeDescriptor == &LLRP::CAISpec::s_typeDescriptor) {
				AIOperation *pAIOperation = (AIOperation *)this->_absAntennaOps.at(SpecIndex);
				TagReportSet *pAITagReportSet = pAIOperation->run();
				for (TagReportSet::iterator ai_rpt_iter = pAITagReportSet->begin();
						ai_rpt_iter != pAITagReportSet->end(); ai_rpt_iter++) {
					pAIOperation->addTagReportUnique(pROTagReportSet, (*ai_rpt_iter));
				}
				_pLLRPCore->m_EventNotifier->sendAISpecEvent(_CROSpec->getROSpecID(), SpecIndex, LLRP::AISpecEventType_End_Of_AISpec);
				// If the ROReportTrigger is set as follows, need to generate RO_ACCESS_REPORT and send it.
				// Remove all reports that is sent here, from the _pTagReportSet of AIOperation.
				// First, send RO_ACCESS_REPORT if the overall tag reports exceeds pNTags
				/// @todo  Add support for time based ROReportSpec
				if (_pROReportSpec->getROReportTrigger() == LLRP::ROReportTriggerType_Upon_N_Tags_Or_End_Of_AISpec_Or_End_Of_RFSurveySpec ||
						_pROReportSpec->getROReportTrigger() == LLRP::ROReportTriggerType_Upon_N_Tags_Or_End_Of_ROSpec) {
					int pNTags = _pROReportSpec->getN();
					if (pNTags == 0) {
						pNTags = 999999; // Actually, this should be 'infinity'.
					}
					int pROTagReportSetSize = static_cast<int>(pROTagReportSet->size());
					while (pROTagReportSetSize >= pNTags) {
						READER_LOG (LOGTYPE_DEBUG, "ROReportTrigger: %d tags, Currently stored: %d tags\n", pROTagReportSetSize, pNTags);
						LLRP::CRO_ACCESS_REPORT *pNTagRpt = this->getCurrentReport(pROTagReportSet, pROTagReportSet, pNTags);
						// If that report was gathered this time, erase it.
						// If the ROReportTrigger is ROReportTriggerType_Upon_N_Tags_Or_End_Of_AISpec,
						// remaining tags would be sent.
						for (std::list<LLRP::CTagReportData *>::iterator iter = pNTagRpt->beginTagReportData();
								iter != pNTagRpt->beginTagReportData(); iter++) {
							TagReportSet::iterator rpt_iter = pAITagReportSet->find(*iter);
							if (rpt_iter != pAITagReportSet->end()) {
								pAITagReportSet->erase(rpt_iter);
							}
						}
						_pLLRPCore->sendMessage(pNTagRpt);
						delete pNTagRpt;
						pROTagReportSetSize -= pNTags;
					}

				}

				// Second, send RO_ACCESS_REPORT if the ROReport should be sent at the end of AISpec
				if (_pROReportSpec->getROReportTrigger() == LLRP::ROReportTriggerType_Upon_N_Tags_Or_End_Of_AISpec_Or_End_Of_RFSurveySpec) {
					LLRP::CRO_ACCESS_REPORT *pAIRpt = this->getCurrentReport(pAITagReportSet, pROTagReportSet, 0);
					/// @fixme  Should we send callback to StubReaderApp here?
					_pLLRPCore->sendMessage(pAIRpt);
					delete pAIRpt;
				}
				// This is pointer to the global variable of AIOperation. Do not delete it.
				pAITagReportSet->clear();

			}
			else if (this->_absAntennaOps.at(SpecIndex)->_pTypeDescriptor == &LLRP::CRFSurveySpec::s_typeDescriptor) {
				RFSurveyOperation *pRFSurveyOperation = (RFSurveyOperation *)this->_absAntennaOps.at(SpecIndex);
				pRFSurveyOperation->run();
			}
			else {
				// not reached
				throw "error";
			}
			SpecIndex++;

			// If the current end condition meets, then decrease LoopCount.
			if (!(SpecIndex < this->_absAntennaOps.size() + 1)) {
				LoopCount++;
				if (this->_pLoopSpecInfinity == 1) {
					SpecIndex = 1;
					_pLLRPCore->m_EventNotifier->sendSpecLoopEvent(this->_CROSpec->getROSpecID(), LoopCount);
				} else if (LoopCountLeft > 0) {
					LoopCountLeft--;
					SpecIndex = 1;
					_pLLRPCore->m_EventNotifier->sendSpecLoopEvent(this->_CROSpec->getROSpecID(), LoopCount);
				} else {
					// end of loop
				}
			}

			// When ROSpecStopTrigger is invoked, stop the loop.
			if (this->_pROSpecStopTriggerInvoked == 1) {
				READER_LOG (LOGTYPE_DEBUG, "ROSpecStopTrigger of ROSpec %d is invoked\n", this->_CROSpec->getROSpecID());
				_pROSpecStopTriggerThread.interrupt();
				_pROSpecStopTriggerThread.join();
				break;
			}
		}
		// Because the ROSpec is finished, send ROSpecEvent as End_Of_ROspec.
		_pLLRPCore->m_EventNotifier->sendROSpecEvent(
				_CROSpec->getROSpecID(), LLRP::ROSpecEventType_End_Of_ROSpec, 0);

		for (TagReportSet::iterator ro_rpt_iter = pROTagReportSet->begin();
				ro_rpt_iter != pROTagReportSet->end(); ro_rpt_iter++) {
			pRpt->addTagReportData(*ro_rpt_iter);
		}
		delete pROTagReportSet;

		SpecIndex = 1;
		while (SpecIndex < this->_absAntennaOps.size() + 1) {
			if (this->_absAntennaOps.at(SpecIndex)->_pTypeDescriptor == &LLRP::CAISpec::s_typeDescriptor) {

			}
			else if (this->_absAntennaOps.at(SpecIndex)->_pTypeDescriptor == &LLRP::CRFSurveySpec::s_typeDescriptor) {
				RFSurveyOperation *pRFSurveyOperation = (RFSurveyOperation *)this->_absAntennaOps.at(SpecIndex);
				pRFSurveyOperation->run();
				// generate RF_Survey report. Because it is optional feature, omitted.
			}
			else {
				// not reached
				throw "error";
			}
			SpecIndex++;
		}


		TagVector *pReportedTagVector = new TagVector();
		pReportedTagVector->resize(pRpt->countTagReportData(), NULL);
		int i = 0;
		for (std::list<LLRP::CTagReportData *>::iterator rpt_Iter = pRpt->beginTagReportData();
				rpt_Iter != pRpt->endTagReportData();rpt_Iter++) {
			LLRP::CTagReportData *pTagRpt = (*rpt_Iter);
			LLRP::CEPC_96 *pEPCData = (LLRP::CEPC_96 *) pTagRpt->getEPCParameter();
			StubTag *pTag = new StubTag(pTagRpt->getAntennaID()->getAntennaID());
			// Inventory result may not contain PC and CRC, so set it to 0
			pTag->_MB[1]->push_back(0x00);
			pTag->_MB[1]->push_back(0x00);
			pTag->_MB[1]->push_back(0x00);
			pTag->_MB[1]->push_back(0x00);
			// Set contents of memory bank 1 (EPC bank)
			for (int j = 0;j < 12;j++) { // first 16-bits are PC and CRC
				pTag->_MB[1]->push_back(pEPCData->getEPC().m_aValue[j]);
			}
			pTag->setLogEPCStr();

			//pReportedTagVector->push_back(pTag);
			pReportedTagVector->at(i) = pTag;
			i++;
		}


		int sendResult = -1;
		// Third, send RO_ACCESS_REPORT if the ROReport should be sent at the end of ROSpec
		if (_pROReportSpec->getROReportTrigger() == LLRP::ROReportTriggerType_Upon_N_Tags_Or_End_Of_ROSpec) {
			READER_LOG (LOGTYPE_DEBUG, "Sending %d tag data reports.\n", pRpt->countTagReportData());
			sendResult = _pLLRPCore->sendMessage(pRpt);
			Utils::printCountedTime("TagReadStop-ReportSend");
			Utils::resetTimeCount();
			delete pRpt;
			pRpt = NULL;
		} else if (_pROReportSpec->getROReportTrigger() == LLRP::ROReportTriggerType_None) {
			READER_LOG (LOGTYPE_DEBUG, "Pending %d tag data reports.\n", pRpt->countTagReportData());
			/// @todo  Check whether we should store all reports, or only latest one.
			/// Currently, only latest one is stored.
			if (this->_pPendingReport != NULL) {
				delete this->_pPendingReport;
			}
			this->_pPendingReport = pRpt;
		} else if (_pROReportSpec->getROReportTrigger() == LLRP::ROReportTriggerType_Upon_N_Tags_Or_End_Of_AISpec_Or_End_Of_RFSurveySpec){
			if (pRpt->countTagReportData() > 0) {
				int tagRptCount = static_cast<int>(pRpt->countTagReportData());
				READER_LOG (LOGTYPE_ERROR, "%d reports are left after finishing all AI Operation!!!\n", tagRptCount);
			}
		} else {
			//NOT_REACHED
			throw "error";
		}

		// If this ROSpec is triggered by the GPI Event, then inform the StubApp that the tag singulation is finished.
		if (sendResult == 0) {
			if (this->_CROSpec->getROBoundarySpec()->getROSpecStartTrigger()->getROSpecStartTriggerType() == LLRP::ROSpecStartTriggerType_GPI) {
				if (_pReader->m_StubApp != NULL)
					_pReader->m_StubApp->GPITriggeredInventoryFinishedCallback(this->_CROSpec->getROSpecID(), pReportedTagVector);
			}
			else {
				if (_pReader->m_StubApp != NULL)
					_pReader->m_StubApp->InventoryOperationFinishedCallback(this->_CROSpec->getROSpecID(), pReportedTagVector);
			}
			for (TagVector::iterator tag_iter = pReportedTagVector->begin();
					tag_iter != pReportedTagVector->end();tag_iter++) {
				delete (*tag_iter);
			}
			delete pReportedTagVector;
		}
	}
	else
	{
		READER_LOG(LOGTYPE_INFO, "ReaderOperation for ROSpec ID %d is interrupted\n", this->_CROSpec->getROSpecID());
	}
}
void ELFIN::ReaderOperation::handle_timeout_finalize(const boost::system::error_code& e) {

}

// Stop the timertask immediately with the interrupt.
void ELFIN::ReaderOperation::stopRO() {
	_pStopping = 1;
	_pROSpecStopTriggerInvoked = 1;
	this->_pReader->InterruptTagSingulation();
	this->cancel();
	READER_LOG (LOGTYPE_TRACE, "Joining ro->_pThread...\n");
	_pThread.join();
	READER_LOG (LOGTYPE_TRACE, "Joined ro->_pThread...\n");
	READER_LOG (LOGTYPE_TRACE, "Joining ro->_pROSpecStopTriggerThread...\n");
	_pROSpecStopTriggerThread.join();
	READER_LOG (LOGTYPE_TRACE, "Joined ro->_pROSpecStopTriggerThread...\n");
	_pStopping = 0;
}


// Stop the timertask, wait for termination, and then delete ReaderOpertaion object
void ELFIN::ReaderOperation::deleteRO(ReaderOperation *pRO) {
	pRO->_pStopping = 1;
	pRO->cancel();
	pRO->stopRO();

	delete pRO;
}

void ELFIN::ReaderOperation::run_ROSpecStopTriggerThread(int stopMS) {
	boost::this_thread::sleep_for(boost::chrono::milliseconds(stopMS));
	READER_LOG (LOGTYPE_DEBUG, "ROSpecStopTriggerThread of ROSpec %d timeout. StopTrigger is invoked.\n", this->_CROSpec->getROSpecID());
	_pROSpecStopTriggerInvoked = 1;
	this->_pReader->InterruptTagSingulation();
	//stopRO();
	//_pROSpecStopTriggerInvoked = 1;
}

LLRP::CRO_ACCESS_REPORT *ELFIN::ReaderOperation::getCurrentReport(
		TagReportSet* pTagReportSet, TagReportSet *pUpperTagReportSet, int tagCount) {
	LLRP::CRO_ACCESS_REPORT *pCurrentRpt = new LLRP::CRO_ACCESS_REPORT();
	// If tagCount is 0, then assume it as unlimited.
	int tagReportCount = (tagCount == 0) ? 999999 : tagCount;
	TagReportSet *pUsedTagReportSet = new TagReportSet();
	for (TagReportSet::iterator rpt_iter = pTagReportSet->begin();
			rpt_iter != pTagReportSet->end(); rpt_iter++) {
		pUsedTagReportSet->insert(*rpt_iter);
		pCurrentRpt->addTagReportData(*rpt_iter);
		tagReportCount--;
		if (tagReportCount <= 0) {
			break;
		}
	}
	for (TagReportSet::iterator used_rpt_iter = pUsedTagReportSet->begin();
			used_rpt_iter != pUsedTagReportSet->end(); used_rpt_iter++) {
		// remove used tag reports from the pUpperTagReportSet.
		TagReportSet::iterator rpt_iter = pUpperTagReportSet->find(*used_rpt_iter);
		if (rpt_iter != pUpperTagReportSet->end()) {
			pUpperTagReportSet->erase(rpt_iter);
		} else {
			// NOT_REACHED
			// If reached, why used report does not exist in pROTagReportSet?
			assert (0);
		}
	}
	delete pUsedTagReportSet;
	return pCurrentRpt;
}
