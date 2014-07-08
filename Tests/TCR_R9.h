/*
 * TCR_R9.h
 *
 *  Created on: Nov 5, 2013
 *      Author: iot-team
 */

#ifndef TCR_R9_H_
#define TCR_R9_H_



#include <boost/thread.hpp>
#include "LLRPTestClient.h"


#define CONST_TAG_PREPARATION 10000
#define CONST_R9_2 30000
#define CONST_R9_5 40000
#define CONST_R9_7 30000
#define CONST_R9_10 30000

class TCR_R9 {
public:

	TCR_R9 (const char *__pReaderHostName, LLRPTestClient* __pTestClient) :
		_pReaderHostName(__pReaderHostName), _pTestClient(__pTestClient),
		_pStartTime(0)
{};
	const char *_pReaderHostName;
	LLRPTestClient* _pTestClient;
	uint64_t _pStartTime;

	int TCR_R9_Prepare (void);
	int TCR_R9_Step1 (void);
	int TCR_R9_Step2 (void);
	int TCR_R9_Step3 (void);
	int TCR_R9_Step4 (void);
	int TCR_R9_Step5 (void);
	int TCR_R9_Step6 (void);
	int TCR_R9_Step7 (void);
	int TCR_R9_Step8 (void);
	int TCR_R9_Step9 (void);
	int TCR_R9_Step10 (void);
};

inline int TCR_R9::TCR_R9_Prepare (void) {
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
		printf("INFO: Connection status OK\n");
	}

	printf ("INFO: ====================================================================================\n");
	printf ("INFO: If the reader is wrapper mode, remove all tags from the FOV of reader in 10 seconds.\n");
	printf ("INFO: ====================================================================================\n");
	boost::this_thread::sleep_for(boost::chrono::milliseconds(CONST_TAG_PREPARATION / 2));
	// remove all tags from FOV of antenna.
	if (_pTestClient->sendCloseConnectionWithCustomCommand(LLRPTestClient::CLOSECONNECTION_CUSTOM_COMMAND_PLACE_0_TAGS) != 0)
		return -1;
	boost::this_thread::sleep_for(boost::chrono::milliseconds(CONST_TAG_PREPARATION / 2));

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R9_Prepare1 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	return 0;


	fail:
	/*
	 * Something went wrong. Tattle. Clean up. Return error.
	 */
	printf("ERROR: checkConnectionStatus failed\n");
	delete pMessage;
	return -1;
}

inline int TCR_R9::TCR_R9_Step1(void) {
	CSET_READER_CONFIG *pCmd;
	CROReportSpec *pRORptSpec;
	CAccessReportSpec *pAccessRptSpec;
	CReaderEventNotificationSpec *pNotiSpec;
	CMessage *pRspMsg;
	CSET_READER_CONFIG_RESPONSE *pRsp;

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
	pENState->setNotificationState(1);
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
	pENState->setNotificationState(1);
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
	pENState->setNotificationState(0);
	pNotiSpec->addEventNotificationState(pENState);
	pCmd->setReaderEventNotificationSpec(pNotiSpec);

	pRspMsg = _pTestClient->transact(pCmd);

	delete pCmd;

	if(NULL == pRspMsg)
	{
		return -1;
	}

	pRsp = (CSET_READER_CONFIG_RESPONSE *) pRspMsg;

	if(0 != _pTestClient->checkLLRPStatus(pRsp->getLLRPStatus(), "TCR_R9_Step1"))
	{
		delete pRspMsg;
		return -1;
	}

	delete pRspMsg;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R9_Step1 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	return 0;
}


inline int TCR_R9::TCR_R9_Step2(void) {
	CADD_ROSPEC *               pCmd;
	CROSpec *					pBasicROSpec;
	CAISpec *					pBasicAISpec;
	CTagObservationTrigger *	pTagObservationTrigger;
	CMessage *                  pRspMsg;
	CADD_ROSPEC_RESPONSE *      pRsp;

	pBasicROSpec = _pTestClient->getBasicROSpec();
	pBasicAISpec = _pTestClient->getBasicAISpec();

	pBasicAISpec->getAISpecStopTrigger()->setAISpecStopTriggerType(LLRP::AISpecStopTriggerType_Tag_Observation);
	pTagObservationTrigger = new CTagObservationTrigger();
	pTagObservationTrigger->setNumberOfAttempts(0);
	pTagObservationTrigger->setNumberOfTags(2);
	pTagObservationTrigger->setT(0);
	pTagObservationTrigger->setTimeout(CONST_R9_2);
	pTagObservationTrigger->setTriggerType(LLRP::TagObservationTriggerType_Upon_Seeing_N_Tags_Or_Timeout);
	pBasicAISpec->getAISpecStopTrigger()->setTagObservationTrigger(pTagObservationTrigger);
	pBasicAISpec->getAISpecStopTrigger()->setDurationTrigger(0);
	pBasicROSpec->addSpecParameter(pBasicAISpec);

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
		printf("INFO: TCR_R9_Step2 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	/*
	 * Victory.
	 */
	return 0;
}

inline int TCR_R9::TCR_R9_Step3(void) {
	if (_pTestClient->enableROSpec(123, "TCR_R9_Step3") != 0)
		return -1;
	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R9_Step3 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}
	return 0;
}

inline int TCR_R9::TCR_R9_Step4(void) {
	CREADER_EVENT_NOTIFICATION *pRsp;
	CReaderEventNotificationData *pNtfData;
	CROSpecEvent *				pEvent;
	// Start ROSpec
	_pStartTime = _pTestClient->getCurrentTimeMilliseconds();
	if (_pTestClient->startROSpec(123, "TCR_R9_Step4") != 0)
		return -1;

	// Check Reader event notification
	pRsp = _pTestClient->recvNotification(10000, "TCR_R9_Step4");
	if (pRsp == NULL)
	{
		goto fail;
	}
	pNtfData = pRsp->getReaderEventNotificationData();
	if(NULL == pNtfData)
	{
		goto fail;
	}

	pEvent = pNtfData->getROSpecEvent();
	if(NULL == pEvent)
	{
		goto fail;
	}

	if(LLRP::ROSpecEventType_Start_Of_ROSpec != pEvent->getEventType()
			|| 123 != pEvent->getROSpecID())
	{
		goto fail;
	}

	delete pRsp;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R9_Step4 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	return 0;

	fail:
	printf("ERROR\n");
	delete pRsp;
	return -1;
}

inline int TCR_R9::TCR_R9_Step5(void) {
	CREADER_EVENT_NOTIFICATION *pRsp = NULL;
	CReaderEventNotificationData *pNtfData;
	CROSpecEvent *				pROEvent;
	CAISpecEvent *				pAIEvent;
	CRO_ACCESS_REPORT *			pReport;

	// Wait R9.5 (default=40) seconds for AISpec and ROSpec to stop.
	boost::this_thread::sleep_for(boost::chrono::milliseconds(CONST_R9_5));

	// Check AISpec_Stop event
	// Check Reader event notification
	pRsp = _pTestClient->recvNotification(0, "TCR_R9_Step5");
	if (pRsp == NULL)
	{
		goto fail;
	}
	pNtfData = pRsp->getReaderEventNotificationData();
	if(NULL == pNtfData)
	{
		goto fail;
	}

	pAIEvent = pNtfData->getAISpecEvent();
	if(NULL == pAIEvent)
	{
		goto fail;
	}

	if(LLRP::AISpecEventType_End_Of_AISpec != pAIEvent->getEventType()
			|| 123 != pAIEvent->getROSpecID())
	{
		goto fail;
	}

	delete pRsp;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: AISpec_Stop event is received\n");
	}

	// Check RO_ACCESS_REPORT
	pReport = _pTestClient->getROAccessReport(0, "TCR_R9_Step5");
	if (pReport == NULL)
		return -1;
	_pTestClient->printTagReportData(pReport);
	if (pReport->countTagReportData() != 0) {
		// There should be no tag report in the RO_ACCESS_REPORT.
		delete pReport;
		return -1;
	}
	delete pReport;

	// Check ROSpec_Stop event
	// Check Reader event notification
	pRsp = _pTestClient->recvNotification(0, "TCR_R9_Step5");
	if (pRsp == NULL)
	{
		goto fail;
	}
	pNtfData = pRsp->getReaderEventNotificationData();
	if(NULL == pNtfData)
	{
		goto fail;
	}

	pROEvent = pNtfData->getROSpecEvent();
	if(NULL == pROEvent)
	{
		goto fail;
	}

	if(LLRP::ROSpecEventType_End_Of_ROSpec != pROEvent->getEventType()
			|| 123 != pROEvent->getROSpecID())
	{
		goto fail;
	}

	delete pRsp;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R9_Step5 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	return 0;

	fail:
	printf("ERROR: TCR_R9_Step5 failed\n");
	if (pRsp != NULL)
		delete pRsp;
	return -1;
}

inline int TCR_R9::TCR_R9_Step6(void) {
	CREADER_EVENT_NOTIFICATION *pRsp;
	CReaderEventNotificationData *pNtfData;
	CROSpecEvent *				pEvent;
	// Start ROSpec
	_pStartTime = _pTestClient->getCurrentTimeMilliseconds();
	if (_pTestClient->startROSpec(123, "TCR_R9_Step6") != 0)
		return -1;

	// Check Reader event notification
	pRsp = _pTestClient->recvNotification(10000, "TCR_R9_Step6");
	if (pRsp == NULL)
	{
		goto fail;
	}
	pNtfData = pRsp->getReaderEventNotificationData();
	if(NULL == pNtfData)
	{
		goto fail;
	}

	pEvent = pNtfData->getROSpecEvent();
	if(NULL == pEvent)
	{
		goto fail;
	}

	if(LLRP::ROSpecEventType_Start_Of_ROSpec != pEvent->getEventType()
			|| 123 != pEvent->getROSpecID())
	{
		goto fail;
	}

	delete pRsp;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R9_Step6 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	return 0;

	fail:
	printf("ERROR: TCR_R9_Step6 failed\n");
	delete pRsp;
	return -1;
}

inline int TCR_R9::TCR_R9_Step7(void) {
	CREADER_EVENT_NOTIFICATION *pRsp = NULL;
	CReaderEventNotificationData *pNtfData;
	CROSpecEvent *				pROEvent;
	CAISpecEvent *				pAIEvent;
	CRO_ACCESS_REPORT *			pReport;

	// Wait R9.5 (default=40) seconds for AISpec and ROSpec to stop.
	printf ("INFO: ==================================================================================\n");
	printf ("INFO: If the reader is wrapper mode, place 2 tags to the FOV of Antenna 1 in 10 seconds.\n");
	printf ("INFO: ==================================================================================\n");
	boost::this_thread::sleep_for(boost::chrono::milliseconds(CONST_R9_7 / 2));
	// place 2 tags per each antenna
	if (_pTestClient->sendCloseConnectionWithCustomCommand(LLRPTestClient::CLOSECONNECTION_CUSTOM_COMMAND_PLACE_2_TAGS) != 0)
		return -1;
	boost::this_thread::sleep_for(boost::chrono::milliseconds(CONST_R9_7 / 2));

	// Check AISpec_Stop event
	// Check Reader event notification
	pRsp = _pTestClient->recvNotification(0, "TCR_R9_Step7");
	if (pRsp == NULL)
	{
		goto fail;
	}
	pNtfData = pRsp->getReaderEventNotificationData();
	if(NULL == pNtfData)
	{
		goto fail;
	}

	pAIEvent = pNtfData->getAISpecEvent();
	if(NULL == pAIEvent)
	{
		goto fail;
	}

	if(LLRP::AISpecEventType_End_Of_AISpec != pAIEvent->getEventType()
			|| 123 != pAIEvent->getROSpecID())
	{
		goto fail;
	}

	delete pRsp;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: AISpec_Stop event is received\n");
	}

	// Check RO_ACCESS_REPORT
	pReport = _pTestClient->getROAccessReport(0, "TCR_R9_Step7");
	if (pReport == NULL)
		return -1;
	_pTestClient->printTagReportData(pReport);
	if (pReport->countTagReportData() != 2) {
		// There should be exactly 2 tag reports in the RO_ACCESS_REPORT.
		printf ("ERROR: %d tags are in the tag report. It should be %d", pReport->countTagReportData(), 2);
		delete pReport;
		return -1;
	}
	delete pReport;

	// Check ROSpec_Stop event
	// Check Reader event notification
	pRsp = _pTestClient->recvNotification(0, "TCR_R9_Step7");
	if (pRsp == NULL)
	{
		goto fail;
	}
	pNtfData = pRsp->getReaderEventNotificationData();
	if(NULL == pNtfData)
	{
		goto fail;
	}

	pROEvent = pNtfData->getROSpecEvent();
	if(NULL == pROEvent)
	{
		goto fail;
	}

	if(LLRP::ROSpecEventType_End_Of_ROSpec != pROEvent->getEventType()
			|| 123 != pROEvent->getROSpecID())
	{
		goto fail;
	}

	delete pRsp;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R9_Step7 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	return 0;

	fail:
	printf("ERROR: TCR_R9_Step7 failed\n");
	if (pRsp != NULL)
		delete pRsp;
	return -1;
}

inline int TCR_R9::TCR_R9_Step8(void) {
	CSET_READER_CONFIG *pCmd;
	CROReportSpec *pRORptSpec;
	CMessage *pRspMsg;
	CSET_READER_CONFIG_RESPONSE *pRsp;

	pCmd = new CSET_READER_CONFIG();
	pCmd->setMessageID(121);

	pRORptSpec = new CROReportSpec();
	pRORptSpec->setN(1);
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

	pRspMsg = _pTestClient->transact(pCmd);

	delete pCmd;

	if(NULL == pRspMsg)
	{
		return -1;
	}

	pRsp = (CSET_READER_CONFIG_RESPONSE *) pRspMsg;

	if(0 != _pTestClient->checkLLRPStatus(pRsp->getLLRPStatus(), "TCR_R9_Step8"))
	{
		delete pRspMsg;
		return -1;
	}

	delete pRspMsg;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R9_Step8 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	return 0;
}

inline int TCR_R9::TCR_R9_Step9(void) {
	CREADER_EVENT_NOTIFICATION *pRsp;
	CReaderEventNotificationData *pNtfData;
	CROSpecEvent *				pEvent;
	CRO_ACCESS_REPORT *			pReport;


	// remove remaining messages
	while (1) {
		pRsp = _pTestClient->recvNotification(0, "TCR_R9_Step9: Removing all notifications messages in buffer\n");
		if (pRsp == NULL)
			break;
	}
	while (1) {
		pReport = _pTestClient->getROAccessReport(0, "TCR_R9_Step9: Removing all RO_ACCESS_REPORTS in buffer\n");
		if (pReport == NULL)
			break;
	}

	// Start ROSpec
	_pStartTime = _pTestClient->getCurrentTimeMilliseconds();
	if (_pTestClient->startROSpec(123, "TCR_R9_Step9") != 0)
		return -1;

	// Check Reader event notification
	pRsp = _pTestClient->recvNotification(10000, "TCR_R9_Step9");
	if (pRsp == NULL)
	{
		goto fail;
	}
	pNtfData = pRsp->getReaderEventNotificationData();
	if(NULL == pNtfData)
	{
		goto fail;
	}

	pEvent = pNtfData->getROSpecEvent();
	if(NULL == pEvent)
	{
		goto fail;
	}

	if(LLRP::ROSpecEventType_Start_Of_ROSpec != pEvent->getEventType()
			|| 123 != pEvent->getROSpecID())
	{
		goto fail;
	}

	delete pRsp;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R9_Step9 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	return 0;

	fail:
	printf("ERROR: TCR_R9_Step9 failed\n");
	delete pRsp;
	return -1;
}

inline int TCR_R9::TCR_R9_Step10(void) {
	CREADER_EVENT_NOTIFICATION *pRsp = NULL;
	CReaderEventNotificationData *pNtfData;
	CROSpecEvent *				pROEvent;
	CAISpecEvent *				pAIEvent;
	CRO_ACCESS_REPORT *			pReport1;
	CRO_ACCESS_REPORT *			pReport2;
	CTagReportData *			pRptData1;
	CTagReportData *			pRptData2;
	// Wait R9.5 (default=40) seconds for AISpec and ROSpec to stop.
	printf ("INFO: ==================================================================================\n");
	printf ("INFO: If the reader is wrapper mode, place 2 tags to the FOV of Antenna 1 in 10 seconds.\n");
	printf ("INFO: ==================================================================================\n");
	boost::this_thread::sleep_for(boost::chrono::milliseconds(CONST_R9_10 / 2));
	// place 2 tags per each antenna
	if (_pTestClient->sendCloseConnectionWithCustomCommand(LLRPTestClient::CLOSECONNECTION_CUSTOM_COMMAND_PLACE_2_TAGS) != 0)
		return -1;
	boost::this_thread::sleep_for(boost::chrono::milliseconds(CONST_R9_10 / 2));

	// Check AISpec_Stop event
	// Check Reader event notification
	pRsp = _pTestClient->recvNotification(0, "TCR_R9_Step10");
	if (pRsp == NULL)
	{
		goto fail;
	}
	pNtfData = pRsp->getReaderEventNotificationData();
	if(NULL == pNtfData)
	{
		goto fail;
	}

	pAIEvent = pNtfData->getAISpecEvent();
	if(NULL == pAIEvent)
	{
		goto fail;
	}

	if(LLRP::AISpecEventType_End_Of_AISpec != pAIEvent->getEventType()
			|| 123 != pAIEvent->getROSpecID())
	{
		goto fail;
	}

	delete pRsp;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: AISpec_Stop event is received\n");
	}

	// Check RO_ACCESS_REPORT 1
	pReport1 = _pTestClient->getROAccessReport(0, "TCR_R9_Step10");
	if (pReport1 == NULL)
		return -1;
	_pTestClient->printTagReportData(pReport1);
	if (pReport1->countTagReportData() != 1) {
		// There should be exactly 2 tag reports in the RO_ACCESS_REPORT.
		delete pReport1;
		return -1;
	}

	// Check RO_ACCESS_REPORT 2
	pReport2 = _pTestClient->getROAccessReport(0, "TCR_R9_Step10");
	if (pReport2 == NULL) {
		delete pReport1;
		return -1;
	}
	_pTestClient->printTagReportData(pReport2);
	if (pReport2->countTagReportData() != 1) {
		// There should be exactly 2 tag reports in the RO_ACCESS_REPORT.
		delete pReport1;
		delete pReport2;
		return -1;
	}
	pRptData1 = *(pReport1->beginTagReportData());
	pRptData2 = *(pReport2->beginTagReportData());
	// If two tags in each report is same, then this is failure.
	if (_pTestClient->getTagReportHash(pRptData1) == _pTestClient->getTagReportHash(pRptData2)) {
		delete pReport1;
		delete pReport2;
		return -1;
	}

	delete pReport1;
	delete pReport2;

	// Check ROSpec_Stop event
	// Check Reader event notification
	pRsp = _pTestClient->recvNotification(0, "TCR_R9_Step10");
	if (pRsp == NULL)
	{
		goto fail;
	}
	pNtfData = pRsp->getReaderEventNotificationData();
	if(NULL == pNtfData)
	{
		goto fail;
	}

	pROEvent = pNtfData->getROSpecEvent();
	if(NULL == pROEvent)
	{
		goto fail;
	}

	if(LLRP::ROSpecEventType_End_Of_ROSpec != pROEvent->getEventType()
			|| 123 != pROEvent->getROSpecID())
	{
		goto fail;
	}

	delete pRsp;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R9_Step10 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	return 0;

	fail:
	printf("ERROR: TCR_R9_Step10 failed\n");
	if (pRsp != NULL)
		delete pRsp;
	return -1;
}



#endif /* TCR_R9_H_ */
