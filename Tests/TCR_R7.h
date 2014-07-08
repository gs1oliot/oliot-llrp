/*
 * TCR_R7.h
 *
 *  Created on: Nov 5, 2013
 *      Author: iot-team
 */

#ifndef TCR_R7_H_
#define TCR_R7_H_



#include <boost/thread.hpp>
#include "LLRPTestClient.h"

#define CONST_R1 10000
#define CONST_R6_11 10000
#define CONST_R7_10 20000

class TCR_R7 {
public:

	TCR_R7 (const char *__pReaderHostName, LLRPTestClient* __pTestClient) :
		_pReaderHostName(__pReaderHostName), _pTestClient(__pTestClient),
		_pStartTime(0)
{};
	const char *_pReaderHostName;
	LLRPTestClient* _pTestClient;
	uint64_t _pStartTime;

	int TCR_R7_Prepare (void);
	int TCR_R7_Step1 (void);
	int TCR_R7_Step2 (void);
	int TCR_R7_Step3 (void);
	int TCR_R7_Step4 (void);
	int TCR_R7_Step5 (void);
	int TCR_R7_Step6 (void);
	int TCR_R7_Step7 (void);
	int TCR_R7_Step8 (void);
	int TCR_R7_Step9 (void);
	int TCR_R7_Step10 (void);
	int TCR_R7_Step11 (void);
	int TCR_R7_Step12 (void);
	int TCR_R7_Step13 (void);
	int TCR_R7_Step14 (void);
	int TCR_R7_Step15 (void);
	int TCR_R7_Step16 (void);
};

inline int TCR_R7::TCR_R7_Prepare (void) {
	CMessage *                  pMessage;
	CREADER_EVENT_NOTIFICATION *pNtf;
	CReaderEventNotificationData *pNtfData;
	CConnectionAttemptEvent *   pEvent;

	if (_pTestClient->openConnectionToReader() != 0)
		goto fail;

	/*
	 * Expect the notification within 10 seconds.
	 * It is suppose to be the very first message sent.
	 */
	pMessage = _pTestClient->recvMessage(10000);

	/*
	 * recvMessage() returns NULL if something went wrong.
	 */
	if(NULL == pMessage)
	{
		/* recvMessage already tattled */
		goto fail;
	}

	/*
	 * Check to make sure the message is of the right type.
	 * The type label (pointer) in the message should be
	 * the type descriptor for READER_EVENT_NOTIFICATION.
	 */
	if(&CREADER_EVENT_NOTIFICATION::s_typeDescriptor != pMessage->m_pType)
	{
		goto fail;
	}

	/*
	 * Now that we are sure it is a READER_EVENT_NOTIFICATION,
	 * traverse to the ReaderEventNotificationData parameter.
	 */
	pNtf = (CREADER_EVENT_NOTIFICATION *) pMessage;
	pNtfData = pNtf->getReaderEventNotificationData();
	if(NULL == pNtfData)
	{
		goto fail;
	}

	/*
	 * The ConnectionAttemptEvent parameter must be present.
	 */
	pEvent = pNtfData->getConnectionAttemptEvent();
	if(NULL == pEvent)
	{
		goto fail;
	}

	/*
	 * The status in the ConnectionAttemptEvent parameter
	 * must indicate connection success.
	 */
	if(ConnectionAttemptStatusType_Success != pEvent->getStatus())
	{
		goto fail;
	}

	/*
	 * Done with the message
	 */
	delete pMessage;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R7_Prepare1 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}
	/*
	 * Victory.
	 */
	return 0;

	fail:
	/*
	 * Something went wrong. Tattle. Clean up. Return error.
	 */
	printf("ERROR: checkConnectionStatus failed\n");
	delete pMessage;
	return -1;
}

inline int TCR_R7::TCR_R7_Step1(void) {
	CSET_READER_CONFIG *pCmd;
	CROReportSpec *pRORptSpec;
	CAccessReportSpec *pAccessRptSpec;
	CReaderEventNotificationSpec *pNotiSpec;
	CMessage *pRspMsg;
	CSET_READER_CONFIG_RESPONSE *pRsp;

	/*
	 * Compose the command message
	 */
	pCmd = new CSET_READER_CONFIG();
	pCmd->setMessageID(121);

	pRORptSpec = new CROReportSpec();
	pRORptSpec->setN(0);
	pRORptSpec->setROReportTrigger(LLRP::ROReportTriggerType_Upon_N_Tags_Or_End_Of_ROSpec);
	LLRP::CTagReportContentSelector *pSelector = new LLRP::CTagReportContentSelector();
	pSelector->setEnableAccessSpecID(1);
	pSelector->setEnableAntennaID(1);
	pSelector->setEnableChannelIndex(1);
	pSelector->setEnableFirstSeenTimestamp(1);
	pSelector->setEnableInventoryParameterSpecID(1);
	pSelector->setEnableLastSeenTimestamp(1);
	pSelector->setEnablePeakRSSI(1);
	pSelector->setEnableROSpecID(1);
	pSelector->setEnableSpecIndex(1);
	pSelector->setEnableTagSeenCount(1);
	LLRP::CC1G2EPCMemorySelector *pC1G2Selector = new LLRP::CC1G2EPCMemorySelector();
	pC1G2Selector->setEnableCRC(1);
	pC1G2Selector->setEnablePCBits(1);
	pSelector->addAirProtocolEPCMemorySelector((LLRP::CParameter *) pC1G2Selector);
	pRORptSpec->setTagReportContentSelector(pSelector);
	pCmd->setROReportSpec(pRORptSpec);

	pAccessRptSpec = new CAccessReportSpec();
	pAccessRptSpec->setAccessReportTrigger(LLRP::AccessReportTriggerType_Whenever_ROReport_Is_Generated);
	pCmd->setAccessReportSpec(pAccessRptSpec);

	pNotiSpec = new LLRP::CReaderEventNotificationSpec();
	LLRP::CEventNotificationState *pENState = new LLRP::CEventNotificationState();
	pENState->setEventType(LLRP::NotificationEventType_AISpec_Event);
	pENState->setNotificationState(0);
	pNotiSpec->addEventNotificationState(pENState);
	pENState = new LLRP::CEventNotificationState();
	pENState->setEventType(LLRP::NotificationEventType_AISpec_Event_With_Details);
	pENState->setNotificationState(0);
	pNotiSpec->addEventNotificationState(pENState);
	pENState = new LLRP::CEventNotificationState();
	pENState->setEventType(LLRP::NotificationEventType_Antenna_Event);
	pENState->setNotificationState(0);
	pNotiSpec->addEventNotificationState(pENState);
	pENState = new LLRP::CEventNotificationState();
	pENState->setEventType(LLRP::NotificationEventType_GPI_Event);
	pENState->setNotificationState(0);
	pNotiSpec->addEventNotificationState(pENState);
	pENState = new LLRP::CEventNotificationState();
	pENState->setEventType(LLRP::NotificationEventType_RFSurvey_Event);
	pENState->setNotificationState(0);
	pNotiSpec->addEventNotificationState(pENState);
	pENState = new LLRP::CEventNotificationState();
	pENState->setEventType(LLRP::NotificationEventType_ROSpec_Event);
	pENState->setNotificationState(0);
	pNotiSpec->addEventNotificationState(pENState);
	pENState = new LLRP::CEventNotificationState();
	pENState->setEventType(LLRP::NotificationEventType_Reader_Exception_Event);
	pENState->setNotificationState(0);
	pNotiSpec->addEventNotificationState(pENState);
	pENState = new LLRP::CEventNotificationState();
	pENState->setEventType(LLRP::NotificationEventType_Report_Buffer_Fill_Warning);
	pENState->setNotificationState(0);
	pNotiSpec->addEventNotificationState(pENState);
	pENState = new LLRP::CEventNotificationState();
	pENState->setEventType(LLRP::NotificationEventType_Upon_Hopping_To_Next_Channel);
	pENState->setNotificationState(0);
	pNotiSpec->addEventNotificationState(pENState);
	pENState = new LLRP::CEventNotificationState();
	pENState->setEventType(LLRP::NotificationEventType_SpecLoop_Event);
	pENState->setNotificationState(1);
	pNotiSpec->addEventNotificationState(pENState);

	pCmd->setReaderEventNotificationSpec(pNotiSpec);

	/*
	 * Send the message, expect the response of certain type
	 */
	pRspMsg = _pTestClient->transact(pCmd);

	/*
	 * Done with the command message
	 */
	delete pCmd;

	/*
	 * transact() returns NULL if something went wrong.
	 */
	if(NULL == pRspMsg)
	{
		/* transact already tattled */
		return -1;
	}

	/*
	 * Cast to a GET_READER_CONFIG_RESPONSE message.
	 */
	pRsp = (CSET_READER_CONFIG_RESPONSE *) pRspMsg;

	/*
	 * Check the LLRPStatus parameter.
	 */
	if(0 != _pTestClient->checkLLRPStatus(pRsp->getLLRPStatus(), "getLLRPConfigurationStateValue"))
	{
		/* checkLLRPStatus already tattled */
		delete pRspMsg;
		return -1;
	}

	/*
	 * Done with the response message.
	 */
	delete pRspMsg;

	/*
	 * Tattle progress, maybe
	 */
	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R7_Step1 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	/*
	 * Victory.
	 */
	return 0;
}


inline int TCR_R7::TCR_R7_Step2(void) {
	CADD_ROSPEC *               pCmd;
	CROSpec *					pBasicROSpec;
	CAISpec *					pBasicAISpec;
	CLoopSpec *					pLoopSpec;
	CMessage *                  pRspMsg;
	CADD_ROSPEC_RESPONSE *      pRsp;

	pBasicROSpec = _pTestClient->getBasicROSpec();
	pBasicAISpec = _pTestClient->getBasicAISpec();
	// This duration trigger is not defined on conformance document.
	// But without this, the ROSpec never finishes, so it seems the bug of the document.
	pBasicAISpec->getAISpecStopTrigger()->setAISpecStopTriggerType(LLRP::AISpecStopTriggerType_Duration);
	pBasicAISpec->getAISpecStopTrigger()->setDurationTrigger(3000);
	pBasicROSpec->addSpecParameter(pBasicAISpec);

	pLoopSpec = new CLoopSpec();
	pLoopSpec->setLoopCount(1);
	pBasicROSpec->addSpecParameter(pLoopSpec);
	pCmd = new CADD_ROSPEC();
	pCmd->setMessageID(201);
	pCmd->setROSpec(pBasicROSpec);

	/*
	 * Send the message, expect the response of certain type
	 */
	pRspMsg = _pTestClient->transact(pCmd);

	/*
	 * Done with the command message.
	 * N.B.: And the parameters
	 */
	delete pCmd;

	/*
	 * transact() returns NULL if something went wrong.
	 */
	if(NULL == pRspMsg)
	{
		/* transact already tattled */
		return -1;
	}

	/*
	 * Cast to a ADD_ROSPEC_RESPONSE message.
	 */
	pRsp = (CADD_ROSPEC_RESPONSE *) pRspMsg;

	/*
	 * Check the LLRPStatus parameter.
	 */
	if(0 != _pTestClient->checkLLRPStatus(pRsp->getLLRPStatus(), "addROSpec"))
	{
		/* checkLLRPStatus already tattled */
		delete pRspMsg;
		return -1;
	}

	/*
	 * Done with the response message.
	 */
	delete pRspMsg;

	/*
	 * Tattle progress, maybe
	 */
	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R7_Step2 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	/*
	 * Victory.
	 */
	return 0;
}

inline int TCR_R7::TCR_R7_Step3(void) {
	if (_pTestClient->enableROSpec(123, "TCR_R7_Step3") != 0)
		return -1;
	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R7_Step3 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}
	return 0;
}

inline int TCR_R7::TCR_R7_Step4(void) {
	_pStartTime = _pTestClient->getCurrentTimeMilliseconds();
	if (_pTestClient->startROSpec(123, "TCR_R7_Step4") != 0)
		return -1;
	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R7_Step4 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}
	return 0;
}

inline int TCR_R7::TCR_R7_Step5(void) {
	CREADER_EVENT_NOTIFICATION *pRsp;
	CReaderEventNotificationData *pNtfData;
	LLRP::CSpecLoopEvent *		pEvent;
	CRO_ACCESS_REPORT *			pReport;

	// Wait for R6.11(default=10) seconds.
	printf ("Waiting for R6.11(default=10) seconds.\n");
	boost::this_thread::sleep_for(boost::chrono::milliseconds(CONST_R6_11));

	// Check Reader event notification
	pRsp = _pTestClient->recvNotification(0, "TCR_R7_Step5");
	if (pRsp == NULL)
	{
		goto fail;
	}
	pNtfData = pRsp->getReaderEventNotificationData();
	if(NULL == pNtfData)
	{
		goto fail;
	}

	pEvent = pNtfData->getSpecLoopEvent();

	if(NULL == pEvent)
	{
		goto fail;
	}

	if(1 != pEvent->getLoopCount()
			|| 123 != pEvent->getROSpecID())
	{
		goto fail;
	}

	delete pRsp;

	// Check RO_ACCESS_REPORT
	pReport = _pTestClient->getROAccessReport(0, "TCR_R7_Step5");
	if (pReport == NULL)
		return -1;
	_pTestClient->printTagReportData(pReport);
	delete pReport;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R7_Step5 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}
	return 0;

	fail:
	printf("ERROR\n");
	delete pRsp;
	return -1;

}

inline int TCR_R7::TCR_R7_Step6(void) {
	// Delete ROSpec
	if (_pTestClient->deleteROSpec(123, "TCR_R7_Step6") != 0)
		return -1;
	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R7_Step6 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}
	return 0;
}

inline int TCR_R7::TCR_R7_Step7(void) {
	CADD_ROSPEC *               pCmd;
	CROSpec *					pBasicROSpec;
	CAISpec *					pBasicAISpec;
	CLoopSpec *					pLoopSpec;
	CMessage *                  pRspMsg;
	CADD_ROSPEC_RESPONSE *      pRsp;

	pBasicROSpec = _pTestClient->getBasicROSpec();
	pBasicAISpec = _pTestClient->getBasicAISpec();
	// This duration trigger is not defined on conformance document.
	// But without this, the ROSpec never finishes, so it seems the bug of the document.
	pBasicAISpec->getAISpecStopTrigger()->setAISpecStopTriggerType(LLRP::AISpecStopTriggerType_Duration);
	pBasicAISpec->getAISpecStopTrigger()->setDurationTrigger(3000);
	pBasicROSpec->addSpecParameter(pBasicAISpec);
	pLoopSpec = new CLoopSpec();
	pLoopSpec->setLoopCount(0);
	pBasicROSpec->addSpecParameter(pLoopSpec);
	pCmd = new CADD_ROSPEC();
	pCmd->setMessageID(201);
	pCmd->setROSpec(pBasicROSpec);

	/*
	 * Send the message, expect the response of certain type
	 */
	pRspMsg = _pTestClient->transact(pCmd);

	/*
	 * Done with the command message.
	 * N.B.: And the parameters
	 */
	delete pCmd;

	/*
	 * transact() returns NULL if something went wrong.
	 */
	if(NULL == pRspMsg)
	{
		/* transact already tattled */
		return -1;
	}

	/*
	 * Cast to a ADD_ROSPEC_RESPONSE message.
	 */
	pRsp = (CADD_ROSPEC_RESPONSE *) pRspMsg;

	/*
	 * Check the LLRPStatus parameter.
	 */
	if(0 != _pTestClient->checkLLRPStatus(pRsp->getLLRPStatus(), "addROSpec"))
	{
		/* checkLLRPStatus already tattled */
		delete pRspMsg;
		return -1;
	}

	/*
	 * Done with the response message.
	 */
	delete pRspMsg;

	/*
	 * Tattle progress, maybe
	 */
	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R7_Step7 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	/*
	 * Victory.
	 */
	return 0;
}

inline int TCR_R7::TCR_R7_Step8(void) {
	if (_pTestClient->enableROSpec(123, "TCR_R7_Step8") != 0)
		return -1;
	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R7_Step8 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}
	return 0;
}

inline int TCR_R7::TCR_R7_Step9(void) {
	if (_pTestClient->startROSpec(123, "TCR_R7_Step9") != 0)
		return -1;
	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R7_Step9 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}
	_pStartTime = _pTestClient->getCurrentTimeMilliseconds();
	return 0;
}

inline int TCR_R7::TCR_R7_Step10(void) {
	CREADER_EVENT_NOTIFICATION *pRsp;
	CReaderEventNotificationData *pNtfData;
	LLRP::CSpecLoopEvent *		pEvent;

	// Wait for R7.10(default=20) seconds.
	printf ("Waiting for R7.10(default=20) seconds.\n");
	boost::this_thread::sleep_for(boost::chrono::milliseconds(CONST_R7_10));

	int goFail = 0;
	unsigned int notiCount = 0;
	while (1) {
		// Check Reader event notification
		pRsp = _pTestClient->recvNotification(0, "TCR_R7_Step10");
		if (pRsp == NULL)
		{
			// If there were less than 2 notification, its failure.
			if (notiCount < 2) {
				printf ("INFO: 1 or less SpecLoopEvent is received. Fail.\n");
				goFail = 1;
			}
			else {
				printf ("INFO: More than 2 SpecLoopEvents are received successfully.\n");
			}
			break;
		}
		notiCount++;
		pNtfData = pRsp->getReaderEventNotificationData();
		if(NULL == pNtfData)
		{
			goFail = 1;
			break;
		}

		pEvent = pNtfData->getSpecLoopEvent();

		if(NULL == pEvent)
		{
			goFail = 1;
			break;
		}

		if(notiCount != pEvent->getLoopCount()
				|| 123 != pEvent->getROSpecID())
		{
			goFail = 1;
			break;
		}

		delete pRsp;
	}
	if (goFail == 1) {
		printf("ERROR\n");
		delete pRsp;
		return -1;
	}
	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R7_Step10 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}
	return 0;
}

inline int TCR_R7::TCR_R7_Step11(void) {
	CRO_ACCESS_REPORT *			pReport;
	if (_pTestClient->stopROSpec(123, "TCR_R7_Step11") != 0) {
		return -1;
	}

	// Check RO_ACCESS_REPORT
	pReport = _pTestClient->getROAccessReport(10000, "TCR_R7_Step11");
	if (pReport == NULL)
		return -1;
	_pTestClient->printTagReportData(pReport);
	delete pReport;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R7_Step11 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}
	return 0;
}

inline int TCR_R7::TCR_R7_Step12(void) {
	if (_pTestClient->deleteROSpec(123, "TCR_R7_Step12") != 0)
		return -1;
	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R7_Step12 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}
	return 0;
}

#endif /* TCR_R7_H_ */
